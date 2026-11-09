#include "hal.h"
#include "ch.h"
#include "pwm.h"
#include <cstdint>
#include "sampling.h"
#include <rusefi/math.h>

#include "wideband_config.h"
#include "analog.h"

#ifdef ANALOG_PWM_DEVICE

#define ANALOG_PWM_PERIOD 4096

static Pwm analogPwm(ANALOG_PWM_DEVICE);
static const PWMConfig analogPwmConfig = {
    .frequency = STM32_SYSCLK,
    .period = ANALOG_PWM_PERIOD,
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

    // Map lambda 0.68 -> 1.36 linearly to 0 -> full scale counts.
    // One float multiply + conversion, then clamp in integer: soft float compares and
    // conversions are expensive on M0, so avoid going through float duty + SetDuty.
    constexpr float countsPerLambda = ANALOG_PWM_PERIOD / 0.68f;
    int32_t highTime = (int32_t)((lambda - 0.68f) * countsPerLambda);

    if (highTime < 0) {
        highTime = 0;
    } else if (highTime > ANALOG_PWM_PERIOD) {
        highTime = ANALOG_PWM_PERIOD;
    }

    analogPwm.SetHighTime(ANALOG_PWM_CHANNEL, highTime);
}

#endif // defined ANALOG_PWM_DEVICE
