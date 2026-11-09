#include "pwm.h"

#include <rusefi/math.h>

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

void Pwm::SetDuty(int channel, float duty)
{
    auto dutyFloat = clampF(0, duty, 1);
    m_dutyFloat[channel] = dutyFloat;

    // Signed conversion is cheaper than unsigned with soft float, and the result fits (0 <= result <= period)
    SetHighTime(channel, (int32_t)(m_counterPeriod * dutyFloat));
}

void Pwm::SetHighTime(int channel, pwmcnt_t highTime)
{
    // Force highTime to be fully computed before pwmEnableChannel takes its lock. Soft float
    // calls are pure as far as the compiler knows, so otherwise LTO is free to sink the caller's
    // float math in to the critical section, needlessly extending the time interrupts are disabled.
    __asm__ volatile("" : "+r"(highTime));

    pwmEnableChannel(m_driver, channel, highTime);
}

float Pwm::GetLastDuty(int channel)
{
    return m_dutyFloat[channel];
}
