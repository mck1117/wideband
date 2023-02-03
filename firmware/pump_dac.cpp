#include "pump_dac.h"
#include "pwm.h"
#include "heater_control.h"

#include "wideband_config.h"

#include "hal.h"

// 48MHz / 1024 = 46.8khz PWM
// 64MHz / 1024 = 62.5khz PWM
static const PWMConfig pumpDacConfig = {
    STM32_SYSCLK,
    1024,
    nullptr,
    {
        {PWM_OUTPUT_ACTIVE_HIGH, nullptr},
        {PWM_OUTPUT_ACTIVE_HIGH, nullptr},
        {PWM_OUTPUT_ACTIVE_HIGH, nullptr},
        {PWM_OUTPUT_ACTIVE_HIGH, nullptr}
    },
    0,
    0,
#if STM32_PWM_USE_ADVANCED
    0
#endif
};

static Pwm pumpDac(PUMP_DAC_PWM_DEVICE);

struct pump_dac_state {
    int32_t curIpump;
};

static const uint8_t pumpDacPwmCh[] = {
    PUMP_DAC_PWM_CHANNEL_0,
#if (AFR_CHANNELS > 1)
    PUMP_DAC_PWM_CHANNEL_1,
#endif
};

static struct pump_dac_state state[AFR_CHANNELS];

void InitPumpDac()
{
    pumpDac.Start(pumpDacConfig);

    for (int ch = 0; ch < AFR_CHANNELS; ch++)
    {
        // Set zero current to start - sensor can be damaged if current flowing
        // while warming up
        SetPumpCurrentTarget(ch, 0);
    }
}

void SetPumpCurrentTarget(int ch, int32_t microampere)
{
    // Don't allow pump current when the sensor isn't hot
    if (!IsRunningClosedLoop(ch))
    {
        microampere = 0;
    }

    state[ch].curIpump = microampere;

    // 47 ohm resistor
    // 0.147 gain
    // effective resistance of 317 ohms
    float volts = -0.000321162f * microampere;

    // offset by half vcc
    volts += HALF_VCC;

    pumpDac.SetDuty(pumpDacPwmCh[ch], volts / VCC_VOLTS);
}

float GetPumpOutputDuty(int ch)
{
    return pumpDac.GetLastDuty(pumpDacPwmCh[ch]);
}

float GetPumpCurrent(int ch)
{
    return (float)state[ch].curIpump / 1000.0;
}
