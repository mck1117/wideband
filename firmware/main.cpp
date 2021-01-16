#include "ch.h"
#include "hal.h"

#include "can.h"
#include "fault.h"
#include "heater_control.h"
#include "pump_control.h"
#include "pump_dac.h"
#include "sampling.h"
#include "uart.h"


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

    InitCan();
    //InitUart();

    while(true)
    {
        auto fault = getCurrentFault();

        if (fault == Fault::None)
        {
            // blue is off
            palClearPad(GPIOB, 5);

            // Green is blinking
            palTogglePad(GPIOB, 6);

            // Slow blink if closed loop, fast if not
            chThdSleepMilliseconds(IsRunningClosedLoop() ? 700 : 50);
        }
        else
        {
            // green is off
            palClearPad(GPIOB, 6);

            // Blink out the error code
            for (int i = 0; i < 2 * static_cast<int>(fault); i++)
            {
                // Blue is blinking
                palTogglePad(GPIOB, 5);

                // fast blink
                chThdSleepMilliseconds(300);
            }

            chThdSleepMilliseconds(2000);
        }
    }
}
