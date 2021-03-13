#include "port.h"

AnalogResult AnalogSample()
{
    // TODO: implement me!
    return {};
}

const CANConfig canConfig500 =
{
    CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP,
    0 // TODO: set bit timing! correctly!
};
