#include "fault.h"

static Fault currentFault = Fault::None;

void setFault(Fault fault)
{
    currentFault = fault;
}

bool hasFault()
{
    return currentFault == Fault::None;
}

Fault getCurrentFault()
{
    return currentFault;
}
