#include "heater_control.h"
#include "wideband_config.h"

#include "ch.h"
#include "hal.h"

#include "fault.h"
#include "pwm.h"
#include "sampling.h"
#include "pid.h"
#include "can.h"

using namespace wbo;

// 400khz / 1024 = 390hz PWM
static Pwm heaterPwm(HEATER_PWM_DEVICE, HEATER_PWM_CHANNEL, 400'000, 1024);

static constexpr int preheatTimeCounter = HEATER_PREHEAT_TIME / HEATER_CONTROL_PERIOD;
static constexpr int batteryStabTimeCounter = HEATER_BATTERY_STAB_TIME / HEATER_CONTROL_PERIOD;

struct heater_state {
    int timeCounter;
    int batteryStabTime;
    float rampVoltage;
    HeaterState heaterState;
    int ch;
};

static struct heater_state state[AFR_CHANNELS];

static HeaterState GetNextState(struct heater_state *s, HeaterAllow heaterAllowState, float batteryVoltage, float sensorEsr)
{
    bool heaterAllowed = heaterAllowState == HeaterAllow::Allowed;

    // Check battery voltage for thresholds only if there is still no command over CAN
    if (heaterAllowState == HeaterAllow::Unknown)
    {
        // measured voltage too low to auto-start heating
        if (batteryVoltage < HEATER_BATTETY_OFF_VOLTAGE)
        {
            s->batteryStabTime = batteryStabTimeCounter;
        }
        // measured voltage is high enougth to auto-start heating, wait some time to stabilize
        if ((batteryVoltage > HEATER_BATTERY_ON_VOLTAGE) && (s->batteryStabTime > 0))
        {
            s->batteryStabTime--;
        }
        heaterAllowed = s->batteryStabTime == 0;
    }

    if (!heaterAllowed)
    {
        // ECU hasn't allowed preheat yet, reset timer, and force preheat state
        s->timeCounter = preheatTimeCounter;
        return HeaterState::Preheat;
    }

    switch (s->heaterState)
    {
        case HeaterState::Preheat:
            s->timeCounter--;

            // If preheat timeout, or sensor is already hot (engine running?)
            if (s->timeCounter <= 0 || sensorEsr < HEATER_CLOSED_LOOP_THRESHOLD_ESR)
            {
                // If enough time has elapsed, start the ramp
                // Start the ramp at 4 volts
                s->rampVoltage = 4;

                // Next phase times out at 15 seconds
                s->timeCounter = HEATER_WARMUP_TIMEOUT / HEATER_CONTROL_PERIOD;

                return HeaterState::WarmupRamp;
            }

            // Stay in preheat - wait for time to elapse
            break;
        case HeaterState::WarmupRamp:
            if (sensorEsr < HEATER_CLOSED_LOOP_THRESHOLD_ESR)
            {
                return HeaterState::ClosedLoop;
            }
            else if (s->timeCounter == 0)
            {
                SetFault(s->ch, Fault::SensorDidntHeat);
                return HeaterState::Stopped;
            }

            s->timeCounter--;

            break;
        case HeaterState::ClosedLoop:
            // Check that the sensor's ESR is acceptable for normal operation
            if (sensorEsr < HEATER_OVERHEAT_ESR)
            {
                SetFault(s->ch, Fault::SensorOverheat);
                return HeaterState::Stopped;
            }
            else if (sensorEsr > HEATER_UNDERHEAT_ESR)
            {
                SetFault(s->ch, Fault::SensorUnderheat);
                return HeaterState::Stopped;
            }

            break;
        case HeaterState::Stopped: break;
    }

    return s->heaterState;
}

static Pid heaterPid(
    0.3f,      // kP
    0.3f,      // kI
    0.01f,     // kD
    3.0f,      // Integrator clamp (volts)
    HEATER_CONTROL_PERIOD
);

static float GetVoltageForState(struct heater_state *s, float heaterEsr)
{
    switch (s->heaterState)
    {
        case HeaterState::Preheat:
            // Max allowed during condensation phase (preheat) is 2v
            return 1.5f;
        case HeaterState::WarmupRamp:
            if (s->rampVoltage < 10)
            {
                // 0.3 volt per second, divided by battery voltage and update rate
                constexpr float rampRateVoltPerSecond = 0.3f;
                constexpr float heaterFrequency = 1000.0f / HEATER_CONTROL_PERIOD;
                s->rampVoltage += (rampRateVoltPerSecond / heaterFrequency);
            }

            return s->rampVoltage;
        case HeaterState::ClosedLoop:
            // "nominal" heater voltage is 7.5v, so apply correction around that point (instead of relying on integrator so much)
            // Negated because lower resistance -> hotter
            return 7.5f - heaterPid.GetOutput(HEATER_TARGET_ESR, heaterEsr);
        case HeaterState::Stopped:
            // Something has gone wrong, turn off the heater.
            return 0;
    }

    // should be unreachable
    return 0;
}

static THD_WORKING_AREA(waHeaterThread, 256);
static void HeaterThread(void*)
{
    int ch;

    /* reset */
    for (ch = 0; ch < AFR_CHANNELS; ch++) {
        struct heater_state *s = &state[ch];

        s->timeCounter = preheatTimeCounter;
        s->batteryStabTime = batteryStabTimeCounter;
        s->rampVoltage = 0;
        s->heaterState = HeaterState::Preheat;
        s->ch = ch;
    }
    // Wait for temperature sensing to stabilize so we don't
    // immediately think we overshot the target temperature
    chThdSleepMilliseconds(1000);

    while (true)
    {
        auto heaterAllowState = GetHeaterAllowed();

        for (ch = 0; ch < AFR_CHANNELS; ch++) {
            struct heater_state *s = &state[ch];

            // Read sensor state
            float heaterEsr = GetSensorInternalResistance(ch);

            // If we haven't heard from rusEFI, use the internally sensed 
            // battery voltage instead of voltage over CAN.
            float batteryVoltage = heaterAllowState == HeaterAllow::Unknown
                                        ? GetInternalBatteryVoltage(ch)
                                        : GetRemoteBatteryVoltage();

            // Run the state machine
            s->heaterState = GetNextState(s, heaterAllowState, batteryVoltage, heaterEsr);
            float heaterVoltage = GetVoltageForState(s, heaterEsr);

            // Limit to 11 volts
            if (heaterVoltage > 11) {
                heaterVoltage = 11;
            }

            // duty = (V_eff / V_batt) ^ 2
            float voltageRatio = heaterVoltage / batteryVoltage;
            float duty = voltageRatio * voltageRatio;

            #ifdef HEATER_MAX_DUTY
            if (duty > HEATER_MAX_DUTY) {
                duty = HEATER_MAX_DUTY;
            }
            #endif

            if (batteryVoltage < 23)
            {
                // Pipe the output to the heater driver
                heaterPwm.SetDuty(duty);
            }
            else
            {
                // Overvoltage protection - sensor not rated for PWM above 24v
                heaterPwm.SetDuty(0);
            }
        }

        // Loop at ~20hz
        chThdSleepMilliseconds(HEATER_CONTROL_PERIOD);
    }
}

void StartHeaterControl()
{
    heaterPwm.Start();
    heaterPwm.SetDuty(0);

    chThdCreateStatic(waHeaterThread, sizeof(waHeaterThread), NORMALPRIO + 1, HeaterThread, nullptr);
}

bool IsRunningClosedLoop(int ch)
{
    return state[ch].heaterState == HeaterState::ClosedLoop;
}

float GetHeaterDuty(int ch)
{
    return heaterPwm.GetLastDuty();
}

HeaterState GetHeaterState(int ch)
{
    return state[ch].heaterState;
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
    }

    return "Unknown";
}
