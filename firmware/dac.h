#pragma once

#include <cstdint>

/* for DACConfig */
#include "hal.h"

#if HAL_USE_DAC

class Dac
{
public:
    Dac(DACDriver& driver);

    void Start(DACConfig& config);
    void SetVoltage(int channel, float voltage);
    float GetLastVoltage(int channel);

private:
    DACDriver* const m_driver;
    float m_voltageFloat[2];
};

#endif
