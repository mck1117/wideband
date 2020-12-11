#pragma once

enum class Fault
{
    None = 0,

    // First fault code at 3 so it's easier to see
    SensorDidntHeat = 3,
    SensorOverheat = 4,
};

void setFault(Fault fault);
bool hasFault();
Fault getCurrentFault();
