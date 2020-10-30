#include "can.h"
#include "hal.h"

#include "can_helper.h"

static const CANConfig canConfig500 =
{
    CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP | CAN_MCR_NART,
    CAN_BTR_SJW(0) | CAN_BTR_BRP(5)  | CAN_BTR_TS1(12) | CAN_BTR_TS2(1) | CAN_BTR_LBKM,
};

void InitCan()
{
    canStart(&CAND1, &canConfig500);
}

struct StandardDataFrame
{
    uint16_t lambda;
    uint16_t measuredResistance;
    uint8_t pad[4];
};

void SendCanData(float lambda, uint16_t measuredResistance)
{
    CanTxTyped<StandardDataFrame> frame(0x130);

    frame.get().lambda = lambda / 1000.0f;
    frame.get().measuredResistance = measuredResistance;
}
