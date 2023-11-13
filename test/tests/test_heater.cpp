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

TEST(HeaterStateMachine, PreheatToWarmupTimeout)
{
    MockHeater dut;
    dut.Configure(780, 300);

    for (size_t i = 0; i < HeaterControllerBase::preheatTimeCounter - 1; i++)
    {
        EXPECT_EQ(HeaterState::Preheat, dut.GetNextState(HeaterState::Preheat, HeaterAllow::Allowed, 12, 500));
    }

    // Timer expired, transition to warmup ramp
    EXPECT_EQ(HeaterState::WarmupRamp, dut.GetNextState(HeaterState::Preheat, HeaterAllow::Allowed, 12, 500));
}

TEST(HeaterStateMachine, PreheatToWarmupAlreadyWarm)
{
    MockHeater dut;
    dut.Configure(780, 300);

    // Preheat for a little while
    for (size_t i = 0; i < 10; i++)
    {
        EXPECT_EQ(HeaterState::Preheat, dut.GetNextState(HeaterState::Preheat, HeaterAllow::Allowed, 12, 500));
    }

    // Sensor magically warms up, skip to warmup ramp!
    EXPECT_EQ(HeaterState::WarmupRamp, dut.GetNextState(HeaterState::Preheat, HeaterAllow::Allowed, 12, 780));
}

TEST(HeaterStateMachine, WarmupToClosedLoop)
{
    MockHeater dut;
    dut.Configure(780, 300);

    // Warm up for a little while
    for (size_t i = 0; i < 10; i++)
    {
        EXPECT_EQ(HeaterState::WarmupRamp, dut.GetNextState(HeaterState::WarmupRamp, HeaterAllow::Allowed, 12, 500));
    }

    // Sensor warms up, time for closed loop!
    EXPECT_EQ(HeaterState::ClosedLoop, dut.GetNextState(HeaterState::WarmupRamp, HeaterAllow::Allowed, 12, 780));
}

TEST(HeaterStateMachine, WarmupTimeout)
{
    MockHeater dut;
    dut.Configure(780, 300);

    size_t timeoutPeriod = dut.GetTimeCounter();

    // Warm up for a little while
    for (size_t i = 0; i < timeoutPeriod - 1; i++)
    {
        EXPECT_EQ(HeaterState::WarmupRamp, dut.GetNextState(HeaterState::WarmupRamp, HeaterAllow::Allowed, 12, 500)) << "i = " << i;
    }

    // Warmup times out, sensor transitions to stopped
    EXPECT_EQ(HeaterState::Stopped, dut.GetNextState(HeaterState::WarmupRamp, HeaterAllow::Allowed, 12, 500));
}

TEST(HeaterStateMachine, ClosedLoop)
{
    MockHeater dut;
    dut.Configure(780, 300);

    // Temperature is reasonable, stay in closed loop
    EXPECT_EQ(HeaterState::ClosedLoop, dut.GetNextState(HeaterState::ClosedLoop, HeaterAllow::Allowed, 12, 780));

    // Temperature is too hot, overheat
    EXPECT_EQ(HeaterState::Stopped, dut.GetNextState(HeaterState::ClosedLoop, HeaterAllow::Allowed, 12, 1000));

    // Temperature is too cold, underheat
    EXPECT_EQ(HeaterState::Stopped, dut.GetNextState(HeaterState::ClosedLoop, HeaterAllow::Allowed, 12, 600));
}

TEST(HeaterStateMachine, TerminalStates)
{
    MockHeater dut;
    dut.Configure(780, 300);

    EXPECT_EQ(HeaterState::Stopped, dut.GetNextState(HeaterState::Stopped, HeaterAllow::Allowed, 12, 780));
}
