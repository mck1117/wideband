#include "fault.h"

using namespace wbo;

static Fault currentFault = Fault::None;

void SetFault(Fault fault)
{
    currentFault = fault;
}

bool HasFault()
{
    return currentFault != Fault::None;
}

Fault GetCurrentFault()
{
    return currentFault;
}
