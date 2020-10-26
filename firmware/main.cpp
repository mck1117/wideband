#include "ch.h"
#include "hal.h"

#include "analog_input.h"
#include "can.h"
#include "pwm.h"

// 400khz / 1024 = 390hz PWM
// TODO: this is wired to an inverted output, what do?
Pwm heaterPwm(PWMD1, 0, 400'000, 1024);

// 48MHz / 1024 = 46.8khz PWM
Pwm pumpDac(PWMD3, 0, 48'000'000, 1024);

/*
 * Application entry point.
 */
int main() {
    halInit();
    chSysInit();

    InitCan();

    palSetPadMode(GPIOA, 6, PAL_MODE_ALTERNATE(1));

    adcStart(&ADCD1, nullptr);

    heaterPwm.Start();
    pumpDac.Start();

    heaterPwm.SetDuty(0.2f);
    pumpDac.SetDuty(0.4f);

    while (true) {
        auto result = AnalogSample();

        // dummy data
        SendCanData(0.5f, 300);
        chThdSleepMilliseconds(10);
    }
}
