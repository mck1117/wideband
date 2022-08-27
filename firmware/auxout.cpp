#include "pwm.h"
#include "lambda_conversion.h"

#include "wideband_config.h"

#include "hal.h"

#include <rusefi/math.h>

#ifdef AUXOUT_DAC_PWM_DEVICE

// Rev2 low pass filter cut frequency is about 21Hz (sic!)
// 48Mhz / (2 ^ 12) ~= 12 KHz
PWMConfig auxPwmConfig = {
    48'000'000,
    1 << 12,
    nullptr,
    {
        {PWM_OUTPUT_ACTIVE_HIGH | PWM_COMPLEMENTARY_OUTPUT_ACTIVE_HIGH, nullptr},
        {PWM_OUTPUT_ACTIVE_HIGH | PWM_COMPLEMENTARY_OUTPUT_ACTIVE_HIGH, nullptr},
        {PWM_OUTPUT_ACTIVE_HIGH | PWM_COMPLEMENTARY_OUTPUT_ACTIVE_HIGH, nullptr},
        {PWM_OUTPUT_ACTIVE_HIGH | PWM_COMPLEMENTARY_OUTPUT_ACTIVE_HIGH, nullptr}
    },
    0,
    0,
#if STM32_PWM_USE_ADVANCED
    0
#endif
};

static Pwm auxDac(AUXOUT_DAC_PWM_DEVICE);

static const uint8_t auxOutPwmCh[] = {
    AUXOUT_DAC_PWM_CHANNEL_0,
#if (AFR_CHANNELS > 1)
    AUXOUT_DAC_PWM_CHANNEL_1,
#endif
};

void SetAuxDac(int channel, float voltage)
{
    voltage = voltage / AUXOUT_GAIN;
    auto duty = voltage / VCC_VOLTS;
    duty = 1.0 - duty;
    duty = clampF(0, duty, 1);

    auxDac.SetDuty(auxOutPwmCh[channel], duty);
}

/* TODO: merge with some other communication thread? */
static THD_WORKING_AREA(waAuxOutThread, 256);
void AuxOutThread(void*)
{
    while(1)
    {
        for (int ch = 0; ch < AFR_CHANNELS; ch++)
        {
            SetAuxDac(ch, GetLambda(ch));
        }

        chThdSleepMilliseconds(10);
    }
}

void InitAuxDac()
{
    auxDac.Start(auxPwmConfig);

    SetAuxDac(0, 0.0);
    SetAuxDac(1, 0.0);

    chThdCreateStatic(waAuxOutThread, sizeof(waAuxOutThread), NORMALPRIO, AuxOutThread, nullptr);
}

#endif
