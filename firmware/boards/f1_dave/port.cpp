#include "port.h"


#include "wideband_config.h"

#include "hal.h"

#define ADC_CHANNEL_COUNT 4
#define ADC_SAMPLE ADC_SAMPLE_7P5

static adcsample_t adcBuffer[ADC_CHANNEL_COUNT * ADC_OVERSAMPLE];

ADCConversionGroup convGroup =
{
    .circular = false,
    .num_channels = ADC_CHANNEL_COUNT,
    .end_cb = nullptr,
    .error_cb = nullptr,
    .cr1 = 0,
    .cr2 = ADC_CR2_CONT,
    .smpr1 = 0,
    .smpr2 = ADC_SMPR2_SMP_AN3(ADC_SAMPLE) | ADC_SMPR2_SMP_AN0(ADC_SAMPLE) |
             ADC_SMPR2_SMP_AN2(ADC_SAMPLE) | ADC_SMPR2_SMP_AN5(ADC_SAMPLE),
    .sqr1 = ADC_SQR1_NUM_CH(ADC_CHANNEL_COUNT),
    .sqr2 = 0,
    .sqr3 = ADC_SQR3_SQ1_N(3) | ADC_SQR3_SQ2_N(0) |
            ADC_SQR3_SQ3_N(2) | ADC_SQR3_SQ4_N(5)
};

static float AverageSamples(adcsample_t* buffer, size_t idx)
{
    uint32_t sum = 0;

    for (size_t i = 0; i < ADC_OVERSAMPLE; i++)
    {
        sum += buffer[idx];
        idx += ADC_CHANNEL_COUNT;
    }

    constexpr float scale = VCC_VOLTS / (ADC_MAX_COUNT * ADC_OVERSAMPLE);

    return (float)sum * scale;
}

AnalogResult AnalogSample()
{
    adcConvert(&ADCD1, &convGroup, adcBuffer, ADC_OVERSAMPLE);

    return
    {
        .ch =
        {
            {
                .NernstVoltage = AverageSamples(adcBuffer, 0) * NERNST_INPUT_GAIN,
                .PumpCurrentVoltage = AverageSamples(adcBuffer, 1),
                .BatteryVoltage = AverageSamples(adcBuffer, 3) / BATTERY_INPUT_DIVIDER,
            }
        },
        .VirtualGroundVoltageInt = AverageSamples(adcBuffer, 2),
    };
}

Configuration GetConfiguration()
{
    // TODO: implement me!
    return {};
}

void SetConfiguration(const Configuration& newConfig)
{
    // TODO: implement me!
}
