#include "ch.h"
#include "hal.h"

#include "analog_input.h"
#include "can.h"
#include "pwm.h"
#include "pump_dac.h"

// 400khz / 1024 = 390hz PWM
// TODO: this is wired to an inverted output, what do?
Pwm heaterPwm(PWMD1, 0, 400'000, 1024);

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

    InitPumpDac();

    InitCan();

    uartStart(&UARTD1, &uartCfg);

    adcStart(&ADCD1, nullptr);

    heaterPwm.Start();

    heaterPwm.SetDuty(0.2f);

     while (true) {
//         auto result = AnalogSample();

//         // dummy data
//         SendCanData(0.5f, 300);

//         uartStartSend(&UARTD1, 13, "Hello, world!");
//         chThdSleepMilliseconds(10);


        SetPumpCurrentTarget(-1000);
        chThdSleepMilliseconds(10);
        
        SetPumpCurrentTarget(0);
        chThdSleepMilliseconds(10);

        SetPumpCurrentTarget(1000);
        chThdSleepMilliseconds(10);
    }
}
