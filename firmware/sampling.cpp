#include "sampling.h"

#include "ch.h"
#include "hal.h"

#include "wideband_config.h"

#include "port.h"
#include "io_pins.h"
#include "livedata.h"

#include <rusefi/interpolation.h>

// Last point is approximated by the greatest measurable sensor resistance
static const float lsu49TempBins[]   = {   80, 100, 150, 200, 250, 300, 350, 400, 450, 550, 650, 800, 1000, 1200, 2500, 4500 };
static const float lsu49TempValues[] = { 1030, 972, 888, 840, 806, 780, 761, 744, 729, 703, 686, 665,  642,  628,  567,  500 };

static const float lsu42TempBins[]   = {   35,  40,  50,  60,  70,  80,  90, 100, 120, 150, 200, 250, 300, 400, 450, 500, 600, 700, 800, 900, 1000, 1100 };
static const float lsu42TempValues[] = { 1199, 961, 857, 806, 775, 750, 730, 715, 692, 666, 635, 613, 598, 574, 564, 556, 543, 535, 528, 521,  514,  503 };

static const float lsuAdvTempBins[]   = {   53,  96, 130, 162, 184, 206, 239, 278, 300, 330, 390, 462, 573, 730, 950, 1200, 1500, 1900, 2500, 3500, 5000, 6000 };
static const float lsuAdvTempValues[] = { 1198, 982, 914, 875, 855, 838, 816, 794, 785, 771, 751, 732, 711, 691, 671,  653,  635,  614,  588,  562,  537,  528 };

struct Sampler : public ISampler {
public:
    void ApplySample(AnalogChannelResult& result, float virtualGroundVoltageInt);

    float GetNernstDc() const override {
        return nernstDc;
    }

    float GetNernstAc() const override {
        return nernstAc;
    }

    float GetPumpNominalCurrent() const override {
        // Gain is 10x, then a 61.9 ohm resistor
        // Effective resistance with the gain is 619 ohms
        // 1000 is to convert to milliamperes
        constexpr float ratio = -1000 / (PUMP_CURRENT_SENSE_GAIN * LSU_SENSE_R);
        return pumpCurrentSenseVoltage * ratio;
    }

    float GetInternalBatteryVoltage() const override {
        // Dual HW can measure heater voltage for each channel
        // by measuring voltage on Heater- while FET is off
        // TODO: rename function?
        return internalBatteryVoltage;
    }

private:
    float r_2 = 0;
    float r_3 = 0;

    float nernstAc;
    float nernstDc;
    float pumpCurrentSenseVoltage;
    float internalBatteryVoltage;
};

static Sampler samplers[AFR_CHANNELS];

constexpr float f_abs(float x)
{
    return x > 0 ? x : -x;
}

static THD_WORKING_AREA(waSamplingThread, 256);

static void SamplingThread(void*)
{
    chRegSetThreadName("Sampling");

    SetupESRDriver(GetSensorType());

    /* GD32: Insert 20us delay after ADC enable */
    chThdSleepMilliseconds(1);

    while(true)
    {
        auto result = AnalogSample();

        // Toggle the pin after sampling so that any switching noise occurs while we're doing our math instead of when sampling
        ToggleESRDriver(GetSensorType());

        for (int ch = 0; ch < AFR_CHANNELS; ch++)
        {
            samplers[ch].ApplySample(result.ch[ch], result.VirtualGroundVoltageInt);
        }

#if defined(TS_ENABLED)
        /* tunerstudio */
        SamplingUpdateLiveData();
#endif
    }
}

void Sampler::ApplySample(AnalogChannelResult& result, float virtualGroundVoltageInt)
{
    float r_1 = result.NernstVoltage;

    // r2_opposite_phase estimates where the previous sample would be had we not been toggling
    // AKA the absolute value of the difference between r2_opposite_phase and r2 is the amplitude
    // of the AC component on the nernst voltage.  We have to pull this trick so as to use the past 3
    // samples to cancel out any slope in the DC (aka actual nernst cell output) from the AC measurement
    // See firmware/sampling.png for a drawing of what's going on here
    float r2_opposite_phase = (r_1 + r_3) / 2;

    // Compute AC (difference) and DC (average) components
    float nernstAcLocal = f_abs(r2_opposite_phase - r_2);
    nernstDc = (r2_opposite_phase + r_2) / 2;

    nernstAc =
        (1 - ESR_SENSE_ALPHA) * nernstAc +
        ESR_SENSE_ALPHA * nernstAcLocal;

    // Exponential moving average (aka first order lpf)
    pumpCurrentSenseVoltage =
        (1 - PUMP_FILTER_ALPHA) * pumpCurrentSenseVoltage +
        PUMP_FILTER_ALPHA * (result.PumpCurrentVoltage - virtualGroundVoltageInt);

#ifdef BATTERY_INPUT_DIVIDER
    internalBatteryVoltage = result.BatteryVoltage;
#endif

    // Shift history over by one
    r_3 = r_2;
    r_2 = r_1;
}

void StartSampling()
{
    adcStart(&ADCD1, nullptr);
    chThdCreateStatic(waSamplingThread, sizeof(waSamplingThread), NORMALPRIO + 5, SamplingThread, nullptr);
}

float GetNernstAc(int ch)
{
    return samplers[ch].GetNernstAc();
}

float GetSensorInternalResistance(int ch)
{
    // Sensor is the lowside of a divider, top side is GetESRSupplyR(), and 3.3v AC pk-pk is injected
    float totalEsr = GetESRSupplyR() / (VCC_VOLTS / GetNernstAc(ch) - 1);

    // There is a resistor between the opamp and Vm sensor pin.  Remove the effect of that
    // resistor so that the remainder is only the ESR of the sensor itself
    return totalEsr - VM_RESISTOR_VALUE;
}

float GetSensorTemperature(int ch)
{
    float esr = GetSensorInternalResistance(ch);

    if (esr > 5000)
    {
        return 0;
    }

    switch (GetSensorType()) {
        case SensorType::LSU49:
            return interpolate2d(esr, lsu49TempBins, lsu49TempValues);
        case SensorType::LSU42:
            return interpolate2d(esr, lsu42TempBins, lsu42TempValues);
        case SensorType::LSUADV:
            return interpolate2d(esr, lsuAdvTempBins, lsuAdvTempValues);
    }

    return 0;
}

float GetNernstDc(int ch)
{
    return samplers[ch].GetNernstDc();
}

float GetPumpNominalCurrent(int ch)
{
    return samplers[ch].GetPumpNominalCurrent();
}

float GetInternalBatteryVoltage(int ch)
{
    return samplers[ch].GetInternalBatteryVoltage();
}
