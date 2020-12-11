#include "can.h"
#include "hal.h"

#include "can_helper.h"
#include "heater_control.h"
#include "lambda_conversion.h"
#include "sampling.h"

static const CANConfig canConfig500 =
{
    CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP | CAN_MCR_NART,
    CAN_BTR_SJW(0) | CAN_BTR_BRP(5)  | CAN_BTR_TS1(12) | CAN_BTR_TS2(1) | CAN_BTR_LBKM,
};

static THD_WORKING_AREA(waCanTxThread, 256);
void CanTxThread(void*)
{
    while(1)
    {
        float esr = GetSensorInternalResistance();
        float lambda = GetLambda();

        SendCanData(lambda, esr);
        SendEmulatedAemXseries(lambda, 0);

        chThdSleepMilliseconds(10);
    }
}

void InitCan()
{
    canStart(&CAND1, &canConfig500);
    chThdCreateStatic(waCanTxThread, sizeof(waCanTxThread), NORMALPRIO, CanTxThread, nullptr);
}

struct StandardDataFrame
{
    uint16_t lambda;
    uint16_t measuredResistance;
    uint8_t pad[4];
};

#define SWAP_UINT16(x) (((x) << 8) | ((x) >> 8))

void SendEmulatedAemXseries(float lambda, uint8_t idx) {
    CanTxMessage frame(0x180 + idx);

    bool isValid = IsRunningClosedLoop();

    uint16_t intLambda = lambda * 10000;

    // swap endian
    intLambda = SWAP_UINT16(intLambda);

    *reinterpret_cast<uint16_t*>(&frame[0]) = intLambda;

    // bit 1 = LSU 4.9 detected
    // bit 7 = reading valid
    frame[6] = 0x02 | (isValid ? 0x80 : 0x00);
}

void SendCanData(float lambda, uint16_t measuredResistance)
{
    CanTxTyped<StandardDataFrame> frame(0x130);

    frame.get().lambda = lambda * 10000;
    frame.get().measuredResistance = measuredResistance;
}
