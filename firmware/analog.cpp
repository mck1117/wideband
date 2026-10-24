#include "hal.h"
#include "ch.h"
#include "pwm.h"
#include <cstdint>
#include "sampling.h"
#include <rusefi/math.h>

#include "wideband_config.h"
#include "analog.h"

#ifdef ANALOG_PWM_DEVICE

static Pwm analogPwm(ANALOG_PWM_DEVICE);
static const PWMConfig analogPwmConfig = {
    .frequency = STM32_SYSCLK,
    .period = 4096,
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
    analogPwm.Start(analogPwmConfig);
}

void setAnalogOut(uint8_t ch, float lambda) {
    if (ch != 0) {
        // only support single analog output
        return;
    }

    lambda = clampF(0.68f, lambda, 1.36f);
    float duty = (lambda - 0.68f) / 0.68f;
    analogPwm.SetDuty(ANALOG_PWM_CHANNEL, duty);
}

#endif // defined ANALOG_PWM_DEVICE
