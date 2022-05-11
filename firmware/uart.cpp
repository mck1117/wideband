#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "lambda_conversion.h"
#include "sampling.h"
#include "heater_control.h"
#include "fault.h"
#include "uart.h"

static const UARTConfig uartCfg =
{
    .txend1_cb = nullptr,
    .txend2_cb = nullptr,
    .rxend_cb = nullptr,
    .rxchar_cb = nullptr,
    .rxerr_cb = nullptr,
    .timeout_cb = nullptr,

#ifdef STM32F0XX
    .timeout = 0,
#endif

    .speed = 115200,
    .cr1 = 0,
    .cr2 = 0,
    .cr3 = 0,
};

static char printBuffer[200];

static THD_WORKING_AREA(waUartThread, 512);
static void UartThread(void*)
{
    while(true)
    {
        float lambda = GetLambda();
        int lambdaIntPart = lambda;
        int lambdaThousandths = (lambda - lambdaIntPart) * 1000;
        int batteryVoltageMv = GetInternalBatteryVoltage() * 1000;
        int duty = GetHeaterDuty() * 100;

        size_t writeCount = chsnprintf(printBuffer, 200,
            "%d.%03d\tAC %d mV\tR: %d\tT: %d\tIpump: %d\tVbat: %d\theater: %s (%d)\tfault: %s\r\n",
            lambdaIntPart, lambdaThousandths,
            (int)(GetNernstAc() * 1000.0),
            (int)GetSensorInternalResistance(),
            (int)GetSensorTemperature(),
            (int)(GetPumpNominalCurrent() * 1000),
            batteryVoltageMv,
            describeHeaterState(GetHeaterState()), duty,
            describeFault(GetCurrentFault()));
        uartStartSend(&UARTD1, writeCount, printBuffer);

        chThdSleepMilliseconds(50);
    }
}

void InitUart()
{
    uartStart(&UARTD1, &uartCfg);

    chThdCreateStatic(waUartThread, sizeof(waUartThread), NORMALPRIO, UartThread, nullptr);
}
