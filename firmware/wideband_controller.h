#pragma once

#include <cstddef>

struct ChannelAnalogResult;

class WidebandController
{
public:
    // SAMPLING: Convert ADC information in to O2 sensor information.
    void ProcessSample(const ChannelAnalogResult& result, float virtualGroundVoltageInt);
    float GetNernstDc() const;
    float GetNernstAc() const;
    float GetSensorInternalResistance() const;
    float GetSensorTemperature() const;
    float GetPumpNominalCurrent() const;
    float GetInternalBatteryVoltage() const;

    // LAMBDA CONVERSION: Convert to a useful lambda value.
    float GetLambda() const;
    float GetPhi(float pumpCurrent) const;
private:
    float NernstAc = 0;
    float NernstDc = 0;
    float PumpCurrentSenseVoltage = 0;

#ifdef BATTERY_INPUT_DIVIDER
    float InternalBatteryVoltage = 0;
#endif

    float r_2 = 0;
    float r_3 = 0;
};

WidebandController& GetController(size_t idx = 0);
