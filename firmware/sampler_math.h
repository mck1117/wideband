#pragma once

#include <cstdint>

/**
 * Math policies for Sampler<Math>: how the sampler represents voltages and does its per-sample math.
 *
 * A policy provides:
 *   Voltage            a voltage, supporting + and -
 *   Coefficient        a first order filter coefficient
 *   CountScale         a volts-per-ADC-count scale, for building Voltages from integer ADC readings
 *
 *   constexpr Coefficient MakeCoefficient(float alpha)
 *   constexpr CountScale  MakeCountScale(double voltsPerCount, uint32_t maxCount)
 *   Voltage FromCounts(uint32_t counts, CountScale scale)
 *   Voltage FromVolts(float volts)
 *   float   ToVolts(Voltage v)
 *   Voltage Half(Voltage v)
 *   Voltage Abs(Voltage v)
 *   Voltage Filter(Voltage state, Voltage input, Coefficient alpha)   one step of a first order IIR
 */

// Plain float: for CPUs with an FPU
struct FloatSamplerMath
{
    using Voltage = float;
    using Coefficient = float;
    using CountScale = float;

    static constexpr Coefficient MakeCoefficient(float alpha)
    {
        return alpha;
    }

    static constexpr CountScale MakeCountScale(double voltsPerCount, uint32_t /*maxCount*/)
    {
        return voltsPerCount;
    }

    static Voltage FromCounts(uint32_t counts, CountScale scale)
    {
        return counts * scale;
    }

    static constexpr Voltage FromVolts(float volts)
    {
        return volts;
    }

    static constexpr float ToVolts(Voltage v)
    {
        return v;
    }

    static Voltage Half(Voltage v)
    {
        return v * 0.5f;
    }

    static Voltage Abs(Voltage v)
    {
        return v > 0 ? v : -v;
    }

    static Voltage Filter(Voltage state, Voltage input, Coefficient alpha)
    {
        // Deliberately not state + alpha * (input - state): with -ffast-math this arrangement lets the
        // compiler fold constant scaling of the input (eg. ADC scale) in to the coefficient.
        return (1 - alpha) * state + alpha * input;
    }
};

// Volts as a signed fixed point number with 26 fractional bits: 15nV resolution, +-32V range.
// A distinct type rather than a bare int32_t so that a float volts value can't silently convert in to one.
struct FixedVolts
{
    int32_t raw;
};

constexpr FixedVolts operator+(FixedVolts a, FixedVolts b)
{
    return {a.raw + b.raw};
}

constexpr FixedVolts operator-(FixedVolts a, FixedVolts b)
{
    return {a.raw - b.raw};
}

// Integer only: for CPUs without an FPU, where soft float costs 100-600 cycles per operation
struct FixedSamplerMath
{
    using Voltage = FixedVolts;

    // Filter coefficient k / 2^bits, with k < 2^16. Each coefficient gets as many fractional bits as
    // fit, so that small coefficients (eg. 0.002) are still precise.
    struct Coefficient
    {
        uint32_t k;
        uint32_t bits;
    };

    // Volts per ADC count, as whole + frac / 2^fracBits LSBs per count
    struct CountScale
    {
        uint32_t whole;
        uint32_t frac;
        uint32_t fracBits;
    };

    static constexpr int FractionalBits = 26;
    static constexpr float OneVolt = 1 << FractionalBits;

    static constexpr Coefficient MakeCoefficient(float alpha)
    {
        // alpha must be in (0, 1)
        uint32_t bits = 16;
        while (bits < 31 && static_cast<double>(alpha) * static_cast<double>(1ull << (bits + 1)) + 0.5 < 65536)
        {
            bits++;
        }

        return {static_cast<uint32_t>(static_cast<double>(alpha) * static_cast<double>(1ull << bits) + 0.5), bits};
    }

    // Splitting the scale in to whole and fractional parts lets FromCounts be precise without a 64 bit
    // multiply: fracBits is chosen such that counts * frac can't overflow 32 bits.
    static constexpr CountScale MakeCountScale(double voltsPerCount, uint32_t maxCount)
    {
        uint32_t fracBits = 0;
        while (fracBits < 31 && static_cast<uint64_t>(maxCount) * ((1ull << (fracBits + 1)) - 1) <= 0xFFFFFFFFull)
        {
            fracBits++;
        }

        double lsbPerCount = voltsPerCount * OneVolt;
        auto whole = static_cast<uint32_t>(lsbPerCount);
        auto frac = static_cast<uint32_t>((lsbPerCount - whole) * static_cast<double>(1ull << fracBits) + 0.5);

        // Rounding the fraction up may carry in to the whole part
        if (frac >> fracBits)
        {
            whole++;
            frac = 0;
        }

        return {whole, frac, fracBits};
    }

    static Voltage FromCounts(uint32_t counts, CountScale scale)
    {
        return {static_cast<int32_t>(counts * scale.whole + ((counts * scale.frac) >> scale.fracBits))};
    }

    static constexpr Voltage FromVolts(float volts)
    {
        float scaled = volts * OneVolt;
        return {static_cast<int32_t>(scaled + (scaled >= 0 ? 0.5f : -0.5f))};
    }

    static float ToVolts(Voltage v)
    {
        return v.raw * (1 / OneVolt);
    }

    static Voltage Half(Voltage v)
    {
        // Arithmetic shift, so this rounds toward -inf (well defined for negatives as of C++20)
        return {v.raw >> 1};
    }

    static Voltage Abs(Voltage v)
    {
        return {v.raw < 0 ? -v.raw : v.raw};
    }

    static Voltage Filter(Voltage state, Voltage input, Coefficient alpha)
    {
        // Rounding (rather than truncating) keeps the steady state error symmetric and small:
        // the filter stops moving once |input - state| < 0.5 / alpha LSB (3.7uV for alpha = 0.002)
        return {state.raw + Multiply(input.raw - state.raw, alpha)};
    }

private:
    // Rounded x * c.k / 2^c.bits, without a 64 bit multiply (a slow library call on Cortex-M0).
    // Split x in to halves, x = hi * 2^16 + lo with 0 <= lo < 2^16, then
    //   x * k / 2^bits = (hi * k) / 2^(bits - 16) + (lo * k) / 2^bits
    static int32_t Multiply(int32_t x, Coefficient c)
    {
        int32_t hi = x >> 16;
        uint32_t lo = static_cast<uint32_t>(x) & 0xFFFF;

        // |hi * k| < 2^31 and lo * k + 2^15 < 2^32 (k < 2^16), so neither overflows
        int32_t a = hi * static_cast<int32_t>(c.k);
        uint32_t b = (lo * c.k + 0x8000) >> 16;

        uint32_t s = c.bits - 16;
        if (s == 0)
        {
            return a + static_cast<int32_t>(b);
        }

        // Shift a down, keeping its remainder to combine with b before rounding
        int32_t aWhole = a >> s;
        uint32_t aRemainder = static_cast<uint32_t>(a) & ((1u << s) - 1);

        return aWhole + static_cast<int32_t>((aRemainder + b + (1u << (s - 1))) >> s);
    }
};

// The math used by this board's sampler: float if the CPU has an FPU, fixed point otherwise.
// A board can override this by defining SAMPLER_MATH as FloatSamplerMath or FixedSamplerMath.
#if defined(SAMPLER_MATH)
using SamplerMath = SAMPLER_MATH;
#elif defined(__ARM_FP) || !defined(__arm__)
using SamplerMath = FloatSamplerMath;
#else
using SamplerMath = FixedSamplerMath;
#endif

using SamplerVoltage = SamplerMath::Voltage;
