#pragma once

enum class Fault
{
    None,
    SensorDidntHeat,
};

void setFault(Fault fault);
bool hasFault();
Fault getCurrentFault();
