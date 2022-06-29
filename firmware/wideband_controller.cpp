#include "wideband_config.h"
#include "wideband_controller.h"

static WidebandController controllers[AFR_CHANNELS];

WidebandController& GetController(size_t idx)
{
    return controllers[idx];
}

WidebandController::WidebandController()
    : heaterPid(
        0.3f,      // kP
        0.3f,      // kI
        0.01f,     // kD
        3.0f,      // Integrator clamp (volts)
        HEATER_CONTROL_PERIOD
    )
{
}
