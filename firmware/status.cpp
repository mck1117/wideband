#include "wideband_config.h"
#include "status.h"

using namespace wbo;

static Status currentStatus[AFR_CHANNELS];

void SetStatus(int ch, Status fault)
{
    currentStatus[ch] = fault;
}

Status GetCurrentStatus(int ch)
{
    return currentStatus[ch];
}

bool HasFault()
{
    bool fault = false;

    for (int ch = 0; ch < AFR_CHANNELS; ch++) {
        fault |= (GetCurrentStatus(ch) >= Status::SensorDidntHeat);
    }

    return fault;
}
