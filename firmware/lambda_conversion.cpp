#include "lambda_conversion.h"
#include "sampling.h"

static float GetPhi(float pumpCurrent)
{
    // Maximum lambda ~2
    if (pumpCurrent > 1.11f)
    {
        return 0.5f;
    }

    // Minimum lambda ~0.5
    if (pumpCurrent < -3.5f)
    {
        return 1 / 0.5f;
    }

    // This estimation is accurate within 0.5% from 0.8 to 1.0, and 0.01% from 1 to 1.2 lambda when compared to the lookup table in the Bosch datasheet
    // This error is less than half of the claimed accuracy of the sensor itself
    float gain = pumpCurrent < 0 ? -0.28299f : -0.44817f;

    return gain * pumpCurrent + 0.99559f;
}

float GetLambda(int ch)
{
    float pumpCurrent = GetPumpNominalCurrent(ch);

    // Lambda is reciprocal of phi
    return 1 / GetPhi(pumpCurrent);
}
