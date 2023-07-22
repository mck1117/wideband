#pragma once

#include <cstdint>

/* for PWMConfig */
#include "hal.h"

struct PWMDriver;

class Pwm
{
public:
    Pwm(PWMDriver& driver);

    void Start(const PWMConfig& config);
    void SetDuty(int channel, float duty);
    float GetLastDuty(int channel);

private:
    PWMDriver* const m_driver;
    //const uint8_t m_channel;
    /* const */ uint32_t m_counterFrequency;
    /* const */ uint16_t m_counterPeriod;
    float m_dutyFloat[4];
};
