#include "heater_control.h"

#include "ch.h"
#include "hal.h"

#include "port.h"
#include "fault.h"
#include "pwm.h"
#include "sampling.h"
#include "pid.h"

struct sensorHeaterParams {
    float targetTemp;
    float targetESR;
};

static const struct sensorHeaterParams heaterParams49 = {
    .targetTemp = 780,
    .targetESR = 300,
};

static const struct sensorHeaterParams heaterParams42 = {
    .targetTemp = 730,
    .targetESR = 80,
};

static const struct sensorHeaterParams heaterParamsAdv = {
    .targetTemp = 785,
    .targetESR = 300,
};

static const sensorHeaterParams *getHeaterParams(SensorType type) {
    switch (type) {
        case SensorType::LSU42:
            return &heaterParams42;
        case SensorType::LSUADV:
            return &heaterParamsAdv;
        case SensorType::LSU49:
        default:
            return &heaterParams49;
    }
}

using namespace wbo;

// 400khz / 1024 = 390hz PWM
static Pwm heaterPwm(HEATER_PWM_DEVICE);
static const PWMConfig heaterPwmConfig = {
    .frequency = 400'000,
    .period = 1024,
    .callback = nullptr,
    .channels = {
        {PWM_OUTPUT_ACTIVE_HIGH, nullptr},
        {PWM_OUTPUT_ACTIVE_HIGH, nullptr},
        {PWM_OUTPUT_ACTIVE_HIGH, nullptr},
        {PWM_OUTPUT_ACTIVE_HIGH, nullptr}
    },
    .cr2 = 0,
#if STM32_PWM_USE_ADVANCED
    .bdtr = 0,
#endif
    .dier = 0
};

static constexpr int preheatTimeCounter = HEATER_PREHEAT_TIME / HEATER_CONTROL_PERIOD;
static constexpr int batteryStabTimeCounter = HEATER_BATTERY_STAB_TIME / HEATER_CONTROL_PERIOD;
static const struct sensorHeaterParams *heater;

class HeaterController : public IHeaterController
{
public:
    HeaterController(int ch, int pwm_ch)
        : ch(ch), pwm_ch(pwm_ch)
    {
    }

    void Update(const ISampler& sampler, HeaterAllow heaterAllowState) override;

    bool IsRunningClosedLoop() const override
    {
        return heaterState == HeaterState::ClosedLoop;
    }

    float GetHeaterEffectiveVoltage() const override
    {
        return heaterVoltage;
    }

    HeaterState GetHeaterState() const override
    {
        return heaterState;
    }

protected:
    HeaterState GetNextState(HeaterAllow haeterAllowState, float batteryVoltage, float sensorTemp);
    float GetVoltageForState(float heaterEsr);

private:
    Pid heaterPid =
        {
            0.3f,      // kP
            0.3f,      // kI
            0.01f,     // kD
            3.0f,      // Integrator clamp (volts)
            HEATER_CONTROL_PERIOD
        };

    int timeCounter = preheatTimeCounter;
    int batteryStabTime = batteryStabTimeCounter;
    float rampVoltage = 0;
    float heaterVoltage = 0;
    HeaterState heaterState = HeaterState::Preheat;
#ifdef HEATER_MAX_DUTY
    int cycle;
#endif

// TODO: private:
public:
    const uint8_t ch;
    const uint8_t pwm_ch;
};

HeaterController heaterControllers[AFR_CHANNELS] =
{
    { 0, HEATER_PWM_CHANNEL_0 },

#if AFR_CHANNELS >= 2
    { 1, HEATER_PWM_CHANNEL_1 }
#endif
};

const IHeaterController& GetHeaterController(int ch)
{
    return heaterControllers[ch];
}

HeaterState HeaterController::GetNextState(HeaterAllow heaterAllowState, float batteryVoltage, float sensorTemp)
{
    bool heaterAllowed = heaterAllowState == HeaterAllow::Allowed;

    // Check battery voltage for thresholds only if there is still no command over CAN
    if (heaterAllowState == HeaterAllow::Unknown)
    {
        // measured voltage too low to auto-start heating
        if (batteryVoltage < HEATER_BATTETY_OFF_VOLTAGE)
        {
            batteryStabTime = batteryStabTimeCounter;
        }
        // measured voltage is high enougth to auto-start heating, wait some time to stabilize
        if ((batteryVoltage > HEATER_BATTERY_ON_VOLTAGE) && (batteryStabTime > 0))
        {
            batteryStabTime--;
        }
        heaterAllowed = batteryStabTime == 0;
    }

    if (!heaterAllowed)
    {
        // ECU hasn't allowed preheat yet, reset timer, and force preheat state
        timeCounter = preheatTimeCounter;
        return HeaterState::Preheat;
    }

    float overheatTemp = heater->targetTemp + 100;
    float closedLoopTemp = heater->targetTemp - 50;
    float underheatTemp = heater->targetTemp - 100;

    switch (heaterState)
    {
        case HeaterState::Preheat:
            timeCounter--;

            // If preheat timeout, or sensor is already hot (engine running?)
            if (timeCounter <= 0 || sensorTemp > closedLoopTemp)
            {
                // If enough time has elapsed, start the ramp
                // Start the ramp at 4 volts
                rampVoltage = 4;

                // Next phase times out at 15 seconds
                timeCounter = HEATER_WARMUP_TIMEOUT / HEATER_CONTROL_PERIOD;

                return HeaterState::WarmupRamp;
            }

            // Stay in preheat - wait for time to elapse
            break;
        case HeaterState::WarmupRamp:
            if (sensorTemp > closedLoopTemp)
            {
                return HeaterState::ClosedLoop;
            }
            else if (timeCounter == 0)
            {
                SetFault(ch, Fault::SensorDidntHeat);
                return HeaterState::Stopped;
            }

            timeCounter--;

            break;
        case HeaterState::ClosedLoop:
            // Check that the sensor's ESR is acceptable for normal operation
            if (sensorTemp > overheatTemp)
            {
                SetFault(ch, Fault::SensorOverheat);
                return HeaterState::Stopped;
            }
            else if (sensorTemp < underheatTemp)
            {
                SetFault(ch, Fault::SensorUnderheat);
                return HeaterState::Stopped;
            }

            break;
        case HeaterState::Stopped:
        case HeaterState::NoHeaterSupply:
            /* nop */
            break;
    }

    return heaterState;
}

