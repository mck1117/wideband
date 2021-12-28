#include "port_shared.h"

// board-specific stuff shared between bootloader and firmware

const CANConfig canConfig500 =
{
    CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP,
    0 // TODO: set bit timing! correctly!
};
