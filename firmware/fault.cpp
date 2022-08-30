#include "fault.h"

using namespace wbo;

static Fault currentFault = Fault::None;

void SetFault(int ch, Fault fault)
{
    currentFault = fault;
}

bool HasFault()
{
    return currentFault != Fault::None;
}

Fault GetCurrentFault(int ch)
{
    return currentFault;
}
