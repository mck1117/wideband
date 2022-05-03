#pragma once

#include <cstdint>

/* for PWMConfig */
#include "hal.h"

struct PWMDriver;

class Pwm
{
public:
    Pwm(PWMDriver& driver, uint8_t channel, uint32_t counterFrequency, uint32_t counterPeriod);
    Pwm(PWMDriver& driver);

    void Start();
    void Start(PWMConfig *config);
    void SetDuty(float duty);
    void SetDuty(int channel, float duty);
    float GetLastDuty() const;

private:
    PWMDriver* const m_driver;
    const uint8_t m_channel;
    /* const */ uint32_t m_counterFrequency;
    /* const */ uint16_t m_counterPeriod;
    float m_dutyFloat;
};
