#include "pump_dac.h"
#include "pwm.h"
#include "dac.h"
#include "heater_control.h"

#include "wideband_config.h"

#include "hal.h"

#ifdef PUMP_DAC_PWM_DEVICE

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

static const uint8_t pumpDacPwmCh[] = {
    PUMP_DAC_PWM_CHANNEL_0,
#if (AFR_CHANNELS > 1)
    PUMP_DAC_PWM_CHANNEL_1,
#endif
};

static Pwm pumpDac(PUMP_DAC_PWM_DEVICE);

static void SetPumpVoltage(int ch, float volts)
{
    pumpDac.SetDuty(pumpDacPwmCh[ch], volts / VCC_VOLTS);
}

#endif

#ifdef PUMP_DAC_DAC_DEVICE_0

static DACConfig pumpDacConfig = {
  .init         = 2047U,    /* half of VCC */
  .datamode     = DAC_DHRM_12BIT_RIGHT,
  .cr           = 0
};

static Dac pumpDacs[] {
    Dac(PUMP_DAC_DAC_DEVICE_0),
    Dac(PUMP_DAC_DAC_DEVICE_1)
};

static void SetPumpVoltage(int ch, float volts)
{
    pumpDacs[ch].SetVoltage(0, volts);
}

#endif

struct pump_dac_state {
    int32_t curIpump;
};

static struct pump_dac_state state[AFR_CHANNELS];

void InitPumpDac()
{
#ifdef PUMP_DAC_PWM_DEVICE
    pumpDac.Start(pumpDacConfig);
#endif
#ifdef PUMP_DAC_DAC_DEVICE_0
    pumpDacs[0].Start(pumpDacConfig);
    pumpDacs[1].Start(pumpDacConfig);
#endif

    for (int ch = 0; ch < AFR_CHANNELS; ch++)
    {
        // Set zero current to start - sensor can be damaged if current flowing
        // while warming up
        SetPumpCurrentTarget(ch, 0);
    }
}

void SetPumpCurrentTarget(int ch, int32_t microampere)
{
#ifndef START_PUMP_TEMP_OFFSET
    // Don't allow pump current when the sensor isn't hot
    if (!GetHeaterController(ch).IsRunningClosedLoop())
    {
        microampere = 0;
    }
#endif

    state[ch].curIpump = microampere;

    // 47 ohm resistor
    // 0.147 gain
    // effective resistance of 317 ohms
    float volts = -0.000321162f * microampere;

    // offset by half vcc
    volts += HALF_VCC;

    SetPumpVoltage(ch, volts);
}

float GetPumpOutputDuty(int ch)
{
#ifdef PUMP_DAC_PWM_DEVICE
    return pumpDac.GetLastDuty(pumpDacPwmCh[ch]);
#endif
#ifdef PUMP_DAC_DAC_DEVICE_0
    return pumpDacs[ch].GetLastVoltage(0) / 3.3;
#endif
}

float GetPumpCurrent(int ch)
{
    return (float)state[ch].curIpump / 1000.0;
}
