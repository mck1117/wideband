#include <gtest/gtest.h>

#include <cmath>
#include <cstdint>
#include <functional>
#include <random>
#include <string>
#include <type_traits>

#include "sampling.h"
#include "sampler_math.h"
#include "port.h"
#include "lambda_conversion.h"
#include "timer.h"

#include "test_stubs.h"
#include "tests/sampler_reference.h"

/**
 * These tests treat Sampler as a black box: they only use ApplySample() and the ISampler getters,
 * and compare against physical expectations or the ideal model in sampler_reference.h.
 * Every test runs against every math policy (see sampler_math.h).
 *
 * The tolerances below are the contract for any math policy (float, fixed point).
 * They're derived from the input resolution: one count of the oversampled ADC sum is
 * VCC / (4095 * 24) = 34uV, or 12uV at the Nernst input after its gain.
 */

namespace
{

constexpr double VoltageTolerance = 100e-6;         // 0.1mV, ~8 counts at the Nernst input
constexpr double CurrentTolerance = 0.2e-3;         // mA, ~0.12mV at the pump current sense
constexpr double LambdaTolerance = 0.5e-3;
constexpr double ResistanceTolerance = 1;           // ohms...
constexpr double RelativeResistanceTolerance = 2e-3; // ...plus 0.2%
constexpr double TemperatureTolerance = 2;          // deg C

// Volts per count of the oversampled ADC sum
constexpr double AdcLsb = VCC_VOLTS / (ADC_MAX_COUNT * ADC_OVERSAMPLE);
constexpr double NernstLsb = AdcLsb / NERNST_INPUT_GAIN;
constexpr double NernstFullScale = VCC_VOLTS / NERNST_INPUT_GAIN;

constexpr double PumpMilliampsPerVolt = -1000.0 / (PUMP_CURRENT_SENSE_GAIN * LSU_SENSE_R);

double Clamp(double v, double lo, double hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

// Round a voltage to what the ADC path can actually produce
float QuantizeAdc(double volts)
{
    return std::round(Clamp(volts, 0, VCC_VOLTS) / AdcLsb) * AdcLsb;
}

float QuantizeNernst(double volts)
{
    return std::round(Clamp(volts, 0, NernstFullScale) / NernstLsb) * NernstLsb;
}

// Uniform integer noise in [-amplitude, amplitude], deterministic across platforms
int Noise(std::mt19937& rng, int amplitude)
{
    return static_cast<int>(rng() % (2 * amplitude + 1)) - amplitude;
}

// The ESR driver toggles every sample, so the Nernst voltage alternates around its DC value
double WithAc(double dc, double acPkPk, int sample)
{
    return dc + ((sample & 1) ? acPkPk / 2 : -acPkPk / 2);
}

// AC pk-pk seen at the Nernst input for a given sensor internal resistance
double AcForResistance(double ohms)
{
    double total = ohms + VM_RESISTOR_VALUE;
    return VCC_VOLTS * total / (TestEsrSupplyR + total);
}

// The only place the tests construct sampler input. If ApplySample's input type changes
// (eg. raw ADC counts), this is the only thing that needs to change.
template <typename Math>
void Feed(Sampler<Math>& dut, float nernst, float pump, float virtualGround)
{
    typename Sampler<Math>::Input result{};
    result.NernstVoltage = Math::FromVolts(nernst);
    result.PumpCurrentVoltage = Math::FromVolts(pump);
    result.HeaterSupplyVoltage = 0;

    dut.ApplySample(result, Math::FromVolts(virtualGround));
}

// Drives a Sampler with steady inputs, keeping track of ESR toggle phase between calls
template <typename Math>
struct Harness
{
    Sampler<Math> dut;
    int sample = 0;

    Harness()
    {
        dut.Init();
    }

    void Run(int count, double nernstDc, double nernstAc, double pump, double virtualGround = HALF_VCC)
    {
        for (int i = 0; i < count; i++, sample++)
        {
            Feed(dut, QuantizeNernst(WithAc(nernstDc, nernstAc, sample)), QuantizeAdc(pump), QuantizeAdc(virtualGround));
        }
    }
};

// Pump current the sampler should report for these inputs, accounting for ADC quantization
double ExpectedPumpCurrent(double pump, double virtualGround)
{
    return (QuantizeAdc(pump) - QuantizeAdc(virtualGround)) * PumpMilliampsPerVolt;
}

struct Stimulus
{
    double nernst;
    double pump;
    double virtualGround;
};

// Run the firmware sampler and the ideal model through the same inputs, checking every output
// after every sample.
template <typename Math>
void ExpectMatchesReference(int samples, const std::function<Stimulus(int)>& scenario)
{
    Sampler<Math> dut;
    dut.Init();
    ReferenceSampler ref;

    for (int i = 0; i < samples; i++)
    {
        Stimulus s = scenario(i);
        float nernst = QuantizeNernst(s.nernst);
        float pump = QuantizeAdc(s.pump);
        float virtualGround = QuantizeAdc(s.virtualGround);

        Feed(dut, nernst, pump, virtualGround);
        ref.ApplySample(nernst, pump, virtualGround);

        // ASSERT so that we stop at the first mismatch rather than print thousands
        ASSERT_NEAR(ref.NernstDc(), dut.GetNernstDc(), VoltageTolerance) << "sample " << i;
        ASSERT_NEAR(ref.NernstAc(), dut.GetNernstAc(), VoltageTolerance) << "sample " << i;

        double refCurrent = ref.PumpNominalCurrent();
        ASSERT_NEAR(refCurrent, dut.GetPumpNominalCurrent(), CurrentTolerance) << "sample " << i;

        // The lambda curve has small steps at its clamp points, so accept anything the curve
        // produces within current tolerance of the reference.
        float lambdaA = ConvertLambda(refCurrent - CurrentTolerance);
        float lambdaB = ConvertLambda(refCurrent);
        float lambdaC = ConvertLambda(refCurrent + CurrentTolerance);
        float lambda = dut.GetLambda();
        ASSERT_GE(lambda, std::min({lambdaA, lambdaB, lambdaC}) - LambdaTolerance) << "sample " << i;
        ASSERT_LE(lambda, std::max({lambdaA, lambdaB, lambdaC}) + LambdaTolerance) << "sample " << i;

        // Above ~5k ohm the sensor is cold enough that temperature reads 0, and ESR becomes
        // extremely sensitive to AC as it approaches VCC, so only check the useful range.
        double refEsr = ref.InternalResistance(TestEsrSupplyR);
        if (refEsr < 5000)
        {
            ASSERT_NEAR(refEsr, dut.GetSensorInternalResistance(),
                        ResistanceTolerance + RelativeResistanceTolerance * std::abs(refEsr))
                << "sample " << i;
        }
    }
}

struct MathPolicyNames
{
    template <typename T>
    static std::string GetName(int)
    {
        if constexpr (std::is_same_v<T, FloatSamplerMath>)
        {
            return "Float";
        }
        else
        {
            return "Fixed";
        }
    }
};

using MathPolicies = ::testing::Types<FloatSamplerMath, FixedSamplerMath>;

} // namespace

template <typename Math>
class SamplerVsReference : public ::testing::Test
{
};
TYPED_TEST_SUITE(SamplerVsReference, MathPolicies, MathPolicyNames);

template <typename Math>
class SamplerTest : public ::testing::Test
{
};
TYPED_TEST_SUITE(SamplerTest, MathPolicies, MathPolicyNames);

// *******************************
// Firmware vs. ideal model
// *******************************

TYPED_TEST(SamplerVsReference, SensorWarmup)
{
    std::mt19937 rng(1);

    ExpectMatchesReference<TypeParam>(40000, [&](int i) {
        // AC decays from cold (~1V) to operating temperature (~45mV) as the sensor heats
        double ac = 0.045 + (1.0 - 0.045) * std::exp(-i / 8000.0);

        return Stimulus{
            WithAc(0.45, ac, i) + Noise(rng, 3) * NernstLsb,
            HALF_VCC + 0.1 * std::sin(i / 3000.0) + Noise(rng, 3) * AdcLsb,
            HALF_VCC + Noise(rng, 1) * AdcLsb,
        };
    });
}

TYPED_TEST(SamplerVsReference, ClosedLoopOperation)
{
    std::mt19937 rng(2);
    double nernstDc = 0.45;
    double pump = HALF_VCC;

    ExpectMatchesReference<TypeParam>(40000, [&](int i) {
        // Random walk around the operating point, pump swings rich and lean
        nernstDc = Clamp(nernstDc + Noise(rng, 2) * 4 * NernstLsb, 0.3, 0.6);
        pump = Clamp(pump + Noise(rng, 2) * 20 * AdcLsb, 0.3, 3.0);

        return Stimulus{
            WithAc(nernstDc, 0.045, i) + Noise(rng, 3) * NernstLsb,
            pump + Noise(rng, 3) * AdcLsb,
            HALF_VCC + Noise(rng, 1) * AdcLsb,
        };
    });
}

TYPED_TEST(SamplerVsReference, DcRampWithAc)
{
    ExpectMatchesReference<TypeParam>(30000, [](int i) {
        return Stimulus{
            WithAc(0.05 + 1.05 * i / 30000.0, 0.1, i),
            HALF_VCC,
            HALF_VCC,
        };
    });
}

TYPED_TEST(SamplerVsReference, PumpSteps)
{
    const double levels[] = {HALF_VCC, 0, VCC_VOLTS, HALF_VCC + 0.3, HALF_VCC - 0.3, HALF_VCC};

    ExpectMatchesReference<TypeParam>(9000, [&](int i) {
        return Stimulus{
            WithAc(0.45, 0.05, i),
            levels[i / 1500],
            HALF_VCC,
        };
    });
}

TYPED_TEST(SamplerVsReference, Rails)
{
    ExpectMatchesReference<TypeParam>(15000, [](int i) {
        double nernst;
        if (i < 5000)
        {
            // Largest possible AC
            nernst = (i & 1) ? NernstFullScale : 0;
        }
        else if (i < 10000)
        {
            nernst = 0;
        }
        else
        {
            nernst = NernstFullScale;
        }

        return Stimulus{
            nernst,
            // Slam the pump input between rails
            ((i / 7) & 1) ? VCC_VOLTS : 0,
            ((i / 1000) & 1) ? 1.8 : 1.5,
        };
    });
}

// *******************************
// Physical behavior
// *******************************

TYPED_TEST(SamplerTest, InitialState)
{
    // Characterizes the state before any samples have been applied. Note that temperature reads
    // hot: with no AC measured yet, the ESR estimate is below the bottom of the table and clamps.
    Sampler<TypeParam> dut;
    dut.Init();

    EXPECT_NEAR(0, dut.GetNernstDc(), VoltageTolerance);
    EXPECT_NEAR(0, dut.GetNernstAc(), VoltageTolerance);
    EXPECT_NEAR(0, dut.GetPumpNominalCurrent(), CurrentTolerance);
    EXPECT_NEAR(1.004430, dut.GetLambda(), LambdaTolerance);
    EXPECT_NEAR(-VM_RESISTOR_VALUE, dut.GetSensorInternalResistance(), ResistanceTolerance);
    EXPECT_NEAR(1030, dut.GetSensorTemperature(), TemperatureTolerance);
}

TYPED_TEST(SamplerTest, TestDc)
{
    Harness<TypeParam> h;
    h.Run(5000, 0.45, 0, 1.75);

    // not exactly 0 because of filtering
    EXPECT_NEAR(0, h.dut.GetNernstAc(), 1e-3);
    EXPECT_NEAR(0.45, h.dut.GetNernstDc(), VoltageTolerance);
    EXPECT_NEAR(-0.1616, h.dut.GetPumpNominalCurrent(), 1e-3);
}

TYPED_TEST(SamplerTest, TestAc)
{
    Harness<TypeParam> h;
    h.Run(10000, 0.45, 0.2, 1.75);

    EXPECT_NEAR(0.2, h.dut.GetNernstAc(), 1e-3);
    EXPECT_NEAR(0.45, h.dut.GetNernstDc(), VoltageTolerance);
    EXPECT_NEAR(-0.1616, h.dut.GetPumpNominalCurrent(), 1e-3);
}

TYPED_TEST(SamplerTest, DcSteadyState)
{
    for (double dc : {0.0, 0.1, 0.45, 0.9, NernstFullScale})
    {
        Harness<TypeParam> h;
        h.Run(5000, dc, 0, HALF_VCC);

        EXPECT_NEAR(dc, h.dut.GetNernstDc(), VoltageTolerance) << dc;
        EXPECT_NEAR(0, h.dut.GetNernstAc(), VoltageTolerance) << dc;
    }
}

TYPED_TEST(SamplerTest, AcAmplitude)
{
    for (double ac : {0.002, 0.01, 0.045, 0.2, 1.0})
    {
        Harness<TypeParam> h;
        // 40 time constants of the ESR filter
        h.Run(20000, 0.6, ac, HALF_VCC);

        EXPECT_NEAR(ac, h.dut.GetNernstAc(), std::max(VoltageTolerance, 0.005 * ac)) << ac;
        EXPECT_NEAR(0.6, h.dut.GetNernstDc(), VoltageTolerance) << ac;
    }
}

TYPED_TEST(SamplerTest, DcSlopeRejection)
{
    // A fast DC ramp under the AC signal: AC must be unaffected, and DC tracks the ramp
    // exactly one sample behind (it's estimated at the previous sample's position).
    constexpr double slope = 2e-4; // volts per sample
    constexpr double ac = 0.05;

    Sampler<TypeParam> dut;
    dut.Init();

    for (int i = 0; i < 4000; i++)
    {
        Feed(dut, WithAc(0.15 + slope * i, ac, i), HALF_VCC, HALF_VCC);
    }

    EXPECT_NEAR(ac, dut.GetNernstAc(), VoltageTolerance);
    EXPECT_NEAR(0.15 + slope * 3998, dut.GetNernstDc(), VoltageTolerance);
}

TYPED_TEST(SamplerTest, PumpFilterStepResponse)
{
    // First order IIR with PUMP_FILTER_ALPHA per sample
    Harness<TypeParam> h;
    h.Run(1000, 0.45, 0.045, HALF_VCC);

    constexpr double step = 0.5;
    const double finalCurrent = ExpectedPumpCurrent(HALF_VCC + step, HALF_VCC);

    int applied = 0;
    for (int n : {1, 10, 50, 200, 1000})
    {
        h.Run(n - applied, 0.45, 0.045, HALF_VCC + step);
        applied = n;

        double expected = finalCurrent * (1 - std::pow(1 - PUMP_FILTER_ALPHA, n));
        EXPECT_NEAR(expected, h.dut.GetPumpNominalCurrent(), 0.005 * std::abs(finalCurrent)) << n;
    }
}

TYPED_TEST(SamplerTest, EsrFilterStepResponse)
{
    // First order IIR with ESR_SENSE_ALPHA per sample
    Harness<TypeParam> h;
    h.Run(10000, 0.45, 0.2, HALF_VCC);

    constexpr double from = 0.2;
    constexpr double to = 0.05;

    int applied = 0;
    for (int n : {250, 500, 1000, 3000})
    {
        h.Run(n - applied, 0.45, to, HALF_VCC);
        applied = n;

        double expected = to + (from - to) * std::pow(1 - ESR_SENSE_ALPHA, n);
        EXPECT_NEAR(expected, h.dut.GetNernstAc(), 0.006 * (from - to)) << n;
    }
}

TYPED_TEST(SamplerTest, SmallSignalsConverge)
{
    // A filter with too little internal resolution (eg. fixed point without enough fractional
    // bits) stalls short of small targets. Approach a small signal from both directions.
    {
        Harness<TypeParam> h;
        h.Run(20000, 0.45, 0.002, HALF_VCC);
        EXPECT_NEAR(0.002, h.dut.GetNernstAc(), 0.03 * 0.002) << "AC rising";

        h.Run(5000, 0.45, 0.1, HALF_VCC);
        h.Run(20000, 0.45, 0.002, HALF_VCC);
        EXPECT_NEAR(0.002, h.dut.GetNernstAc(), 0.03 * 0.002) << "AC falling";
    }

    {
        // One ADC count off virtual ground
        const double oneCount = ExpectedPumpCurrent(HALF_VCC + AdcLsb, HALF_VCC);

        Harness<TypeParam> h;
        h.Run(2000, 0.45, 0.045, HALF_VCC + AdcLsb);
        EXPECT_NEAR(oneCount, h.dut.GetPumpNominalCurrent(), 0.03 * std::abs(oneCount)) << "pump rising";

        h.Run(2000, 0.45, 0.045, HALF_VCC + 0.5);
        h.Run(2000, 0.45, 0.045, HALF_VCC + AdcLsb);
        EXPECT_NEAR(oneCount, h.dut.GetPumpNominalCurrent(), 0.03 * std::abs(oneCount)) << "pump falling";
    }
}

TYPED_TEST(SamplerTest, PumpCurrentScaling)
{
    for (double virtualGround : {1.6, static_cast<double>(HALF_VCC), 1.7})
    {
        for (double offset : {-1.5, -0.5, 0.0, 0.5, 1.5})
        {
            Harness<TypeParam> h;
            h.Run(2000, 0.45, 0.045, virtualGround + offset, virtualGround);

            EXPECT_NEAR(ExpectedPumpCurrent(virtualGround + offset, virtualGround), h.dut.GetPumpNominalCurrent(),
                        CurrentTolerance)
                << virtualGround << " " << offset;
        }
    }
}

TYPED_TEST(SamplerTest, InternalResistance)
{
    for (double ohms : {80.0, 300.0, 1000.0, 4500.0})
    {
        Harness<TypeParam> h;
        h.Run(20000, 0.45, AcForResistance(ohms), HALF_VCC);

        EXPECT_NEAR(ohms, h.dut.GetSensorInternalResistance(), ResistanceTolerance + RelativeResistanceTolerance * ohms)
            << ohms;
    }
}

TYPED_TEST(SamplerTest, SensorTemperature)
{
    struct Point
    {
        SensorType type;
        double ohms;
        double temperature;
    };

    const Point points[] = {
        {SensorType::LSU49, 80, 1030},
        {SensorType::LSU49, 125, 930}, // interpolated
        {SensorType::LSU49, 300, 780},
        {SensorType::LSU49, 4500, 500},
        {SensorType::LSU49, 6000, 0}, // out of range: too cold
        {SensorType::LSU42, 100, 715},
        {SensorType::LSUADV, 300, 785},
    };

    for (const auto& p : points)
    {
        ScopedSensorType type(p.type);

        Harness<TypeParam> h;
        h.Run(20000, 0.45, AcForResistance(p.ohms), HALF_VCC);

        EXPECT_NEAR(p.temperature, h.dut.GetSensorTemperature(), TemperatureTolerance)
            << static_cast<int>(p.type) << " " << p.ohms;
    }
}

TYPED_TEST(SamplerTest, Lambda)
{
    struct Point
    {
        double pumpCurrent;
        double lambda;
    };

    // LSU4.9, values from lambda_conversion
    const Point points[] = {
        {0, 1.004430},
        {0.5, 1.296168},
        {-1, 0.782118},
        {1.5, 2.0}, // clamped
    };

    for (const auto& p : points)
    {
        Harness<TypeParam> h;
        h.Run(2000, 0.45, 0.045, HALF_VCC + p.pumpCurrent / PumpMilliampsPerVolt);

        EXPECT_NEAR(p.lambda, h.dut.GetLambda(), LambdaTolerance) << p.pumpCurrent;
    }
}

TYPED_TEST(SamplerTest, HeaterVoltageStartupDelay)
{
    // Without supply voltage sensing, pretend we have battery voltage 5 seconds after Init()
    Timer::setMockTime(1'000'000);

    Sampler<TypeParam> dut;
    dut.Init();
    EXPECT_EQ(0, dut.GetInternalHeaterVoltage());

    Timer::advanceMockTime(4'900'000);
    EXPECT_EQ(0, dut.GetInternalHeaterVoltage());

    Timer::advanceMockTime(200'000);
    EXPECT_EQ(13, dut.GetInternalHeaterVoltage());
}
