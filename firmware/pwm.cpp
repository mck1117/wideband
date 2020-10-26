#include "pwm.h"

#include "hal.h"

Pwm::Pwm(PWMDriver& driver, uint8_t channel, uint32_t counterFrequency, uint32_t counterPeriod)
    : m_driver(&driver)
    , m_channel(channel)
    , m_counterFrequency(counterFrequency)
    , m_counterPeriod(counterPeriod)
{
}

void Pwm::Start()
{
    PWMConfig config = {
        m_counterFrequency,
        m_counterPeriod,
        nullptr,
        {
            {PWM_OUTPUT_ACTIVE_HIGH, nullptr},
            {PWM_OUTPUT_ACTIVE_HIGH, nullptr},
            {PWM_OUTPUT_ACTIVE_HIGH, nullptr},
            {PWM_OUTPUT_ACTIVE_HIGH, nullptr}
        },
        0,
        0
    };

    pwmStart(m_driver, &config);
}

void Pwm::SetDuty(float duty) {
    pwmcnt_t highTime = m_counterPeriod * duty;

    pwm_lld_enable_channel(m_driver, m_channel, highTime);
}
