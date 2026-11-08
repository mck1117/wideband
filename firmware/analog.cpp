#include "hal.h"
#include "ch.h"
#include "pwm.h"
#include <cstdint>
#include "sampling.h"
#include <rusefi/math.h>

#include "wideband_config.h"
#include "analog.h"

#ifdef ANALOG_PWM_DEVICE

static constexpr pwmcnt_t analogPwmPeriod = 4096;
static const PWMConfig analogPwmConfig = {
    .frequency = STM32_SYSCLK,
    .period = analogPwmPeriod,
    .callback = nullptr,
    .channels = {
        {PWM_OUTPUT_ACTIVE_HIGH | PWM_COMPLEMENTARY_OUTPUT_ACTIVE_LOW, nullptr},
        {PWM_OUTPUT_ACTIVE_HIGH | PWM_COMPLEMENTARY_OUTPUT_ACTIVE_LOW, nullptr},
        {PWM_OUTPUT_ACTIVE_HIGH | PWM_COMPLEMENTARY_OUTPUT_ACTIVE_LOW, nullptr},
        {PWM_OUTPUT_ACTIVE_HIGH | PWM_COMPLEMENTARY_OUTPUT_ACTIVE_LOW, nullptr},
    },
    .cr2 = 0,
#if STM32_PWM_USE_ADVANCED
    .bdtr = 0,
#endif
    .dier = 0,
};

void initAnalogOut(void)
{
    pwmStart(&ANALOG_PWM_DEVICE, &analogPwmConfig);
}

void setAnalogOut(uint8_t ch, float lambda) {
    if (ch != 0) {
        // only support single analog output
        return;
    }
    lambda = clampF(0.68f, lambda, 1.36f);
    constexpr float ticksPerLambda = analogPwmPeriod / 0.68f;
    pwmcnt_t width = (lambda - 0.68f) * ticksPerLambda;
    pwmEnableChannel(&ANALOG_PWM_DEVICE, ANALOG_PWM_CHANNEL, width);
}

#endif // defined ANALOG_PWM_DEVICE
