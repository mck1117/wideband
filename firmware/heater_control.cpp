#include "heater_control.h"

#include "fault.h"
#include "sampling.h"

using namespace wbo;

HeaterControllerBase::HeaterControllerBase(int ch, int preheatTimeSec, int warmupTimeSec)
    : ch(ch)
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
    m_closedLoopStableTimer.reset();
}

bool HeaterControllerBase::IsRunningClosedLoop() const
{
    return heaterState == HeaterState::ClosedLoop;
}

float HeaterControllerBase::GetHeaterEffectiveVoltage() const
{
    return heaterVoltage;
}

HeaterState HeaterControllerBase::GetHeaterState() const
{
    return heaterState;
}

HeaterState HeaterControllerBase::GetNextState(HeaterState currentState, HeaterAllow heaterAllowState, float batteryVoltage, float sensorTemp)
{
    bool heaterAllowed = heaterAllowState == HeaterAllow::Allowed;

    // Check battery voltage for thresholds only if there is still no command over CAN
    if (heaterAllowState == HeaterAllow::Unknown)
    {
        // measured voltage too low to auto-start heating
        if (batteryVoltage < HEATER_BATTETY_OFF_VOLTAGE)
        {
            m_batteryStableTimer.reset();
            return HeaterState::NoHeaterSupply;
        }
        else if (batteryVoltage > HEATER_BATTERY_ON_VOLTAGE)
        {
            // measured voltage is high enougth to auto-start heating, wait some time to stabilize
            heaterAllowed = m_batteryStableTimer.hasElapsedSec(HEATER_BATTERY_STAB_TIME);
        }
    }

    if (!heaterAllowed)
    {
        // ECU hasn't allowed preheat yet, reset timer, and force preheat state
        m_preheatTimer.reset();
        return HeaterState::Preheat;
    }

    float overheatTemp = m_targetTempC + 100;
    float closedLoopTemp = m_targetTempC - 50;
    float underheatTemp = m_targetTempC - 100;

    switch (currentState)
    {
        case HeaterState::Preheat:
            #ifdef HEATER_FAST_HEATING_THRESHOLD_T
            if (sensorTemp >= HEATER_FAST_HEATING_THRESHOLD_T) {
                // if sensor is already hot - we can start from higher heater voltage
                rampVoltage = 7.5;

                // Reset the timer for the warmup phase
                m_warmupTimer.reset();

                return HeaterState::WarmupRamp;
            }
            #endif

            // If preheat timeout, or sensor is already hot (engine running?)
            if (m_preheatTimer.hasElapsedSec(m_preheatTimeSec) || sensorTemp > closedLoopTemp)
            {
                // If enough time has elapsed, start the ramp
                // Start the ramp at 4 volts
                rampVoltage = 4;

                // Reset the timer for the warmup phase
                m_warmupTimer.reset();

                return HeaterState::WarmupRamp;
            }

            // Stay in preheat - wait for time to elapse
            break;
        case HeaterState::WarmupRamp:
            if (sensorTemp > closedLoopTemp)
            {
                m_closedLoopStableTimer.reset();
                return HeaterState::ClosedLoop;
            }
            else if (m_warmupTimer.hasElapsedSec(m_warmupTimeSec))
            {
                SetFault(ch, Fault::SensorDidntHeat);
                return HeaterState::Stopped;
            }

            break;
        case HeaterState::ClosedLoop:
            if (m_closedLoopStableTimer.hasElapsedSec(HEATER_CLOSED_LOOP_STAB_TIME)) {
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
            } else {
                // give some time for stabilization...
                // looks like heavy ramped Ipump affects sensorTemp measure
                // and right after switch to closed loop sensorTemp drops below underhead threshold
            }

            break;
        case HeaterState::Stopped:
        case HeaterState::NoHeaterSupply:
            /* nop */
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
            return 7.5f - heaterPid.GetOutput(m_targetEsr, sensorEsr);
        case HeaterState::Stopped:
            // Something has gone wrong, turn off the heater.
            return 0;
        case HeaterState::NoHeaterSupply:
            // No/too low heater supply - disable output
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

    // If we haven't heard from the ECU, use the internally sensed
    // battery voltage instead of voltage over CAN.
    float batteryVoltage = heaterAllowState == HeaterAllow::Unknown
                                ? sampler.GetInternalBatteryVoltage()
                                : GetRemoteBatteryVoltage();

    // Run the state machine
    heaterState = GetNextState(heaterState, heaterAllowState, batteryVoltage, sensorTemperature);
    float heaterVoltage = GetVoltageForState(heaterState, sensorEsr);

    // Limit to 11 volts
    if (heaterVoltage > 11) {
        heaterVoltage = 11;
    }

    // duty = (V_eff / V_batt) ^ 2
    float voltageRatio = (batteryVoltage < 1.0f) ? 0 : heaterVoltage / batteryVoltage;
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
        case HeaterState::NoHeaterSupply:
            return "NoHeaterSupply";
    }

    return "Unknown";
}
