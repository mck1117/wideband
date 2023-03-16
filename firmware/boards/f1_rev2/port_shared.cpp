#include "port_shared.h"

// board-specific stuff shared between bootloader and firmware

static const CANConfig canConfig500 =
{
    CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP,
    /*
     For 32MHz http://www.bittiming.can-wiki.info/ gives us Pre-scaler=4, Seq 1=13 and Seq 2=2. Subtract '1' for register values
    */
    CAN_BTR_SJW(0) | CAN_BTR_BRP(4 - 1)  | CAN_BTR_TS1(13 - 1) | CAN_BTR_TS2(2 - 1),
};

const CANConfig& GetCanConfig() {
    return canConfig500;
}
