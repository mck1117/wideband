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

TEST(Sampler, TestDcBehavior)
{
    Sampler dut;

    AnalogChannelResult data;
    data.NernstVoltage = 0.45f;
    data.PumpCurrentVoltage = 1.75f;
    constexpr float virtualGroundVoltage = 1.65f;

    for (size_t i = 0; i < 10; i++)
    {
        dut.ApplySample(data, virtualGroundVoltage);
    }

    // not exactly 0 because of filtering
    EXPECT_NEAR(0, dut.GetNernstAc(), 1e-3);
    EXPECT_FLOAT_EQ(0.45f, dut.GetNernstDc());
    EXPECT_NEAR(-0.0295, dut.GetPumpNominalCurrent(), 1e-3);
}
