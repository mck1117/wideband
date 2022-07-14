#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "lambda_conversion.h"
#include "sampling.h"
#include "heater_control.h"
#include "fault.h"
#include "uart.h"

#include "tunerstudio.h"
#include "tunerstudio_io.h"
#include "wideband_board_config.h"

#ifdef UART_DEBUG
// just a reminder that we have either TS connectivity or this UART_DEBUG but not both

SerialConfig cfg = {
    .speed = 115200,
    .cr1 = 0,
    .cr2 = USART_CR2_STOP1_BITS | USART_CR2_LINEN,
    .cr3 = 0
};

static char printBuffer[200];

static THD_WORKING_AREA(waUartThread, 512);
static void UartThread(void*)
{
    // in UART_DEBUG mode we only support Serial - this file name here has a bit of a confusing naming
    sdStart(&SD1, &cfg);

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
        chnWrite(&SD1, (const uint8_t *)printBuffer, writeCount);

        chThdSleepMilliseconds(50);
    }
}

#elif defined(TS_PRIMARY_UART_PORT) || defined(TS_PRIMARY_SERIAL_PORT)

#ifdef TS_PRIMARY_UART_PORT
static UartTsChannel primaryChannel(TS_PRIMARY_UART_PORT);
#endif

#ifdef TS_PRIMARY_SERIAL_PORT
static SerialTsChannel primaryChannel(TS_PRIMARY_SERIAL_PORT);
#endif

struct PrimaryChannelThread : public TunerstudioThread {
    PrimaryChannelThread() : TunerstudioThread("Primary TS Channel") { }

    TsChannelBase* setupChannel() {
        primaryChannel.start(TS_PRIMARY_BAUDRATE);

        return &primaryChannel;
    }
};

static PrimaryChannelThread primaryChannelThread;

#endif

void InitUart()
{
#ifdef UART_DEBUG
    chThdCreateStatic(waUartThread, sizeof(waUartThread), NORMALPRIO, UartThread, nullptr);
#elif defined(TS_PRIMARY_UART_PORT) || defined(TS_PRIMARY_SERIAL_PORT)
    primaryChannelThread.Start();
#endif
}
