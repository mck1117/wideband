#include "hal.h"

#include "can.h"

#include "fault.h"
#include "can_helper.h"
#include "heater_control.h"
#include "lambda_conversion.h"
#include "sampling.h"
#include "pump_dac.h"
#include "port.h"

// this same header is imported by rusEFI to get struct layouts and firmware version
#include "../for_rusefi/wideband_can.h"

static Configuration* configuration;

static THD_WORKING_AREA(waCanTxThread, 256);
void CanTxThread(void*)
{
    chRegSetThreadName("CAN Tx");

    // Current system time.
    systime_t prev = chVTGetSystemTime();

    while(1)
    {
        for (int ch = 0; ch < AFR_CHANNELS; ch++)
        {
            SendCanForChannel(ch);
        }

        prev = chThdSleepUntilWindowed(prev, chTimeAddX(prev, TIME_MS2I(WBO_TX_PERIOD_MS)));
    }
}

static void SendAck()
{
    CANTxFrame frame;

#ifdef STM32G4XX
    frame.common.RTR = 0;
#else // Not CAN FD
    frame.RTR = CAN_RTR_DATA;
#endif

    CAN_EXT(frame) = 1;
    CAN_EID(frame) = WB_ACK;
    frame.DLC = 0;

    canTransmitTimeout(&CAND1, CAN_ANY_MAILBOX, &frame, TIME_INFINITE);
}

// Start in Unknown state. If no CAN message is ever received, we operate
// on internal battery sense etc.
static HeaterAllow heaterAllow = HeaterAllow::Unknown;
static float remoteBatteryVoltage = 0;

static THD_WORKING_AREA(waCanRxThread, 512);
void CanRxThread(void*)
{
    chRegSetThreadName("CAN Rx");

    while(1)
    {
        CANRxFrame frame;
        msg_t msg = canReceiveTimeout(&CAND1, CAN_ANY_MAILBOX, &frame, TIME_INFINITE);

        // Ignore non-ok results...
        if (msg != MSG_OK)
        {
            continue;
        }

        // Ignore std frames, only listen to ext
        if (!CAN_EXT(frame))
        {
            continue;
        }

        if (frame.DLC == 2 && CAN_ID(frame) == WB_MGS_ECU_STATUS)
        {
            // This is status from ECU - battery voltage and heater enable signal

            // data1 contains heater enable bit
            if ((frame.data8[1] & 0x1) == 0x1)
            {
                heaterAllow = HeaterAllow::Allowed;
            }
            else
            {
                heaterAllow = HeaterAllow::NotAllowed;
            }

            // data0 contains battery voltage in tenths of a volt
            float vbatt = frame.data8[0] * 0.1f;
            if (vbatt < 5)
            {
                // provided vbatt is bogus, default to 14v nominal
                remoteBatteryVoltage = 14;
            }
            else
            {
                remoteBatteryVoltage = vbatt;
            }
        }
        // If it's a bootloader entry request, reboot to the bootloader!
        else if ((frame.DLC == 0 || frame.DLC == 1) && CAN_ID(frame) == WB_BL_ENTER)
        {
            // If 0xFF (force update all) or our ID, reset to bootloader, otherwise ignore
            if (frame.DLC == 0 || frame.data8[0] == 0xFF || frame.data8[0] == GetConfiguration()->afr[0].RusEfiIdOffset)
            {
                SendAck();

                // Let the message get out before we reset the chip
                chThdSleep(50);

                NVIC_SystemReset();
            }
        }
        // Check if it's an "index set" message
        else if (frame.DLC == 1 && CAN_ID(frame) == WB_MSG_SET_INDEX)
        {
            int offset = frame.data8[0];
            configuration = GetConfiguration();
            for (int i = 0; i < AFR_CHANNELS; i++) {
                configuration->afr[i].RusEfiIdOffset = offset + i * 2;
            }
            for (int i = 0; i < EGT_CHANNELS; i++) {
                configuration->egt[i].RusEfiIdOffset = offset + i;
            }
            SetConfiguration();
            SendAck();
        }
    }
}

HeaterAllow GetHeaterAllowed()
{
    return heaterAllow;
}

float GetRemoteBatteryVoltage()
{
    return remoteBatteryVoltage;
}

void InitCan()
{
    configuration = GetConfiguration();

    canStart(&CAND1, &GetCanConfig());
    chThdCreateStatic(waCanTxThread, sizeof(waCanTxThread), NORMALPRIO, CanTxThread, nullptr);
    chThdCreateStatic(waCanRxThread, sizeof(waCanRxThread), NORMALPRIO - 4, CanRxThread, nullptr);
}

void SendRusefiFormat(uint8_t ch)
{
    auto baseAddress = WB_DATA_BASE_ADDR + configuration->afr[ch].RusEfiIdOffset;

    const auto& sampler = GetSampler(ch);
    const auto& heater = GetHeaterController(ch);

    auto nernstDc = sampler.GetNernstDc();
    auto pumpDuty = GetPumpOutputDuty(ch);
    auto lambda = GetLambda(ch);

    // Lambda is valid if:
    // 1. Nernst voltage is near target
    // 2. Pump driver isn't slammed in to the stop
    // 3. Lambda is >0.6 (sensor isn't specified below that)
    bool lambdaValid =
            nernstDc > (NERNST_TARGET - 0.1f) && nernstDc < (NERNST_TARGET + 0.1f) &&
            pumpDuty > 0.1f && pumpDuty < 0.9f &&
            lambda > 0.6f;

    if (configuration->afr[ch].RusEfiTx) {
        CanTxTyped<wbo::StandardData> frame(baseAddress + 0);

        // The same header is imported by the ECU and checked against this data in the frame
        frame.get().Version = RUSEFI_WIDEBAND_VERSION;

        uint16_t lambdaInt = lambdaValid ? (lambda * 10000) : 0;
        frame.get().Lambda = lambdaInt;
        frame.get().TemperatureC = sampler.GetSensorTemperature();
        bool heaterClosedLoop = heater.IsRunningClosedLoop();
        frame.get().Valid = (heaterClosedLoop && lambdaValid) ? 0x01 : 0x00;
    }

    if (configuration->afr[ch].RusEfiTxDiag) {
        CanTxTyped<wbo::DiagData> frame(baseAddress + 1);;

        frame.get().Esr = sampler.GetSensorInternalResistance();
        frame.get().NernstDc = nernstDc * 1000;
        frame.get().PumpDuty = pumpDuty * 255;
        frame.get().Status = GetCurrentFault(ch);
        frame.get().HeaterDuty = GetHeaterDuty(ch) * 255;
    }
}

// Weak link so boards can override it
__attribute__((weak)) void SendCanForChannel(uint8_t ch)
{
    SendRusefiFormat(ch);
}
