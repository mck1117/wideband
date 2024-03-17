#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "lambda_conversion.h"
#include "sampling.h"
#include "heater_control.h"
#include "max3185x.h"
#include "fault.h"
#include "uart.h"

#include "tunerstudio.h"
#include "tunerstudio_io.h"
#include "wideband_board_config.h"

#ifdef DEBUG_SERIAL_PORT

SerialConfig cfg = {
    .speed = DEBUG_SERIAL_BAUDRATE,
    .cr1 = 0,
    .cr2 = USART_CR2_STOP1_BITS | USART_CR2_LINEN,
    .cr3 = 0
};

static char printBuffer[200];

static THD_WORKING_AREA(waUartThread, 512);
static void UartThread(void*)
{
    chRegSetThreadName("UART debug");

    sdStart(&SD1, &cfg);

    while(true)
    {
        int ch;

        for (ch = 0; ch < AFR_CHANNELS; ch++) {
            float lambda = GetLambda(ch);
            int lambdaIntPart = lambda;
            int lambdaThousandths = (lambda - lambdaIntPart) * 1000;
            int batteryVoltageMv = GetSampler(ch).GetInternalHeaterVoltage() * 1000;
            int duty = GetHeaterDuty(ch) * 100;

            size_t writeCount = chsnprintf(printBuffer, 200,
                "[AFR%d]: %d.%03d DC: %4d mV AC: %4d mV ESR: %5d T: %4d C Ipump: %6d uA Vheater: %5d heater: %s (%d)\tfault: %s\r\n",
                ch,
                lambdaIntPart, lambdaThousandths,
                (int)(GetSampler(ch).GetNernstDc(ch) * 1000.0),
                (int)(GetSampler(ch).GetNernstAc(ch) * 1000.0),
                (int)GetSampler(ch).GetSensorInternalResistance(ch),
                (int)GetSampler(ch).GetSensorTemperature(ch),
                (int)(GetSampler(ch).GetPumpNominalCurrent(ch) * 1000),
                batteryVoltageMv,
                describeHeaterState(GetHeaterState(ch)), duty,
                describeFault(GetCurrentFault(ch)));
            chnWrite(&SD1, (const uint8_t *)printBuffer, writeCount);
        }

#if (EGT_CHANNELS > 0)
        for (ch = 0; ch < EGT_CHANNELS; ch++) {
            size_t writeCount = chsnprintf(printBuffer, 200,
                "EGT[%d]: %d C (int %d C)\r\n",
                (int)getEgtDrivers()[ch].temperature,
                (int)getEgtDrivers()[ch].coldJunctionTemperature);
            chnWrite(&SD1, (const uint8_t *)printBuffer, writeCount);
        }
#endif /* EGT_CHANNELS > 0 */

        chThdSleepMilliseconds(100);
    }
}

#endif /* DEBUG_SERIAL_PORT */

#ifdef TS_ENABLED

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

#ifdef TS_SECONDARY_SERIAL_PORT
static SerialTsChannel secondaryChannel(TS_SECONDARY_SERIAL_PORT);

struct SecondaryChannelThread : public TunerstudioThread {
    SecondaryChannelThread() : TunerstudioThread("Secondary TS Channel") { }

    TsChannelBase* setupChannel() {
        secondaryChannel.start(TS_SECONDARY_BAUDRATE);

        return &secondaryChannel;
    }
};

static SecondaryChannelThread secondaryChannelThread;

#endif /* TS_SECONDARY_SERIAL_PORT */
#endif /* TS_ENABLED */

void InitUart()
{
#ifdef DEBUG_SERIAL_PORT
    chThdCreateStatic(waUartThread, sizeof(waUartThread), NORMALPRIO, UartThread, nullptr);
#endif
#ifdef TS_ENABLED
    primaryChannelThread.Start();
#ifdef TS_SECONDARY_SERIAL_PORT
    secondaryChannelThread.Start();
#endif
#endif
}
