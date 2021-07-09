#include "pump_control.h"
#include "wideband_config.h"
#include "heater_control.h"
#include "sampling.h"
#include "pump_dac.h"
#include "pid.h"

#include "ch.h"

// Bosch CJ125 is somewhere VERY ROUGHLY like 200-400A/(v*s) integrator gain
static Pid pumpPid(50.0f, 10000.0f, 0, 10, 2);

static THD_WORKING_AREA(waPumpThread, 256);
static void PumpThread(void*)
{
    while(true)
    {
        // Only actuate pump when running closed loop!
        if (IsRunningClosedLoop())
        {
            float nernstVoltage = GetNernstDc();

            float result = pumpPid.GetOutput(NERNST_TARGET, nernstVoltage);

            // result is in mA
            SetPumpCurrentTarget(result * 1000);
        }
        else
        {
            // Otherwise set zero pump current to avoid damaging the sensor
            SetPumpCurrentTarget(0);
        }

        // Run at 500hz
        chThdSleepMilliseconds(2);
    }
}

void StartPumpControl()
{
    chThdCreateStatic(waPumpThread, sizeof(waPumpThread), NORMALPRIO + 4, PumpThread, nullptr);
}
