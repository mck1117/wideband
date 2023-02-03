#include "lambda_conversion.h"
#include "sampling.h"
#include "port.h"

static float GetPhiLsu49(float pumpCurrent)
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

static float GetPhiLsu42(float pumpCurrent)
{
    // Maximum lambda ~2
    if (pumpCurrent > 1.19f)
    {
        return 0.5f;
    }

    // Minimum lambda ~0.7
    if (pumpCurrent < -1.85f)
    {
        return 1 / 0.7f;
    }

    // This estimation is accurate within 0.5% from 0.8 to 1.0, and 0.01% from 1 to 1.2 lambda when compared to the lookup table in the Bosch datasheet
    // This error is less than half of the claimed accuracy of the sensor itself
    float gain = pumpCurrent < 0 ? -0.23505f : -0.41441f;

    return gain * pumpCurrent + 0.99153f;
}

static float GetPhiLsuAdv(float /*pumpCurrent*/)
{
    // TODO
    return 1;
}

static float GetPhi(float pumpCurrent) {
    switch (GetSensorType()) {
        case SensorType::LSU49:
            return GetPhiLsu49(pumpCurrent);
        case SensorType::LSU42:
            return GetPhiLsu42(pumpCurrent);
        case SensorType::LSUADV:
            return GetPhiLsuAdv(pumpCurrent);
    }

    return 1;
}

float GetLambda(int ch)
{
    float pumpCurrent = GetPumpNominalCurrent(ch);

    // Lambda is reciprocal of phi
    return 1 / GetPhi(pumpCurrent);
}
