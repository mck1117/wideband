#include "analog_input.h"

#include "hal.h"

#define ADC_OVERSAMPLE 8

static adcsample_t adcBuffer[3 * ADC_OVERSAMPLE];

ADCConversionGroup convGroup =
{
    false, 3, nullptr, nullptr,
    0,                  // CFGR1
    ADC_TR(0, 0),       // TR
    ADC_SMPR_SMP_28P5,      // SMPR
    ADC_CHSELR_CHSEL0 | ADC_CHSELR_CHSEL1 | ADC_CHSELR_CHSEL2
};

static float AverageSamples(adcsample_t* buffer, size_t idx)
{
    uint32_t sum = 0;

    for (size_t i = 0; i < ADC_OVERSAMPLE; i++)
    {
        sum += buffer[idx];
        idx += ADC_OVERSAMPLE;
    }

    return (float)sum / ADC_OVERSAMPLE;
}

AnalogResult AnalogSample()
{
    adcConvert(&ADCD1, &convGroup, adcBuffer, ADC_OVERSAMPLE);

    return
    {
        .NernstVoltage = AverageSamples(adcBuffer, 0),
        .VirtualGroundVoltage = AverageSamples(adcBuffer, 1),
        .PumpCurrentVoltage = AverageSamples(adcBuffer, 2),
    };
}
