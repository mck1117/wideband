#include "analog_input.h"

#include "hal.h"

#define ADC_CHANNEL_COUNT 3
#define ADC_OVERSAMPLE 4

static adcsample_t adcBuffer[ADC_CHANNEL_COUNT * ADC_OVERSAMPLE];

ADCConversionGroup convGroup =
{
    false,
    ADC_CHANNEL_COUNT,
    nullptr,
    nullptr,
    ADC_CFGR1_CONT | ADC_CFGR1_RES_12BIT,                  // CFGR1
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
        idx += ADC_CHANNEL_COUNT;
    }

    constexpr float scale = 3.3f / (4095 * ADC_OVERSAMPLE);

    return (float)sum * scale;
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
