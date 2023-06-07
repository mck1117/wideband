#include "port_shared.h"

// board-specific stuff shared between bootloader and firmware

static const CANConfig canConfig125 =
{
    CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP,
    /*
     For 36MHz http://www.bittiming.can-wiki.info/ gives us Pre-scaler=18, Seq 1=13 and Seq 2=2. Subtract '1' for register values
    */
    CAN_BTR_BRP(17) | CAN_BTR_TS1(12) | CAN_BTR_TS2(1) | CAN_BTR_SJW(0),
};

static const CANConfig canConfig250 =
{
    CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP,
    /*
     For 36MHz http://www.bittiming.can-wiki.info/ gives us Pre-scaler=9, Seq 1=13 and Seq 2=2. Subtract '1' for register values
    */
    CAN_BTR_BRP(8) | CAN_BTR_TS1(12) | CAN_BTR_TS2(1) | CAN_BTR_SJW(0),
};

static const CANConfig canConfig500 =
{
    CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP,
    /*
     For 36MHz http://www.bittiming.can-wiki.info/ gives us Pre-scaler=4, Seq 1=15 and Seq 2=2. Subtract '1' for register values
    */
    CAN_BTR_BRP(3) | CAN_BTR_TS1(14) | CAN_BTR_TS2(1) | CAN_BTR_SJW(0),
};

static const CANConfig canConfig1000 =
{
    CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP,
    /*
     For 36MHz http://www.bittiming.can-wiki.info/ gives us Pre-scaler=2, Seq 1=15 and Seq 2=2. Subtract '1' for register values
    */
    CAN_BTR_BRP(1) | CAN_BTR_TS1(14) | CAN_BTR_TS2(1) | CAN_BTR_SJW(0),
};


const CANConfig& GetCanConfig() {
    return canConfig500;
}
