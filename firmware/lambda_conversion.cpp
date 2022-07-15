#include "lambda_conversion.h"
#include "sampling.h"
#include "interpolation.h"
#include "port.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

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

static float LSU42GetLambda(float pumpCurrent)
{
    const struct inter_point lsu42[] =
    {
        {-1.85, 0.700},
        {-1.08, 0.800},
        {-0.76, 0.850},
        {-0.47, 0.900},
        { 0.00, 1.009},
        { 0.34, 1.180},
        { 0.68, 1.430},
        { 0.95, 1.700},
        { 1.40, 2.420},
    };

    return interpolate_1d_float(lsu42, ARRAY_SIZE(lsu42), pumpCurrent);
}

float GetLambda()
{
    float pumpCurrent = GetPumpNominalCurrent();

    if (GetSensorType() == SENSOR_TYPE_LSU49)
    {
        // Lambda is reciprocal of phi
        return 1 / GetPhi(pumpCurrent);
    } else if (GetSensorType() == SENSOR_TYPE_LSU42)
    {
        return LSU42GetLambda(pumpCurrent);
    }

    return 1.0;
}
