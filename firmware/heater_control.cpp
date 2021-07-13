#include "heater_control.h"
#include "wideband_config.h"

#include "ch.h"
#include "hal.h"

#include "fault.h"
#include "pwm.h"
#include "sampling.h"
#include "pid.h"

// 400khz / 1024 = 390hz PWM
Pwm heaterPwm(PWMD1, 0, 400'000, 1024);

enum class HeaterState
{
    Preheat,
    WarmupRamp,
    ClosedLoop,
    Stopped,
};

int timeCounter = HEATER_PREHEAT_TIME / HEATER_CONTROL_PERIOD;
float rampVoltage = 0;

static HeaterState GetNextState(HeaterState state, float sensorEsr)
{
    switch (state)
    {
        case HeaterState::Preheat:
            timeCounter--;

            // If preheat timeout, or sensor is already hot (engine running?)
            if (timeCounter <= 0 || sensorEsr < HEATER_CLOSED_LOOP_THRESHOLD_ESR)
            {
                // If enough time has elapsed, start the ramp
                // Start the ramp at 4 volts
                rampVoltage = 5;

                // Next phase times out at 15 seconds
                timeCounter = HEATER_WARMUP_TIMEOUT / HEATER_CONTROL_PERIOD;

                return HeaterState::WarmupRamp;
            }

            // Stay in preheat - wait for time to elapse
            break;
        case HeaterState::WarmupRamp:
            if (sensorEsr < HEATER_CLOSED_LOOP_THRESHOLD_ESR)
            {
                return HeaterState::ClosedLoop;
            }
            else if (timeCounter == 0)
            {
                setFault(Fault::SensorDidntHeat);
                return HeaterState::Stopped;
            }

            timeCounter--;

            break;
        case HeaterState::ClosedLoop:
            if (sensorEsr < HEATER_OVERHEAT_ESR)
            {
                setFault(Fault::SensorOverheat);
                return HeaterState::Stopped;
            }
            else if (sensorEsr > HEATER_UNDERHEAT_ESR)
            {
                setFault(Fault::SensorUnderheat);
                return HeaterState::Stopped;
            }

            break;
        case HeaterState::Stopped: break;
    }

    return state;
}

static Pid heaterPid(
    0.3f,      // kP
    0.3f,      // kI
    0.02f,     // kD
    3.0f,      // Integrator clamp (volts)
    HEATER_CONTROL_PERIOD
);

static float GetVoltageForState(HeaterState state, float heaterEsr)
{
    switch (state)
    {
        case HeaterState::Preheat: return 0.04f;
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
            return 7.5f - heaterPid.GetOutput(HEATER_TARGET_ESR, heaterEsr);
        case HeaterState::Stopped:
            // Something has gone wrong, turn off the heater.
            return 0;
    }

    // should be unreachable
    return 0;
}

static HeaterState state = HeaterState::Preheat;

// Nominal battery voltage (running engine) is 14v
static float batteryVoltage = 14;

static THD_WORKING_AREA(waHeaterThread, 256);
static void HeaterThread(void*)
{
    // Wait for temperature sensing to stabilize so we don't
    // immediately think we overshot the target temperature
    chThdSleepMilliseconds(1000);

    while (true)
    {
        // Read sensor state
        float heaterEsr = GetSensorInternalResistance();

        // Run the state machine
        state = GetNextState(state, heaterEsr);
        float heaterVoltage = GetVoltageForState(state, heaterEsr);

        // Limit to 11 volts
        if (heaterVoltage > 11) {
            heaterVoltage = 11;
        }

        // duty = (V_eff / V_batt) ^ 2
        float voltageRatio = heaterVoltage / batteryVoltage;
        float duty = voltageRatio * voltageRatio;

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

bool IsRunningClosedLoop()
{
    return state == HeaterState::ClosedLoop;
}

void SetBatteryVoltage(float vbatt)
{
    if (vbatt < 5)
    {
        // provided vbatt is bogus, default to 14v nominal
        batteryVoltage = 14;
    }
    else
    {
        batteryVoltage = vbatt;
    }
}
