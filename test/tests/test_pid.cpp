#include <gtest/gtest.h>

#include "pid.h"

TEST(Pid, Proportional)
{
    const PidConfig config = {.kP = 2, .kI = 0, .kD = 0, .clamp = 10};
    Pid dut(config, 2);

    EXPECT_NEAR(2 * 0.5f, dut.GetOutput(1.0f, 0.5f), 1e-6);
    EXPECT_NEAR(2 * -0.25f, dut.GetOutput(1.0f, 1.25f), 1e-6);
}

TEST(Pid, IntegralAndClamp)
{
    const PidConfig config = {.kP = 0, .kI = 10, .kD = 0, .clamp = 0.1f};
    Pid dut(config, 2);

    // integrator += error * 2ms * kI = 0.5 * 0.002 * 10 = 0.01 per call
    for (int i = 1; i <= 5; i++)
    {
        EXPECT_NEAR(0.01f * i, dut.GetOutput(0.5f, 0), 1e-5) << i;
    }

    // Clamps at +clamp...
    for (int i = 0; i < 100; i++)
    {
        dut.GetOutput(0.5f, 0);
    }
    EXPECT_NEAR(0.1f, dut.GetOutput(0.5f, 0), 1e-6);

    // ...and -clamp
    for (int i = 0; i < 100; i++)
    {
        dut.GetOutput(0, 0.5f);
    }
    EXPECT_NEAR(-0.1f, dut.GetOutput(0, 0.5f), 1e-6);
}

TEST(Pid, Derivative)
{
    const PidConfig config = {.kP = 0, .kI = 0, .kD = 0.5f, .clamp = 10};
    Pid dut(config, 50);

    // Error steps 0 -> 1 over one 50ms period: dE/dt = 20/s, times kD
    EXPECT_NEAR(0.5f * 20, dut.GetOutput(1, 0), 1e-4);
    // Error unchanged: no derivative
    EXPECT_NEAR(0, dut.GetOutput(1, 0), 1e-6);
    // Error steps 1 -> -0.5: dE/dt = -30/s
    EXPECT_NEAR(0.5f * -30, dut.GetOutput(0, 0.5f), 1e-4);
}

TEST(Pid, AllTerms)
{
    const PidConfig config = {.kP = 0.3f, .kI = 0.3f, .kD = 0.01f, .clamp = 1};
    Pid dut(config, 50);

    // P: 0.3 * 0.2, I: 0.2 * 0.05 * 0.3, D: 0.01 * 0.2 / 0.05
    EXPECT_NEAR(0.06f + 0.003f + 0.04f, dut.GetOutput(0.7f, 0.5f), 1e-5);
}

TEST(Pid, NoDerivativeGain)
{
    // With kD = 0, output is exactly P + I no matter how fast the error changes
    const PidConfig config = {.kP = 50, .kI = 10000, .kD = 0, .clamp = 10};
    Pid dut(config, 2);

    float integrator = 0;
    const float errors[] = {0.1f, -0.3f, 0.0f, 0.25f, -0.05f};
    for (float error : errors)
    {
        integrator += error * 0.002f * 10000;
        EXPECT_NEAR(50 * error + integrator, dut.GetOutput(error, 0), 1e-3) << error;
    }
}
