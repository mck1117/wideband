#pragma once

#include <cstdint>

struct PWMDriver;

class Pwm
{
public:
    Pwm(PWMDriver& driver, uint8_t channel, uint32_t counterFrequency, uint32_t counterPeriod);

    void Start();
    void SetDuty(float duty);
    float GetLastDuty() const;

private:
    PWMDriver* const m_driver;
    const uint8_t m_channel;
    const uint32_t m_counterFrequency;
    const uint16_t m_counterPeriod;
    float m_dutyFloat;
};
