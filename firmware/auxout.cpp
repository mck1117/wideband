#include "pwm.h"
#include "dac.h"
#include "lambda_conversion.h"
#include "port.h"
#include "io_pins.h"

#include "wideband_config.h"

#include "max3185x.h"

#include "hal.h"

#include <rusefi/rusefi_math.h>
#include <rusefi/interpolation.h>

#ifdef AUXOUT_DAC_PWM_DEVICE

#ifndef AUXOUT_DAC_PWM_OUTPUT_MODE
#define AUXOUT_DAC_PWM_OUTPUT_MODE PWM_OUTPUT_ACTIVE_HIGH
#endif
#ifndef AUXOUT_DAC_PWM_NC_OUTPUT_MODE
#define AUXOUT_DAC_PWM_NC_OUTPUT_MODE PWM_OUTPUT_ACTIVE_LOW
#endif

// Rev2 low pass filter cut frequency is about 21Hz (sic!)
// 48Mhz / (2 ^ 12) ~= 12 KHz
// 64mhz / (2 ^ 12) ~= 16 KHz
static PWMConfig auxPwmConfig = {
    .frequency = STM32_SYSCLK,
    .period = 1 << 12,
    .callback = nullptr,
    .channels = {
        [0] = {0, nullptr},
        [1] = {0, nullptr},
        [2] = {0, nullptr},
        [3] = {0, nullptr}
    },
    .cr2 = 0,
#if STM32_PWM_USE_ADVANCED
    .bdtr = 0,
#endif
    .dier = 0
};

static void auxDacFillPwmConfig(void)
{
    auxPwmConfig.channels[AUXOUT_DAC_PWM_CHANNEL_0].mode = AUXOUT_DAC_PWM_OUTPUT_MODE;
    auxPwmConfig.channels[AUXOUT_DAC_PWM_CHANNEL_1].mode = AUXOUT_DAC_PWM_OUTPUT_MODE;
#ifdef AUXOUT_DAC_PWM_CHANNEL_0_NC
    auxPwmConfig.channels[AUXOUT_DAC_PWM_CHANNEL_0_NC].mode = AUXOUT_DAC_PWM_NC_OUTPUT_MODE;
#endif
#ifdef AUXOUT_DAC_PWM_CHANNEL_1_NC
    auxPwmConfig.channels[AUXOUT_DAC_PWM_CHANNEL_1_NC].mode = AUXOUT_DAC_PWM_NC_OUTPUT_MODE;
#endif
}

static Pwm auxDac(AUXOUT_DAC_PWM_DEVICE);

static const uint8_t auxOutPwmCh[AFR_CHANNELS] = {
    AUXOUT_DAC_PWM_CHANNEL_0,
#if (AFR_CHANNELS > 1)
    AUXOUT_DAC_PWM_CHANNEL_1,
#endif
};

static const int8_t auxOutPwmChN[AFR_CHANNELS] = {
#ifdef AUXOUT_DAC_PWM_CHANNEL_0_NC
    AUXOUT_DAC_PWM_CHANNEL_0_NC,
#else
    -1,
#endif
#if (AFR_CHANNELS > 1)
#ifdef AUXOUT_DAC_PWM_CHANNEL_1_NC
    AUXOUT_DAC_PWM_CHANNEL_1_NC,
#else
    -1,
#endif
#endif
};

void SetAuxDac(int channel, float voltage)
{
    voltage = voltage / AUXOUT_GAIN;
    auto duty = voltage / VCC_VOLTS;
    duty = clampF(0, duty, 1);

    auxDac.SetDuty(auxOutPwmCh[channel], duty);
    // Ripple cancelation channel
    if (auxOutPwmChN[channel >= 0]) {
        auxDac.SetDuty(auxOutPwmChN[channel], duty);
    }
}

#endif

#ifdef AUXOUT_DAC_DEVICE

static DACConfig auxDacConfig = {
  .init         = 2047U,
  .datamode     = DAC_DHRM_12BIT_RIGHT,
  .cr           = 0
};

static Dac auxDac(AUXOUT_DAC_DEVICE);

static const uint8_t auxOutDacCh[] = {
    AUXOUT_DAC_CHANNEL_0,
#if (AFR_CHANNELS > 1)
    AUXOUT_DAC_CHANNEL_1,
#endif
};

void SetAuxDac(int channel, float voltage)
{
    voltage = voltage / AUXOUT_GAIN;

    auxDac.SetVoltage(auxOutDacCh[channel], voltage);
}

#endif /* AUXOUT_DAC_DEVICE */

#if (defined(AUXOUT_DAC_PWM_DEVICE) || defined(AUXOUT_DAC_DEVICE))

static float AuxGetInputSignal(AuxOutputMode sel)
{
    switch (sel)
    {
        case AuxOutputMode::Afr0:
            return 14.7f * GetLambda(0);
        case AuxOutputMode::Afr1:
            return 14.7f * GetLambda(1);
        case AuxOutputMode::Lambda0:
            return GetLambda(0);
        case AuxOutputMode::Lambda1:
            return GetLambda(1);
#if HAL_USE_SPI
        case AuxOutputMode::Egt0:
            return getEgtDrivers()[0].temperature;
        case AuxOutputMode::Egt1:
            return getEgtDrivers()[1].temperature;
#endif
        default:
            return 0;
    }

    return 0;
}

/* TODO: merge with some other communication thread? */
static THD_WORKING_AREA(waAuxOutThread, 256);
void AuxOutThread(void*)
{
    const auto cfg = GetConfiguration();

    chRegSetThreadName("Aux out");

    while(1)
    {
        for (int ch = 0; ch < AFR_CHANNELS; ch++)
        {
            float input = AuxGetInputSignal(cfg->auxOutputSource[ch]);
            float voltage = interpolate2d(input, cfg->auxOutBins[ch], cfg->auxOutValues[ch]);

            SetAuxDac(ch, voltage);
        }

        chThdSleepMilliseconds(10);
    }
}

void InitAuxDac()
{
#if defined(AUXOUT_DAC_PWM_DEVICE)
    auxDacFillPwmConfig();
    auxDac.Start(auxPwmConfig);

    SetAuxDac(0, 0.0);
    SetAuxDac(1, 0.0);
#endif
#if defined(AUXOUT_DAC_DEVICE)
    auxDac.Start(auxDacConfig);

    SetAuxDac(0, 0.0);
#endif

    chThdCreateStatic(waAuxOutThread, sizeof(waAuxOutThread), NORMALPRIO, AuxOutThread, nullptr);
}

#else /* (AUXOUT_DAC_PWM_DEVICE || AUXOUT_DAC_DEVICE) */

void InitAuxDac()
{
}

#endif
