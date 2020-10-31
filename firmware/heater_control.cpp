#include "heater_control.h"

#include "ch.h"
#include "hal.h"
#include "pwm.h"
#include "sampling.h"

// 400khz / 1024 = 390hz PWM
Pwm heaterPwm(PWMD1, 0, 400'000, 1024);

enum class HeaterState
{
    Preheat,
    WarmupRamp,
    ClosedLoop,
};

int preheatCounter = 5000 / 50;
float rampDuty = 0.5f;

static HeaterState GetNextState(HeaterState state, float sensorEsr)
{
    switch (state)
    {
        case HeaterState::Preheat:
            preheatCounter--;

            if (preheatCounter <= 0)
            {
                // If enough time has elapsed, start the ramp
                // Start the ramp at 50% duty - ~6-7 volts
                rampDuty = 0.5f;
                return HeaterState::WarmupRamp;
            }

            // Stay in preheat - wait for time to elapse
            break;
        case HeaterState::WarmupRamp:
            if (sensorEsr < 2000)
            {
                return HeaterState::ClosedLoop;
            }

            break;
        case HeaterState::ClosedLoop: break;
    }

    return state;
}

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
        {
            // do something more intelligent here
            float error = (heaterEsr - 250) / 100;

            return error * 1.0f;
        }
    }
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
        chThdSleepMilliseconds(50);
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
