#include "analog.h"

void initAnalogOut(void)
{
    analogPwm.Start(analogPwmConfig);
}

void setAnalogOut(uint8_t ch) {
    auto lambda = GetLambda(ch);
    lambda = clampF(0.68f, lambda, 1.36f);
    float duty = (lambda - 0.68f) / 0.68f;
    analogPwm.SetDuty(0U, duty);
}