#pragma once

struct AnalogResult
{
    float NernstVoltage;
    float VirtualGroundVoltage;
    float PumpCurrentVoltage;
};

AnalogResult AnalogSample();
