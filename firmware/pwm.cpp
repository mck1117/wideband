#include "pwm.h"

Pwm::Pwm(PWMDriver& driver, uint8_t channel, uint32_t counterFrequency, uint32_t counterPeriod)
    : m_driver(&driver)
    , m_channel(channel)
    , m_counterFrequency(counterFrequency)
    , m_counterPeriod(counterPeriod)
{
}

Pwm::Pwm(PWMDriver& driver)
    : m_driver(&driver)
    , m_channel(0)
    , m_counterFrequency(0)
    , m_counterPeriod(0)
{
}

void Pwm::Start()
{
    PWMConfig config = {
        m_counterFrequency,
        m_counterPeriod,
        nullptr,
        {
            {PWM_OUTPUT_ACTIVE_HIGH | PWM_COMPLEMENTARY_OUTPUT_ACTIVE_LOW, nullptr},
            {PWM_OUTPUT_ACTIVE_HIGH | PWM_COMPLEMENTARY_OUTPUT_ACTIVE_LOW, nullptr},
            {PWM_OUTPUT_ACTIVE_HIGH | PWM_COMPLEMENTARY_OUTPUT_ACTIVE_LOW, nullptr},
            {PWM_OUTPUT_ACTIVE_HIGH | PWM_COMPLEMENTARY_OUTPUT_ACTIVE_LOW, nullptr}
        },
        0,
        0,
#if STM32_PWM_USE_ADVANCED
        0
#endif
    };

    pwmStart(m_driver, &config);
}

void Pwm::Start(PWMConfig *config)
{
    m_counterFrequency = config->frequency;
    m_counterPeriod = config->period;

    pwmStart(m_driver, config);
}

float maxF(float i1, float i2) {
    return i1 > i2 ? i1 : i2;
}

float minF(float i1, float i2) {
    return i1 < i2 ? i1 : i2;
}

float clampF(float min, float clamp, float max) {
    return maxF(min, minF(clamp, max));
}

void Pwm::SetDuty(int channel, float duty) {
    auto dutyFloat = clampF(0, duty, 1);
    m_dutyFloat = dutyFloat;
    pwmcnt_t highTime = m_counterPeriod * dutyFloat;

    pwm_lld_enable_channel(m_driver, channel, highTime);
}

void Pwm::SetDuty(float duty) {
    SetDuty(m_channel, duty);
}

float Pwm::GetLastDuty() const
{
    return m_dutyFloat;
}
