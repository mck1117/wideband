#pragma once
#ifndef ANALOG_H
#define ANALOG_H

#include "hal.h"
#include "ch.h"
#include "pwm.h"
#include <cstdint>
#include "lambda_conversion.h"
#include <rusefi/math.h>

static Pwm analogPwm(PWMD2);
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

void initAnalogOut(void);
void setAnalogOut(uint8_t ch);
#endif