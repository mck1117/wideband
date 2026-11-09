#include <gtest/gtest.h>

#include <initializer_list>

#include "lambda_conversion.h"

#include "test_stubs.h"

namespace
{

struct Point
{
    float pumpCurrent;
    float lambda;
};

// Characterizes the current curve fits (see lambda_conversion.cpp for their accuracy vs. the
// Bosch datasheets), including the clamps at each end.
void ExpectPoints(SensorType type, std::initializer_list<Point> points)
{
    ScopedSensorType scope(type);

    for (const auto& p : points)
    {
        EXPECT_NEAR(p.lambda, ConvertLambda(p.pumpCurrent), 1e-5) << static_cast<int>(type) << " " << p.pumpCurrent;
    }
}

} // namespace

TEST(LambdaConversion, Lsu49)
{
    ExpectPoints(SensorType::LSU49, {
                                        {-4.0f, 0.500000f}, // clamped rich
                                        {-3.0f, 0.542135f},
                                        {-1.0f, 0.782118f},
                                        {0.0f, 1.004430f},
                                        {0.5f, 1.296168f},
                                        {1.0f, 1.826751f},
                                        {1.5f, 2.000000f}, // clamped lean
                                    });
}

TEST(LambdaConversion, Lsu42)
{
    ExpectPoints(SensorType::LSU42, {
                                        {-2.0f, 0.700000f}, // clamped rich
                                        {-1.5f, 0.743989f},
                                        {-0.5f, 0.901669f},
                                        {0.0f, 1.008542f},
                                        {0.5f, 1.274982f},
                                        {1.0f, 1.732742f},
                                        {1.5f, 2.000000f}, // clamped lean
                                    });
}

TEST(LambdaConversion, LsuAdv)
{
    ExpectPoints(SensorType::LSUADV, {
                                         {-1.5f, 0.650000f}, // clamped rich
                                         {-1.0f, 0.676727f},
                                         {-0.5f, 0.816677f},
                                         {0.0f, 1.014302f},
                                         {0.25f, 1.276620f},
                                         {0.5f, 1.683573f},
                                         {1.0f, 2.434000f}, // clamped lean
                                     });
}

TEST(LambdaConversion, MonotonicAndBounded)
{
    for (auto type : {SensorType::LSU49, SensorType::LSU42, SensorType::LSUADV})
    {
        ScopedSensorType scope(type);

        float previous = ConvertLambda(-5);

        for (float pumpCurrent = -5; pumpCurrent < 3; pumpCurrent += 0.001f)
        {
            float lambda = ConvertLambda(pumpCurrent);

            // Lambda increases with pump current. The fits have small steps at their clamp points
            // (eg. LSU4.9 reaches 2.0075 just before clamping to 2.0 at 1.11mA), so allow 0.5%.
            EXPECT_GE(lambda, previous * 0.995f) << static_cast<int>(type) << " " << pumpCurrent;
            EXPECT_GE(lambda, 0.5f);
            EXPECT_LE(lambda, 2.5f);

            previous = lambda;
        }
    }
}
