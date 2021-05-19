#include "can.h"
#include "hal.h"

#include "can_helper.h"
#include "heater_control.h"
#include "lambda_conversion.h"
#include "sampling.h"
#include "pump_dac.h"
#include "port.h"

Configuration configuration;

static THD_WORKING_AREA(waCanTxThread, 256);
void CanTxThread(void*)
{
    while(1)
    {
        SendEmulatedAemXseries(configuration.CanIndexOffset);

        chThdSleepMilliseconds(10);
    }
}

static void SendAck()
{
    CANTxFrame frame;

    frame.IDE = CAN_IDE_EXT;
    frame.EID = 0x727573;   // ascii "rus"
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

        // If it's a bootloader entry request, reboot to the bootloader!
        if (frame.DLC == 0 && frame.EID == 0xEF0'0000)
        {
            SendAck();

            // Let the message get out before we reset the chip
            chThdSleep(50);

            NVIC_SystemReset();
        }
        // Check if it's an "index set" message
        else if (frame.DLC == 1 && frame.EID == 0xEF4'0000)
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

struct StandardDataFrame
{
    uint16_t lambda;
    uint16_t measuredResistance;
    uint8_t pad[4];
};

#define SWAP_UINT16(x) (((x) << 8) | ((x) >> 8))

void SendEmulatedAemXseries(uint8_t idx) {
    CanTxMessage frame(0x180 + idx, 8, true);

    bool isValid = IsRunningClosedLoop();

    float lambda = GetLambda();
    uint16_t intLambda = lambda * 10000;

    // swap endian
    intLambda = SWAP_UINT16(intLambda);
    *reinterpret_cast<uint16_t*>(&frame[0]) = intLambda;

    // bit 1 = LSU 4.9 detected
    // bit 7 = reading valid
    frame[6] = 0x02 | (isValid ? 0x80 : 0x00);

    // Hijack a reserved bit to indicate that we're NOT an AEM controller
    frame[7] = 0x80;

    // Now we embed some extra data for debug
    // bytes 2-3 are officially oxygen percent
    // byte 4 is officially supply voltage

    // Report pump output PWM in byte 2, 0-255 for min to max target (128 = 0 current)
    frame[2] = GetPumpOutputDuty() / 4;

    // Report sensor ESR in byte 3, 4 ohm steps
    int esrVal = (int)GetSensorInternalResistance() / 4;

    // Clamp to uint8_t limits
    if (esrVal > 255) {
        esrVal = 255;
    } else if (esrVal < 0) {
        esrVal = 0;
    }

    frame[3] = esrVal;

    // Report current nernst voltage in byte 4, 5mv steps
    frame[4] = (int)(GetNernstDc() * 200);
}

void SendCanData(float lambda, uint16_t measuredResistance)
{
    CanTxTyped<StandardDataFrame> frame(0x130);

    frame.get().lambda = lambda * 10000;
    frame.get().measuredResistance = measuredResistance;
}
