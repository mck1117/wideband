#include "heater_control.h"

#include "status.h"
#include "sampling.h"

using namespace wbo;

static const PidConfig heaterPidConfig =
{
    .kP = 0.3f,      // kP
    .kI = 0.3f,      // kI
    .kD = 0.01f,     // kD
    .clamp = 3.0f,      // Integrator clamp (volts)
};

HeaterControllerBase::HeaterControllerBase(int ch, int preheatTimeSec, int warmupTimeSec)
    : m_pid(heaterPidConfig, HEATER_CONTROL_PERIOD)
    , ch(ch)
    , m_preheatTimeSec(preheatTimeSec)
    , m_warmupTimeSec(warmupTimeSec)
{
}

void HeaterControllerBase::Configure(float targetTempC, float targetEsr)
{
    m_targetTempC = targetTempC;
    m_targetEsr = targetEsr;

    m_preheatTimer.reset();
    m_warmupTimer.reset();
    m_batteryStableTimer.reset();
}

bool HeaterControllerBase::IsRunningClosedLoop() const
{
    return heaterState == HeaterState::ClosedLoop;
}

float HeaterControllerBase::GetTargetTemp() const
{
    return m_targetTempC;
}

float HeaterControllerBase::GetHeaterEffectiveVoltage() const
{
    return heaterVoltage;
}

HeaterState HeaterControllerBase::GetHeaterState() const
{
    return heaterState;
}

HeaterState HeaterControllerBase::GetNextState(HeaterState currentState, HeaterAllow heaterAllowState, float heaterSupplyVoltage, float sensorTemp)
{
    bool heaterAllowed = heaterAllowState == HeaterAllow::Allowed;

    // Check battery voltage for thresholds only if there is still no command over CAN
    if (heaterAllowState == HeaterAllow::Unknown)
    {
        // measured voltage too low to auto-start heating
        if (heaterSupplyVoltage < HEATER_BATTETY_OFF_VOLTAGE)
        {
            m_batteryStableTimer.reset();
        }
        else if (heaterSupplyVoltage > HEATER_BATTERY_ON_VOLTAGE)
        {
            // measured voltage is high enough to auto-start heating, wait some time to stabilize
            heaterAllowed = m_batteryStableTimer.hasElapsedSec(HEATER_BATTERY_STAB_TIME);
        }
    }

    if (!heaterAllowed)
    {
        // ECU hasn't allowed preheat yet, reset timer, and force preheat state
        m_preheatTimer.reset();
        SetStatus(ch, Status::Preheat);
        return HeaterState::Preheat;
    }

    float overheatTemp = m_targetTempC + 100;
    float closedLoopTemp = m_targetTempC - 30;
    float underheatTemp = m_targetTempC - 100;

    switch (currentState)
    {
        case HeaterState::Preheat:
            #ifdef HEATER_FAST_HEATING_THRESHOLD_T
            if (sensorTemp >= HEATER_FAST_HEATING_THRESHOLD_T) {
                // if sensor is already hot - we can start from higher heater voltage
                rampVoltage = 9;

                // Reset the timer for the warmup phase
                m_warmupTimer.reset();

                SetStatus(ch, Status::Warmup);
                return HeaterState::WarmupRamp;
            }
            #endif

            // If preheat timeout, or sensor is already hot (engine running?)
            if (m_preheatTimer.hasElapsedSec(m_preheatTimeSec) || sensorTemp > closedLoopTemp)
            {
                // If enough time has elapsed, start the ramp
                // Start the ramp at 7 volts
                rampVoltage = 7;

                // Reset the timer for the warmup phase
                m_warmupTimer.reset();

                SetStatus(ch, Status::Warmup);
                return HeaterState::WarmupRamp;
            }

            // Stay in preheat - wait for time to elapse
            break;
        case HeaterState::WarmupRamp:
            if (sensorTemp > closedLoopTemp)
            {
                SetStatus(ch, Status::RunningClosedLoop);
                return HeaterState::ClosedLoop;
            }
            else if (m_warmupTimer.hasElapsedSec(m_warmupTimeSec))
            {
                SetStatus(ch, Status::SensorDidntHeat);
                return HeaterState::Stopped;
            }

            break;
        case HeaterState::ClosedLoop:
            // Over/under heat timers track how long it's been since
            // temperature was within normal range (then we abort if
            // it's been too long out of range)
            if (sensorTemp <= overheatTemp)
            {
                m_overheatTimer.reset();
            }

            if (sensorTemp >= underheatTemp)
            {
                m_underheatTimer.reset();
            }

            if (m_overheatTimer.hasElapsedSec(0.5f))
            {
                SetStatus(ch, Status::SensorOverheat);
                return HeaterState::Stopped;
            }
            else if (m_underheatTimer.hasElapsedSec(0.5f))
            {
                SetStatus(ch, Status::SensorUnderheat);
                return HeaterState::Stopped;
            }

            break;
        case HeaterState::Stopped:
            break;
    }

    return currentState;
}

