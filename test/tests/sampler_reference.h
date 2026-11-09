#pragma once

#include <cmath>

#include "wideband_config.h"

/**
 * Ideal, double precision model of the sampler's signal processing. This is the oracle that
 * test_sampler.cpp compares the firmware Sampler against.
 *
 * Deliberately independent from firmware/sampling.cpp: the firmware is free to change its
 * internal representation (float, fixed point) and arrangement of the math, but must stay
 * within tolerance of this model. Only change this if the *intended behavior* changes.
 */
class ReferenceSampler
{
public:
    void ApplySample(double nernst, double pumpCurrentVoltage, double virtualGround)
    {
        // Estimate where the previous sample would have been without the ESR toggle, by averaging
        // its neighbors. This cancels any slope in the DC component. See firmware/sampling.png
        double r2OppositePhase = (nernst + m_r3) / 2;

        double acLocal = std::abs(r2OppositePhase - m_r2);
        m_nernstDc = (r2OppositePhase + m_r2) / 2;

        // First order IIR filters, one step per sample
        m_nernstAc += ESR_SENSE_ALPHA * (acLocal - m_nernstAc);
        m_pumpVoltage += PUMP_FILTER_ALPHA * ((pumpCurrentVoltage - virtualGround) - m_pumpVoltage);

        m_r3 = m_r2;
        m_r2 = nernst;
    }

    double NernstDc() const { return m_nernstDc; }
    double NernstAc() const { return m_nernstAc; }

    // mA, sense resistor behind an amplifier
    double PumpNominalCurrent() const
    {
        return m_pumpVoltage * -1000.0 / (PUMP_CURRENT_SENSE_GAIN * LSU_SENSE_R);
    }

    // ohms, sensor is the low side of a divider with esrSupplyR on top, VCC pk-pk injected
    double InternalResistance(double esrSupplyR) const
    {
        double totalEsr = esrSupplyR * m_nernstAc / (VCC_VOLTS - m_nernstAc);
        return totalEsr - VM_RESISTOR_VALUE;
    }

private:
    double m_r2 = 0;
    double m_r3 = 0;
    double m_nernstDc = 0;
    double m_nernstAc = 0;
    double m_pumpVoltage = 0;
};
