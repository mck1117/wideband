#pragma once

#include "port.h"

// Sensor type returned by the GetSensorType() stub. Defaults to LSU49.
extern SensorType g_testSensorType;

// Sets the stubbed sensor type for the lifetime of this object
class ScopedSensorType
{
public:
    explicit ScopedSensorType(SensorType type)
        : m_previous(g_testSensorType)
    {
        g_testSensorType = type;
    }

    ~ScopedSensorType()
    {
        g_testSensorType = m_previous;
    }

private:
    const SensorType m_previous;
};

// Value returned by the GetESRSupplyR() stub, matches hardware
constexpr int TestEsrSupplyR = 22000;
