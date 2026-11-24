#include <gtest/gtest.h>

#include <cmath>
#include <cstdint>

#include "sampler_math.h"

// Primitive operations of the fixed point sampler math policy. Sampler behavior as a whole
// (for every policy) is covered by test_sampler.cpp.

using M = FixedSamplerMath;

namespace
{
// One LSB of FixedVolts
constexpr double Lsb = 1.0 / (1 << FixedSamplerMath::FractionalBits);

double Volts(FixedVolts v)
{
    return v.raw * Lsb;
}
} // namespace

TEST(FixedSamplerMath, FromVoltsToVoltsRoundTrip)
{
    for (float v : {-6.6f, -1.65f, -1e-6f, 0.0f, 1e-6f, 0.45f, 1.65f, 3.3f, 6.6f})
    {
        EXPECT_NEAR(v, Volts(M::FromVolts(v)), Lsb) << v;
        EXPECT_NEAR(v, M::ToVolts(M::FromVolts(v)), 1e-6 + 1e-6 * std::abs(v)) << v;
    }
}

TEST(FixedSamplerMath, FromCounts)
{
    // ADC configurations used by boards: 12 bit, summed over 24 (F0, F1 rev2/3) or 16 (F1 dual)
    // samples, with and without a Nernst input gain folded in to the scale.
    struct Config
    {
        uint32_t maxCount;
        double gain;
    };

    for (auto config : {Config{4095 * 24, 1}, Config{4095 * 24, 2.7}, Config{4095 * 16, 1}, Config{4095 * 16, 3.15}})
    {
        double voltsPerCount = 3.3 / config.maxCount / config.gain;
        auto scale = M::MakeCountScale(voltsPerCount, config.maxCount);

        // Scaling must not overflow at full scale
        EXPECT_LE(static_cast<uint64_t>(scale.frac) * config.maxCount, 0xFFFFFFFFull);
        EXPECT_LE(static_cast<uint64_t>(scale.whole) * config.maxCount, 0x7FFFFFFFull);

        for (uint32_t counts : {0u, 1u, 2u, 1000u, config.maxCount / 2, config.maxCount - 1, config.maxCount})
        {
            double expected = counts * voltsPerCount;
            // Truncation and scale rounding error: a tiny fraction of one ADC count
            EXPECT_NEAR(expected, Volts(M::FromCounts(counts, scale)), 2 * Lsb + 1e-6 * expected)
                << config.maxCount << " " << config.gain << " " << counts;
        }
    }
}

TEST(FixedSamplerMath, HalfAndAbs)
{
    EXPECT_EQ(5, M::Half({10}).raw);
    EXPECT_EQ(-5, M::Half({-10}).raw);
    // Rounds toward -inf
    EXPECT_EQ(-6, M::Half({-11}).raw);

    EXPECT_EQ(7, M::Abs({7}).raw);
    EXPECT_EQ(7, M::Abs({-7}).raw);
    EXPECT_EQ(0, M::Abs({0}).raw);
}

TEST(FixedSamplerMath, CoefficientPrecision)
{
    for (float alpha : {0.002f, 0.02f, 0.1f, 0.5f})
    {
        auto c = M::MakeCoefficient(alpha);
        EXPECT_LT(c.k, 65536u);
        EXPECT_NEAR(alpha, c.k / std::pow(2.0, c.bits), 1e-4 * alpha) << alpha;
    }
}

TEST(FixedSamplerMath, FilterStepMatchesAlpha)
{
    const auto alpha = M::MakeCoefficient(0.02f);
    const double exactAlpha = alpha.k / std::pow(2.0, alpha.bits);

    // Largest steps the sampler can see, in both directions: rail to rail on a +-3.3V difference,
    // plus small ones
    for (auto [from, to] : {std::pair{-3.3f, 3.3f}, std::pair{3.3f, -3.3f}, std::pair{0.0f, 1e-3f}, std::pair{1e-3f, 0.0f}})
    {
        auto state = M::FromVolts(from);
        auto input = M::FromVolts(to);

        auto next = M::Filter(state, input, alpha);

        double expected = Volts(state) + exactAlpha * (Volts(input) - Volts(state));
        EXPECT_NEAR(expected, Volts(next), Lsb) << from << " -> " << to;
    }
}

TEST(FixedSamplerMath, FilterConvergesFromRails)
{
    for (float alpha : {0.002f, 0.02f})
    {
        const auto k = M::MakeCoefficient(alpha);

        for (float target : {-3.3f, -1e-4f, 0.0f, 1e-4f, 0.45f, 3.3f})
        {
            for (float start : {-3.3f, 3.3f})
            {
                auto state = M::FromVolts(start);
                auto input = M::FromVolts(target);

                for (int i = 0; i < static_cast<int>(40 / alpha); i++)
                {
                    state = M::Filter(state, input, k);
                }

                // Steady state error is bounded by 0.5 / alpha LSB (rounding dead band)
                EXPECT_NEAR(target, Volts(state), (0.5 / alpha + 1) * Lsb) << alpha << " " << start << " -> " << target;
            }
        }
    }
}
