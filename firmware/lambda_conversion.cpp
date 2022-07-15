#include "lambda_conversion.h"
#include "sampling.h"
#include "port.h"

#include <rusefi/interpolation.h>

static float LSU49GetLambda(float pumpCurrent)
{
    // Maximum lambda ~2
    if (pumpCurrent > 1.11f)
    {
        return 2.0f;
    }

    // Minimum lambda ~0.5
    if (pumpCurrent < -3.5f)
    {
        return 0.5f;
    }

    // This estimation is accurate within 0.5% from 0.8 to 1.0, and 0.01% from 1 to 1.2 lambda when compared to the lookup table in the Bosch datasheet
    // This error is less than half of the claimed accuracy of the sensor itself
    float gain = pumpCurrent < 0 ? -0.28299f : -0.44817f;

    // Lambda is reciprocal of phi
    return 1 / (gain * pumpCurrent + 0.99559f);
}

static float LSU42GetLambda(float pumpCurrent)
{
    static const float lsu42LambdaBins[]   = {-1.85, -1.08, -0.76, -0.47,  0.00,  0.34,  0.68,  0.95,  1.40};
    static const float lsu42LambdaValues[] = {0.700, 0.800, 0.850, 0.900, 1.009, 1.180, 1.430, 1.700, 2.420};

    return interpolate2d(pumpCurrent, lsu42LambdaBins, lsu42LambdaValues);
}

float GetLambda(int ch)
{
    float pumpCurrent = GetPumpNominalCurrent(ch);

    if (GetSensorType() == SensorType::LSU49)
    {
        return LSU49GetLambda(pumpCurrent);
    } else if (GetSensorType() == SensorType::LSU42)
    {
        return LSU42GetLambda(pumpCurrent);
    }

    return 1.0;
}
