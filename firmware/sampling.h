#pragma once

struct ISampler
{
    virtual float GetNernstDc() const = 0;
    virtual float GetNernstAc() const = 0;
    virtual float GetPumpNominalCurrent() const = 0;
    virtual float GetInternalBatteryVoltage() const = 0;
    virtual float GetSensorTemperature() const = 0;
    virtual float GetSensorInternalResistance() const = 0;
};

// Get the sampler for a particular channel
const ISampler& GetSampler(int ch);

void StartSampling();

float GetInternalBatteryVoltage(int ch);
