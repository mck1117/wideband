#pragma once

#include "wideband_config.h"

#include "timer.h"

struct ISampler
{
    virtual float GetNernstDc() const = 0;
    virtual float GetNernstAc() const = 0;
    virtual float GetPumpNominalCurrent() const = 0;
    virtual float GetInternalBatteryVoltage() const = 0;
    virtual float GetSensorTemperature() const = 0;
    virtual float GetSensorInternalResistance() const = 0;
};

struct AnalogChannelResult;

class Sampler : public ISampler
{
public:
    void ApplySample(AnalogChannelResult& result, float virtualGroundVoltageInt);
    void Init();

    float GetNernstDc() const override;
    float GetNernstAc() const override;
    float GetPumpNominalCurrent() const override;
    float GetInternalBatteryVoltage() const override;
    float GetSensorTemperature() const override;
    float GetSensorInternalResistance() const override;

private:
    float r_2 = 0;
    float r_3 = 0;

    float nernstAc = 0;
    float nernstDc = 0;
    float pumpCurrentSenseVoltage = 0;

#ifdef BATTERY_INPUT_DIVIDER
    float internalBatteryVoltage = 0;
#endif

    Timer m_startupTimer;
};

// Get the sampler for a particular channel
const ISampler& GetSampler(int ch);

void StartSampling();
