#pragma once

#include <cstdint>

/* for DACConfig */
#include "hal.h"

class Dac
{
public:
    Dac(DACDriver& driver);

    void Start(DACConfig& config);
    void SetVoltage(int channel, float volage);
    float GetLastVoltage(int channel);

private:
    DACDriver* const m_driver;
    float m_voltageFloat[2];
};
