#pragma once

#include "hal.h"
#include "port_shared.h"

struct AnalogResult
{
    float NernstVoltage;
    float PumpCurrentVoltage;
    float VirtualGroundVoltageInt;
    float BatteryVoltage;
};

AnalogResult AnalogSample();

class Configuration {
private:
    // Increment this any time the configuration format changes
    // It is stored along with the data to ensure that it has been written before
    static constexpr uint32_t ExpectedTag = 0xDEADBE01;
    uint32_t Tag = ExpectedTag;

public:
    bool IsValid() const
    {
        return this->Tag == ExpectedTag;
    }

    // Actual configuration data
    uint8_t CanIndexOffset = 0;

    uint8_t pad[3];
};

Configuration GetConfiguration();
void SetConfiguration(const Configuration& newConfig);

