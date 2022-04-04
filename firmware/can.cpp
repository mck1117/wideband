#include "can.h"
#include "hal.h"

#include "fault.h"
#include "can_helper.h"
#include "heater_control.h"
#include "lambda_conversion.h"
#include "sampling.h"
#include "pump_dac.h"
#include "port.h"

// this same header is imported by rusEFI to get struct layouts and firmware version
#include "../for_rusefi/wideband_can.h"

Configuration configuration;

static THD_WORKING_AREA(waCanTxThread, 256);
void CanTxThread(void*)
{
    while(1)
    {
        SendRusefiFormat(configuration.CanIndexOffset);

        chThdSleepMilliseconds(10);
    }
}

static void SendAck()
{
    CANTxFrame frame;

    frame.IDE = CAN_IDE_EXT;
    frame.EID = WB_ACK;
    frame.RTR = CAN_RTR_DATA;
    frame.DLC = 0;

    canTransmitTimeout(&CAND1, CAN_ANY_MAILBOX, &frame, TIME_INFINITE);
}

static THD_WORKING_AREA(waCanRxThread, 256);
void CanRxThread(void*)
{
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
        if (frame.IDE != CAN_IDE_EXT)
        {
            continue;
        }

        if (frame.DLC == 2 && frame.EID == WB_MGS_ECU_STATUS) {
            // This is status from ECU - battery voltage and heater enable signal

            // data0 contains battery voltage in tenths of a volt
            float vbatt = frame.data8[0] * 0.1f;
            SetBatteryVoltage(vbatt);

            // data1 contains heater enable bit
            bool heaterAllowed = (frame.data8[1] & 0x1) == 0x1;
            SetHeaterAllowed(heaterAllowed);
        }
        // If it's a bootloader entry request, reboot to the bootloader!
        else if (frame.DLC == 0 && frame.EID == WB_BL_ENTER)
        {
            SendAck();

            // Let the message get out before we reset the chip
            chThdSleep(50);

            NVIC_SystemReset();
        }
        // Check if it's an "index set" message
        else if (frame.DLC == 1 && frame.EID == WB_MSG_SET_INDEX)
        {
            auto newCfg = GetConfiguration();
            newCfg.CanIndexOffset = frame.data8[0];
            SetConfiguration(newCfg);
            configuration = GetConfiguration();
            SendAck();
        }
    }
}

void InitCan()
{
    configuration = GetConfiguration();

    canStart(&CAND1, &canConfig500);
    chThdCreateStatic(waCanTxThread, sizeof(waCanTxThread), NORMALPRIO, CanTxThread, nullptr);
    chThdCreateStatic(waCanRxThread, sizeof(waCanRxThread), NORMALPRIO - 4, CanRxThread, nullptr);
}

#define SWAP_UINT16(x) (((x) << 8) | ((x) >> 8))

void SendRusefiFormat(uint8_t idx)
{
    auto baseAddress = 0x190 + 2 * idx;
    auto esr = GetSensorInternalResistance();

    {
        CanTxTyped<wbo::StandardData> frame(baseAddress + 0);

        // The same header is imported by the ECU and checked against this data in the frame
        frame.get().Version = RUSEFI_WIDEBAND_VERSION;

        uint16_t lambda = GetLambda() * 10000;
        frame.get().Lambda = lambda;

        // TODO: decode temperature instead of putting ESR here
        frame.get().TemperatureC = esr;

        frame.get().Valid = IsRunningClosedLoop() ? 0x01 : 0x00;
    }

    {
        CanTxTyped<wbo::DiagData> frame(baseAddress + 1);

        frame.get().Esr = esr;
        frame.get().NernstDc = GetNernstDc() * 1000;
        frame.get().PumpDuty = GetPumpOutputDuty() * 255;
        frame.get().Status = GetCurrentFault();
        frame.get().HeaterDuty = GetHeaterDuty() * 255;
    }
}
