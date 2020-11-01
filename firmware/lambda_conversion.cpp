#pragma once

#include "sampling.h"

static float GetPhi(float pumpCurrent)
{
    // This estimation is accurate within 0.5% from 0.8 to 1.0, and 0.01% from 1 to 1.2 lambda when compared to the lookup table in the Bosch datasheet
    // This error is less than half of the claimed accuracy of the sensor itself
    float gain = pumpCurrent > 0 ? -0.28299f : -0.44817f;

    return gain * pumpCurrent + 0.99559f;
}

float GetLambda()
{
    float pumpCurrent = GetPumpNominalCurrent();

    // Lambda is reciprocal of phi
    return 1 / GetPhi(pumpCurrent);
}
