#include "pump_dac.h"
#include "pwm.h"

#include "hal.h"

// 48MHz / 1024 = 46.8khz PWM
static Pwm pumpDac(PWMD3, 0, 48'000'000, 1024);

void InitPumpDac()
{
    pumpDac.Start();

    // Set zero current to start - sensor can be damaged if current flowing
    // while warming up
    SetPumpCurrentTarget(0);
}

void SetPumpCurrentTarget(int32_t microampere)
{
    // 47 ohm resistor
    // 0.147 gain
    // effective resistance of 317 ohms
    float volts = 0.000321162f * microampere;

    // offset by 
    volts += 1.65f;

    pumpDac.SetDuty(volts / 3.3f);
}