float HeaterControllerBase::GetVoltageForState(HeaterState state, float sensorEsr)
{
    switch (state)
    {
        case HeaterState::Preheat:
            // Max allowed during condensation phase (preheat) is 2v
            return 2.0f;
        case HeaterState::WarmupRamp:
            if (rampVoltage < 12)
            {
                // 0.4 volt per second, divided by battery voltage and update rate
                constexpr float rampRateVoltPerSecond = 0.4f;
                constexpr float heaterFrequency = 1000.0f / HEATER_CONTROL_PERIOD;
                rampVoltage += (rampRateVoltPerSecond / heaterFrequency);
            }

            return rampVoltage;
        case HeaterState::ClosedLoop:
            // "nominal" heater voltage is 7.5v, so apply correction around that point (instead of relying on integrator so much)
            // Negated because lower resistance -> hotter

            // TODO: heater PID should operate on temperature, not ESR
            return 7.5f - m_pid.GetOutput(m_targetEsr, sensorEsr);
        case HeaterState::Stopped:
            // Something has gone wrong, turn off the heater.
            return 0;
    }

    // should be unreachable
    return 0;
}

void HeaterControllerBase::Update(const ISampler& sampler, HeaterAllow heaterAllowState)
{
    // Read sensor state
    float sensorEsr = sampler.GetSensorInternalResistance();
    float sensorTemperature = sampler.GetSensorTemperature();

    #ifdef BOARD_HAS_VOLTAGE_SENSE
        float heaterSupplyVoltage = GetSupplyVoltage();
    #else // not BOARD_HAS_VOLTAGE_SENSE
        // If we haven't heard from the ECU, use the internally sensed
        // battery voltage instead of voltage over CAN.
        float heaterSupplyVoltage = heaterAllowState == HeaterAllow::Unknown
                                    ? sampler.GetInternalHeaterVoltage()
                                    : GetRemoteBatteryVoltage();
    #endif

    // Run the state machine
    heaterState = GetNextState(heaterState, heaterAllowState, heaterSupplyVoltage, sensorTemperature);
    float heaterVoltage = GetVoltageForState(heaterState, sensorEsr);

    // Limit to 12 volts
    if (heaterVoltage > 12) {
        heaterVoltage = 12;
    }

    // Very low supply voltage -> avoid divide by zero or very high duty
    if (heaterSupplyVoltage < 3) {
        heaterSupplyVoltage = 12;
    }

    // duty = (V_eff / V_batt) ^ 2
    float voltageRatio = (heaterSupplyVoltage < 1.0f) ? 0 : heaterVoltage / heaterSupplyVoltage;
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

    // Protect the sensor in case of very high voltage
    if (heaterSupplyVoltage >= 23)
    {
        duty = 0;
        heaterVoltage = 0;
    }

    // Pipe the output to the heater driver
    SetDuty(duty);
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
