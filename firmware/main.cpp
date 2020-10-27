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

static const UARTConfig uartCfg =
{
    .txend1_cb = nullptr,
    .txend2_cb = nullptr,
    .rxend_cb = nullptr,
    .rxchar_cb = nullptr,
    .rxerr_cb = nullptr,
    .timeout_cb = nullptr,

    .timeout = 0,
    .speed = 230400,
    .cr1 = 0,
    .cr2 = 0,
    .cr3 = 0,
};

/*
 * Application entry point.
 */
int main() {
    halInit();
    chSysInit();

    InitCan();

    uartStart(&UARTD1, &uartCfg);

    adcStart(&ADCD1, nullptr);

    heaterPwm.Start();
    pumpDac.Start();

    heaterPwm.SetDuty(0.2f);
    pumpDac.SetDuty(0.4f);

    while (true) {
        auto result = AnalogSample();

        // dummy data
        SendCanData(0.5f, 300);

        uartStartSend(&UARTD1, 13, "Hello, world!");
        chThdSleepMilliseconds(10);
    }
}
