#pragma once

struct AnalogResult
{
    float NernstVoltage;
    float PumpCurrentVoltage;
    float VirtualGroundVoltageInt;
};

AnalogResult AnalogSample();
