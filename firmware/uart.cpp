#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "uart.h"

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

static char printBuffer[200];

static THD_WORKING_AREA(waUartThread, 256);
static void UartThread(void*)
{

}

void InitUart()
{
    uartStart(&UARTD1, &uartCfg);

    chThdCreateStatic(waUartThread, sizeof(waUartThread), NORMALPRIO, UartThread, nullptr);
}
