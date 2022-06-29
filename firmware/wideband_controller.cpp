#include "wideband_config.h"
#include "wideband_controller.h"

static WidebandController controllers[AFR_CHANNELS];

WidebandController& GetController(size_t idx)
{
    return controllers[idx];
}
