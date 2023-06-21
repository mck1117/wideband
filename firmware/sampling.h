#pragma once

struct ISampler
{
    virtual float GetNernstDc() const = 0;
    virtual float GetNernstAc() const = 0;
    virtual float GetPumpNominalCurrent() const = 0;
    virtual float GetInternalBatteryVoltage() const = 0;
};

// Get the sampler for a particular channel
ISampler& GetSampler(int ch);

void StartSampling();

float GetNernstAc(int ch);
float GetSensorInternalResistance(int ch);
float GetSensorTemperature(int ch);
float GetNernstDc(int ch);
float GetPumpNominalCurrent(int ch);
float GetInternalBatteryVoltage(int ch);