float HeaterController::GetVoltageForState(float heaterEsr)
{
    switch (heaterState)
    {
        case HeaterState::Preheat:
            // Max allowed during condensation phase (preheat) is 2v
            return 1.5f;
        case HeaterState::WarmupRamp:
            if (rampVoltage < 10)
            {
                // 0.3 volt per second, divided by battery voltage and update rate
                constexpr float rampRateVoltPerSecond = 0.3f;
                constexpr float heaterFrequency = 1000.0f / HEATER_CONTROL_PERIOD;
                rampVoltage += (rampRateVoltPerSecond / heaterFrequency);
            }

            return rampVoltage;
        case HeaterState::ClosedLoop:
            // "nominal" heater voltage is 7.5v, so apply correction around that point (instead of relying on integrator so much)
            // Negated because lower resistance -> hotter

            // TODO: heater PID should operate on temperature, not ESR
            return 7.5f - heaterPid.GetOutput(heater->targetESR, heaterEsr);
        case HeaterState::Stopped:
            // Something has gone wrong, turn off the heater.
            return 0;
    }

    // should be unreachable
    return 0;
}

void HeaterController::Update(const ISampler& sampler, HeaterAllow heaterAllowState)
{
    // Read sensor state
    float heaterEsr = sampler.GetSensorInternalResistance();
    float sensorTemperature = sampler.GetSensorTemperature();

    // If we haven't heard from the ECU, use the internally sensed
    // battery voltage instead of voltage over CAN.
    float batteryVoltage = heaterAllowState == HeaterAllow::Unknown
                                ? sampler.GetInternalBatteryVoltage()
                                : GetRemoteBatteryVoltage();

    // Run the state machine
    heaterState = GetNextState(heaterAllowState, batteryVoltage, sensorTemperature);
    float heaterVoltage = GetVoltageForState(heaterEsr);

    // Limit to 11 volts
    if (heaterVoltage > 11) {
        heaterVoltage = 11;
    }

    // duty = (V_eff / V_batt) ^ 2
    float voltageRatio = heaterVoltage / batteryVoltage;
    float duty = voltageRatio * voltageRatio;

    #ifdef HEATER_MAX_DUTY
    cycle++;
    // limit PWM each 10th cycle (2 time per second) to measure heater supply voltage throuth "Heater-"
    if ((cycle % 10) == 0) {
        if (duty > HEATER_MAX_DUTY) {
            duty = HEATER_MAX_DUTY;
        }
    }
    #endif

    if (batteryVoltage >= 23)
    {
        duty = 0;
        heaterVoltage = 0;
    }
    // Pipe the output to the heater driver
    heaterPwm.SetDuty(pwm_ch, duty);
    heaterVoltage = heaterVoltage;
}

static THD_WORKING_AREA(waHeaterThread, 256);
static void HeaterThread(void*)
{
    int i;

    chRegSetThreadName("Heater");

    // Wait for temperature sensing to stabilize so we don't
    // immediately think we overshot the target temperature
    chThdSleepMilliseconds(1000);

    // Get sensor type and settings
    heater = getHeaterParams(GetSensorType());

    while (true)
    {
        auto heaterAllowState = GetHeaterAllowed();

        for (i = 0; i < AFR_CHANNELS; i++) {
            const auto& sampler = GetSampler(i);
            auto& heater = heaterControllers[i];

            heater.Update(sampler, heaterAllowState);
        }

        // Loop at ~20hz
        chThdSleepMilliseconds(HEATER_CONTROL_PERIOD);
    }
}

void StartHeaterControl()
{
    heaterPwm.Start(heaterPwmConfig);
    heaterPwm.SetDuty(heaterControllers[0].pwm_ch, 0);
#if (AFR_CHANNELS > 1)
    heaterPwm.SetDuty(heaterControllers[1].pwm_ch, 0);
#endif

    chThdCreateStatic(waHeaterThread, sizeof(waHeaterThread), NORMALPRIO + 1, HeaterThread, nullptr);
}

float GetHeaterDuty(int ch)
{
    return heaterPwm.GetLastDuty(heaterControllers[ch].pwm_ch);
}

HeaterState GetHeaterState(int ch)
{
    return heaterControllers[ch].GetHeaterState();
}

const char* describeHeaterState(HeaterState state)
{
    switch (state) {
        case HeaterState::Preheat:
            return "Preheat";
        case HeaterState::WarmupRamp:
            return "WarmupRamp";
        case HeaterState::ClosedLoop:
            return "ClosedLoop";
        case HeaterState::Stopped:
            return "Stopped";
        case HeaterState::NoHeaterSupply:
            return "NoHeaterSupply";
    }

    return "Unknown";
}
