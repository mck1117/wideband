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
        auto fault = GetCurrentFault();

        if (fault == Fault::None)
        {
            // blue is off
            palClearPad(BLUE_LED_PORT, BLUE_LED_PIN);

            // Green is blinking
            palTogglePad(GREEN_LED_PORT, GREEN_LED_PIN);

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
                palTogglePad(BLUE_LED_PORT, BLUE_LED_PIN);

                // fast blink
                chThdSleepMilliseconds(300);
            }

            chThdSleepMilliseconds(2000);
        }
    }
}
