#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "can.h"
#include "heater_control.h"
#include "pump_dac.h"
#include "sampling.h"

static const UARTConfig uartCfg =
{
    .txend1_cb = nullptr,
    .txend2_cb = nullptr,
    .rxend_cb = nullptr,
    .rxchar_cb = nullptr,
    .rxerr_cb = nullptr,
    .timeout_cb = nullptr,

    .timeout = 0,
    .speed = 500000,
    .cr1 = 0,
    .cr2 = 0,
    .cr3 = 0,
};

char strBuffer[200];

/*
 * Application entry point.
 */
int main() {
    halInit();
    chSysInit();

    StartSampling();

    InitPumpDac();

    InitCan();

    uartStart(&UARTD1, &uartCfg);

    StartHeaterControl();

    /*for (int i = 0; i < 500; i++) {
        SetPumpCurrentTarget(current);
        chThdSleepMilliseconds(50);

        auto result = AnalogSample();

        //size_t writeCount = chsnprintf(strBuffer, 200, "I: %d\t\tVM: %.3f\tIp: %.3f\n", current, result.VirtualGroundVoltage, result.PumpCurrentVoltage);
        size_t writeCount = chsnprintf(strBuffer, 200, "%d\t%.4f\n", current, result.PumpCurrentVoltage);
        uartStartSend(&UARTD1, writeCount, strBuffer);

        //current += 10;
    }*/

    while(1) {
        size_t writeCount = chsnprintf(strBuffer, 200, "%.4f\t%.2f\t%.2f\n", GetSensorInternalResistance(), nernstVolt, GetPumpNominalCurrent());
        uartStartSend(&UARTD1, writeCount, strBuffer);

        chThdSleepMilliseconds(5);
    }
}
