#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "heater_control.h"

struct MockHeater : public HeaterControllerBase
{
    MockHeater() : HeaterControllerBase(0) { }

    MOCK_METHOD(void, SetDuty, (float), (const, override));
};

TEST(HeaterStateOutput, Preheat)
{
    MockHeater dut;

    // Shouldn't depend upon sensor ESR
    EXPECT_EQ(1.5f, dut.GetVoltageForState(HeaterState::Preheat, 0));
    EXPECT_EQ(1.5f, dut.GetVoltageForState(HeaterState::Preheat, 300));
    EXPECT_EQ(1.5f, dut.GetVoltageForState(HeaterState::Preheat, 1000));
}

TEST(HeaterStateOutput, WarmupRamp)
{
    MockHeater dut;

    EXPECT_EQ(1.5f, dut.GetVoltageForState(HeaterState::Preheat, 0));
    EXPECT_EQ(1.5f, dut.GetVoltageForState(HeaterState::Preheat, 300));
    EXPECT_EQ(1.5f, dut.GetVoltageForState(HeaterState::Preheat, 1000));
}

TEST(HeaterStateOutput, ClosedLoop)
{
    MockHeater dut;
    dut.Configure(780, 300);

    // At target -> zero output but with 7.5v offset
    EXPECT_EQ(dut.GetVoltageForState(HeaterState::ClosedLoop, 300), 7.5f);

    // Below target -> more voltage
    EXPECT_GT(dut.GetVoltageForState(HeaterState::ClosedLoop, 400), 7.5f);

    // Above target -> less voltage
    EXPECT_LT(dut.GetVoltageForState(HeaterState::ClosedLoop, 200), 7.5f);
}

TEST(HeaterStateOutput, Cases)
{
    MockHeater dut;

    EXPECT_EQ(0, dut.GetVoltageForState(HeaterState::Stopped, 0));
    EXPECT_EQ(0, dut.GetVoltageForState(HeaterState::NoHeaterSupply, 0));
}

TEST(HeaterStateMachine, x)
{
    MockHeater dut;
}
