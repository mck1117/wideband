#include "pump_dac.h"
#include "pwm.h"
#include "heater_control.h"

#include "wideband_config.h"

#include "hal.h"

// 48MHz / 1024 = 46.8khz PWM
static Pwm pumpDac(PUMP_DAC_PWM_DEVICE, PUMP_DAC_PWM_CHANNEL, 48'000'000, 1024);

void InitPumpDac()
{
    pumpDac.Start();

    // Set zero current to start - sensor can be damaged if current flowing
    // while warming up
    SetPumpCurrentTarget(0);
}

void SetPumpCurrentTarget(int32_t microampere)
{
    // Don't allow pump current when the sensor isn't hot
    if (!IsRunningClosedLoop())
    {
        microampere = 0;
    }

    // 47 ohm resistor
    // 0.147 gain
    // effective resistance of 317 ohms
    float volts = -0.000321162f * microampere;

    // offset by half vcc
    volts += HALF_VCC;

    pumpDac.SetDuty(volts / VCC_VOLTS);
}

float GetPumpOutputDuty()
{
    return pumpDac.GetLastDuty();
}
