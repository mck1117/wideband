#pragma once

#include "wideband_config.h"

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

struct AnalogChannelResult;

/**
 * Per-sample signal processing for one sensor channel. All sampler math lives behind this class:
 *
 * Input: ApplySample() is called once per sample period with ADC readings already converted to
 *   volts by the port. The ESR driver toggles between consecutive samples, so the Nernst input
 *   alternates between (DC + AC/2) and (DC - AC/2).
 *
 * Filtering is defined per sample: ESR_SENSE_ALPHA and PUMP_FILTER_ALPHA are first order IIR
 *   coefficients applied once per ApplySample() call, so time constants scale with sample rate.
 *
 * Output: the ISampler getters. They're called from other (lower priority) threads while
 *   ApplySample() runs, so each getter must compute its result from state that ApplySample()
 *   writes with a single store (one float/int32), never from multiple members updated separately.
 */
class Sampler : public ISampler
{
public:
    void ApplySample(const AnalogChannelResult& result, float virtualGroundVoltageInt);
    void Init();

    float GetNernstDc() const override;
    float GetNernstAc() const override;
    float GetPumpNominalCurrent() const override;
    float GetInternalHeaterVoltage() const override;
    float GetSensorTemperature() const override;
    float GetSensorInternalResistance() const override;
    float GetLambda() const override;

private:
    float r_2 = 0;
    float r_3 = 0;

    float nernstAc = 0;
    float nernstDc = 0;
    float pumpCurrentSenseVoltage = 0;

#ifdef BOARD_HAS_VOLTAGE_SENSE
    float internalHeaterVoltage = 0;
#endif

    Timer m_startupTimer;
};

// Get the sampler for a particular channel
const ISampler& GetSampler(int ch);

#ifdef BOARD_HAS_VOLTAGE_SENSE
float GetSupplyVoltage();
#endif

float GetMcuTemperature();

void StartSampling();
