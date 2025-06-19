#include "dac.h"

/* for VCC_VOLTS */
#include "wideband_config.h"

#include <rusefi/rusefi_math.h>

#if HAL_USE_DAC

Dac::Dac(DACDriver& driver)
    : m_driver(&driver)
{
}

void Dac::Start(DACConfig& config)
{
    dacStart(m_driver, &config);
}

void Dac::SetVoltage(int channel, float voltage) {
    voltage = clampF(0, voltage, VCC_VOLTS);
    m_voltageFloat[channel] = voltage;

    dacPutChannelX(m_driver, channel, voltage / VCC_VOLTS * ((1 << 12) - 1));
}

float Dac::GetLastVoltage(int channel)
{
    return m_voltageFloat[channel];
}

#endif
