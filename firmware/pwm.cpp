#include "pwm.h"

#include <rusefi/rusefi_math.h>

Pwm::Pwm(PWMDriver& driver)
    : m_driver(&driver)
    , m_counterFrequency(0)
    , m_counterPeriod(0)
{
}

void Pwm::Start(const PWMConfig& config)
{
    m_counterFrequency = config.frequency;
    m_counterPeriod = config.period;

    pwmStart(m_driver, &config);
}

void Pwm::SetDuty(int channel, float duty) {
    auto dutyFloat = clampF(0, duty, 1);
    m_dutyFloat[channel] = dutyFloat;
    pwmcnt_t highTime = m_counterPeriod * dutyFloat;

    pwmEnableChannel(m_driver, channel, highTime);
}

float Pwm::GetLastDuty(int channel)
{
    return m_dutyFloat[channel];
}
