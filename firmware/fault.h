#pragma once

#include <cstdint>

enum class Fault : uint8_t
{
    None = 0,

    // First fault code at 3 so it's easier to see
    SensorDidntHeat = 3,
    SensorOverheat = 4,
    SensorUnderheat = 5,
};

void SetFault(Fault fault);
bool HasFault();
Fault GetCurrentFault();
