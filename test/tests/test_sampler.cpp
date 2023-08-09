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

TEST(Sampler, TestDc)
{
    Sampler dut;

    AnalogChannelResult data;
    data.NernstVoltage = 0.45f;
    data.PumpCurrentVoltage = 1.75f;
    constexpr float virtualGroundVoltage = 1.65f;

    for (size_t i = 0; i < 5000; i++)
    {
        dut.ApplySample(data, virtualGroundVoltage);
    }

    // not exactly 0 because of filtering
    EXPECT_NEAR(0, dut.GetNernstAc(), 1e-3);
    EXPECT_FLOAT_EQ(0.45f, dut.GetNernstDc());
    EXPECT_NEAR(-0.1616, dut.GetPumpNominalCurrent(), 1e-3);
}

TEST(Sampler, TestAc)
{
    Sampler dut;

    AnalogChannelResult dataLow;
    dataLow.NernstVoltage = 0.45f - 0.1f;
    dataLow.PumpCurrentVoltage = 1.75f;

    AnalogChannelResult dataHigh;
    dataHigh.NernstVoltage = 0.45f + 0.1f;
    dataHigh.PumpCurrentVoltage = 1.75f;

    constexpr float virtualGroundVoltage = 1.65f;

    for (size_t i = 0; i < 5000; i++)
    {
        dut.ApplySample(dataLow,  virtualGroundVoltage);
        dut.ApplySample(dataHigh, virtualGroundVoltage);
    }

    EXPECT_NEAR(0.2, dut.GetNernstAc(), 1e-3);
    EXPECT_FLOAT_EQ(0.45f, dut.GetNernstDc());
    EXPECT_NEAR(-0.1616, dut.GetPumpNominalCurrent(), 1e-3);
}
