#pragma once

#include "wideband_config.h"
#include "sampler_math.h"

#include "timer.h"

struct ISampler
{
    // Nernst cell DC voltage, volts
    virtual float GetNernstDc() const = 0;
    // Nernst cell AC (ESR injection) peak-to-peak voltage, volts
    virtual float GetNernstAc() const = 0;
    // Pump cell current, mA (positive = lean)
    virtual float GetPumpNominalCurrent() const = 0;
    // Heater supply voltage, volts
    virtual float GetInternalHeaterVoltage() const = 0;
    // Sensor temperature estimated from ESR, deg C (0 if ESR is out of range)
    virtual float GetSensorTemperature() const = 0;
    // Sensor Nernst cell internal resistance, ohms
    virtual float GetSensorInternalResistance() const = 0;
    virtual float GetLambda() const = 0;
};

template <typename TVoltage>
struct AnalogChannelResultT;

/**
 * Per-sample signal processing for one sensor channel. All sampler math lives behind this class:
 *
 * Math: the Math policy (see sampler_math.h) decides how voltages are represented and computed,
 *   eg. float on CPUs with an FPU, fixed point without. Boards use Sampler<SamplerMath>.
 *
 * Input: ApplySample() is called once per sample period with ADC readings already converted to
 *   Math::Voltage by the port. The ESR driver toggles between consecutive samples, so the Nernst
 *   input alternates between (DC + AC/2) and (DC - AC/2).
 *
 * Filtering is defined per sample: ESR_SENSE_ALPHA and PUMP_FILTER_ALPHA are first order IIR
 *   coefficients applied once per ApplySample() call, so time constants scale with sample rate.
 *
 * Output: the ISampler getters. They're called from other (lower priority) threads while
 *   ApplySample() runs, so each getter must compute its result from state that ApplySample()
 *   writes with a single store (one float/int32), never from multiple members updated separately.
 */
template <typename Math>
class Sampler final : public ISampler
{
public:
    using Voltage = typename Math::Voltage;
    using Input = AnalogChannelResultT<Voltage>;

    void ApplySample(const Input& result, Voltage virtualGroundVoltageInt);
    void Init();

    float GetNernstDc() const override;
    float GetNernstAc() const override;
    float GetPumpNominalCurrent() const override;
    float GetInternalHeaterVoltage() const override;
    float GetSensorTemperature() const override;
    float GetSensorInternalResistance() const override;
    float GetLambda() const override;

private:
    static constexpr typename Math::Coefficient EsrAlpha = Math::MakeCoefficient(ESR_SENSE_ALPHA);
    static constexpr typename Math::Coefficient PumpAlpha = Math::MakeCoefficient(PUMP_FILTER_ALPHA);

    Voltage r_2{};
    Voltage r_3{};

    Voltage nernstAc{};
    Voltage nernstDc{};
    Voltage pumpCurrentSenseVoltage{};

#ifdef BOARD_HAS_VOLTAGE_SENSE
    float internalHeaterVoltage = 0;
#endif

    Timer m_startupTimer;
};

// The sampler type this board uses
using BoardSampler = Sampler<SamplerMath>;

// Get the sampler for a particular channel. Returns the concrete (final) type rather than ISampler so
// that calls are resolved at compile time instead of through the vtable.
const BoardSampler& GetSampler(int ch);

#ifdef BOARD_HAS_VOLTAGE_SENSE
float GetSupplyVoltage();
#endif

float GetMcuTemperature();

void StartSampling();
