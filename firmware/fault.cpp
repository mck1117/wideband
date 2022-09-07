#include "wideband_config.h"
#include "fault.h"

using namespace wbo;

static Fault currentFault[AFR_CHANNELS];

void SetFault(int ch, Fault fault)
{
    currentFault[ch] = fault;
}

Fault GetCurrentFault(int ch)
{
    return currentFault[ch];
}

bool HasFault()
{
    bool fault = false;

    for (int ch = 0; ch < AFR_CHANNELS; ch++)
        fault |= (GetCurrentFault(ch) != Fault::None);

    return fault;
}
