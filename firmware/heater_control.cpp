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
float rampDuty = 0.5f;

static HeaterState GetNextState(HeaterState state, float sensorEsr)
{
    switch (state)
    {
        case HeaterState::Preheat:
            timeCounter--;

            if (timeCounter <= 0)
            {
                // If enough time has elapsed, start the ramp
                // Start the ramp at 50% duty - ~6-7 volts
                rampDuty = 0.5f;

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
            // TODO: handle departure from closed loop
            if (sensorEsr < HEATER_FAULT_ESR)
            {
                setFault(Fault::SensorOverheat);
                return HeaterState::Stopped;
            }

            break;
        case HeaterState::Stopped: break;
    }

    return state;
}

static Pid heaterPid(0.01f, 0.05f, 1, HEATER_CONTROL_PERIOD);

static float GetDutyForState(HeaterState state, float heaterEsr)
{
    switch (state)
    {
        case HeaterState::Preheat: return 0.125f;
        case HeaterState::WarmupRamp:
            if (rampDuty < 0.9f)
            {
                // 0.4 volt per second, divided by battery voltage and update rate
                rampDuty += (0.4f / (14 * 20));
            }

            return rampDuty;
        case HeaterState::ClosedLoop:
            // Negated because lower resistance -> hotter
            return heaterPid.GetOutput(-HEATER_TARGET_ESR, -heaterEsr);
        case HeaterState::Stopped:
            // Something has gone wrong, return 0.
            return 0;
    }

    // should be unreachable
    return 0;
}

static HeaterState state = HeaterState::Preheat;

static THD_WORKING_AREA(waHeaterThread, 256);
static void HeaterThread(void*)
{
    while (true)
    {
        // Read sensor state
        float heaterEsr = GetSensorInternalResistance();

        // Run the state machine
        state = GetNextState(state, heaterEsr);
        float duty = GetDutyForState(state, heaterEsr);

        // Pipe the output to the heater driver
        heaterPwm.SetDuty(duty);

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
