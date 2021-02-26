#pragma once

#include "hal.h"

struct AnalogResult
{
    float NernstVoltage;
    float PumpCurrentVoltage;
    float VirtualGroundVoltageInt;
};

AnalogResult AnalogSample();

extern const CANConfig canConfig500;
