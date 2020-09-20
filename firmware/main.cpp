#include "ch.h"
#include "hal.h"

#include "can.h"
#include "pwm.h"

// 400khz / 1024 = 390hz PWM
// TODO: this is wired to an inverted output, what do?
Pwm heaterPwm(PWMD1, 1, 400000, 1024);

// 48MHz / 1024 = 46.8khz PWM
Pwm pumpDac(PWMD3, 1, 48000000, 1024);

/*
 * Application entry point.
 */
int main(void) {
    halInit();
    chSysInit();

    InitCan();

    heaterPwm.Start();
    pumpDac.Start();

    heaterPwm.SetDuty(0.2f);
    pumpDac.SetDuty(0.4f);

    while (true) {
        // dummy data
        SendCanData(0.5f, 300);
        chThdSleepMilliseconds(10);
    }
}
