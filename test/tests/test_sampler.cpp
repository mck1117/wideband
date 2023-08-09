#include <gtest/gtest.h>

#include "sampling.h"
#include "port.h"

SensorType GetSensorType()
{
    return SensorType::LSU49;
}

int GetESRSupplyR()
{
    // Nernst AC injection resistor value
    return 22000;
}

TEST(Sampler, Basic)
{
    Sampler dut;
}
