#include "pump_control.h"
#include "wideband_config.h"
#include "heater_control.h"
#include "sampling.h"
#include "pump_dac.h"
#include "pid.h"

#include "ch.h"

struct pump_control_state {
    Pid pumpPid;
};

static struct pump_control_state state[AFR_CHANNELS] =
{
    {
        Pid(50.0f, 10000.0f, 0.0f, 10.0f, 2),
    },
#if (AFR_CHANNELS > 1)
    {
        Pid(50.0f, 10000.0f, 0.0f, 10.0f, 2),
    }
#endif
};

static THD_WORKING_AREA(waPumpThread, 256);
static void PumpThread(void*)
{
    while(true)
    {
        for (int ch = 0; ch < AFR_CHANNELS; ch++)
        {
            struct pump_control_state *s = &state[ch];

            // Only actuate pump when running closed loop!
            if (IsRunningClosedLoop(ch))
            {
                float nernstVoltage = GetNernstDc(ch);

                float result = s->pumpPid.GetOutput(NERNST_TARGET, nernstVoltage);

                // result is in mA
                SetPumpCurrentTarget(ch, result * 1000);
            }
            else
            {
                // Otherwise set zero pump current to avoid damaging the sensor
                SetPumpCurrentTarget(ch, 0);
            }
        }

        // Run at 500hz
        chThdSleepMilliseconds(2);
    }
}

void StartPumpControl()
{
    chThdCreateStatic(waPumpThread, sizeof(waPumpThread), NORMALPRIO + 4, PumpThread, nullptr);
}
