#include "analog.h"

void initAnalogOut(void)
{
    palSetPadMode(GPIOA, 15U, PAL_MODE_OUTPUT_PUSHPULL); // PWM2
    palSetPadMode(GPIOA, 15U, PAL_MODE_ALTERNATE(2));    // PWM2
    analogPwm.Start(analogPwmConfig);
}

void setAnalogOut(uint8_t ch) {
    auto lambda = GetLambda(ch);
    lambda = clampF(0.68f, lambda, 1.36f);
    float duty = (lambda - 0.68f) / 0.68f;
    analogPwm.SetDuty(0U, duty);
}