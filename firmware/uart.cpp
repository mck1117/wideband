#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "lambda_conversion.h"
#include "sampling.h"
#include "heater_control.h"
#include "max3185x.h"
#include "status.h"
#include "uart.h"
#include "pump_dac.h"

#include "tunerstudio.h"
#include "tunerstudio_io.h"
#include "wideband_board_config.h"

#ifndef PORT_EXTRA_SERIAL_CR2
#define PORT_EXTRA_SERIAL_CR2 0
#endif

#ifdef DEBUG_SERIAL_PORT

SerialConfig cfg = {
    .speed = DEBUG_SERIAL_BAUDRATE,
    .cr1 = 0,
    .cr2 = USART_CR2_STOP1_BITS | PORT_EXTRA_SERIAL_CR2,
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

        #ifdef BOARD_HAS_VOLTAGE_SENSE
        {
            float vbatt = GetSupplyVoltage();

            int vbattIntPart = vbatt;
            int vbattTenths = (vbatt - vbattIntPart) * 10;

            int tempC = GetMcuTemperature();

            size_t writeCount = chsnprintf(printBuffer, sizeof(printBuffer), "Board: VBatt %d.%01d Temp %d deg C\r\n", vbattIntPart, vbattTenths, tempC);
            chnWrite(&SD1, (const uint8_t *)printBuffer, writeCount);
        }
        #endif

        for (ch = 0; ch < AFR_CHANNELS; ch++) {
            float lambda = GetLambda(ch);
            int lambdaIntPart = lambda;
            int lambdaThousandths = (lambda - lambdaIntPart) * 1000;
            int heaterVoltageMv = GetSampler(ch).GetInternalHeaterVoltage() * 1000;
            int heaterDuty = GetHeaterDuty(ch) * 100;
            int pumpDuty = GetPumpOutputDuty(ch) * 100;

            size_t writeCount = chsnprintf(printBuffer, sizeof(printBuffer),
                "[AFR%d]: %d.%03d DC: %4d mV AC: %4d mV ESR: %5d T: %4d C Ipump: %6d uA PumpDac: %3d Vheater: %5d heater: %s (%d)\tfault: %s\r\n",
                ch,
                lambdaIntPart, lambdaThousandths,
                (int)(GetSampler(ch).GetNernstDc() * 1000.0),
                (int)(GetSampler(ch).GetNernstAc() * 1000.0),
                (int)GetSampler(ch).GetSensorInternalResistance(),
                (int)GetSampler(ch).GetSensorTemperature(),
                (int)(GetSampler(ch).GetPumpNominalCurrent() * 1000),
                pumpDuty,
                heaterVoltageMv,
                describeHeaterState(GetHeaterState(ch)), heaterDuty,
                describeStatus(GetCurrentStatus(ch)));
            chnWrite(&SD1, (const uint8_t *)printBuffer, writeCount);
        }

#if (EGT_CHANNELS > 0)
        for (ch = 0; ch < EGT_CHANNELS; ch++) {
            size_t writeCount = chsnprintf(printBuffer, sizeof(printBuffer),
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

#if defined(TS_ENABLED) && TS_ENABLED != 0

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
#if defined(TS_ENABLED) && TS_ENABLED != 0
    primaryChannelThread.Start();
#ifdef TS_SECONDARY_SERIAL_PORT
    secondaryChannelThread.Start();
#endif
#endif
}
