#include "port_shared.h"

// board-specific stuff shared between bootloader and firmware

const CANConfig canConfig500 =
{
    CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP,
    CAN_BTR_SJW(0) | CAN_BTR_BRP(6)  | CAN_BTR_TS1(8)  | CAN_BTR_TS2(1)
};
