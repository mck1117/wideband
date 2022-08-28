#include "ch.h"
#include "hal.h"

#include "can.h"
#include "fault.h"
#include "heater_control.h"
#include "pump_control.h"
#include "pump_dac.h"
#include "sampling.h"
#include "uart.h"
#include "io_pins.h"
#include "auxout.h"
#include "max31855.h"

#include "wideband_config.h"

using namespace wbo;

/*
 * Application entry point.
 */
int main() {
    halInit();
    chSysInit();

    // Fire up all of our threads
    StartSampling();
    InitPumpDac();
    StartHeaterControl();
    StartPumpControl();
    InitAuxDac();

    InitCan();
    InitUart();

#if (EGT_CHANNELS > 0)
    StartEgt();
#endif

    while(true)
    {
        /* TODO: show error for all AFR channels */
        /* TODO: show EGT errors */
        auto fault = GetCurrentFault(0);

        if (fault == Fault::None)
        {
            // blue is off
            palClearPad(LED_BLUE_PORT, LED_BLUE_PIN);

            // Green is blinking
            palTogglePad(LED_GREEN_PORT, LED_GREEN_PIN);

            // Slow blink if closed loop, fast if not
            chThdSleepMilliseconds(IsRunningClosedLoop(0) ? 700 : 50);
        }
        else
        {
            // green is off
            palClearPad(LED_GREEN_PORT, LED_GREEN_PIN);

            // Blink out the error code
            for (int i = 0; i < 2 * static_cast<int>(fault); i++)
            {
                // Blue is blinking
                palTogglePad(LED_BLUE_PORT, LED_BLUE_PIN);

                // fast blink
                chThdSleepMilliseconds(300);
            }

            chThdSleepMilliseconds(2000);
        }
    }
}
