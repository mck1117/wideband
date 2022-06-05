#include "port.h"


#include "wideband_config.h"

#include "hal.h"

#define ADC_CHANNEL_COUNT 8
#define ADC_SAMPLE ADC_SAMPLE_7P5

static adcsample_t adcBuffer[ADC_CHANNEL_COUNT * ADC_OVERSAMPLE];

ADCConversionGroup convGroup =
{
    .circular = false,
    .num_channels = ADC_CHANNEL_COUNT,
    .end_cb = nullptr,
    .error_cb = nullptr,
    .cr1 = 0,
    .cr2 =
        ADC_CR2_CONT |
        ADC_CR2_ADON,   /* keep ADC enabled between convertions - for GD32 */
    .smpr1 = 0,
    .smpr2 =
        ADC_SMPR2_SMP_AN0(ADC_SAMPLE) |
        ADC_SMPR2_SMP_AN3(ADC_SAMPLE) |
        ADC_SMPR2_SMP_AN4(ADC_SAMPLE) |
        ADC_SMPR2_SMP_AN5(ADC_SAMPLE) |
        ADC_SMPR2_SMP_AN6(ADC_SAMPLE) |
        ADC_SMPR2_SMP_AN7(ADC_SAMPLE) |
        ADC_SMPR2_SMP_AN8(ADC_SAMPLE) |
        ADC_SMPR2_SMP_AN9(ADC_SAMPLE),
    .sqr1 = ADC_SQR1_NUM_CH(ADC_CHANNEL_COUNT),
    .sqr2 =
        /* TODO: move these two channels to slow ADC! */
        ADC_SQR2_SQ7_N(8) | /* PB0 - ADC12_IN8 - L_Heater_sense */
        ADC_SQR2_SQ8_N(9),  /* PB1 - ADC12_IN9 - R_Heater_sense */
    .sqr3 =
        ADC_SQR3_SQ1_N(0) | /* PA0 - ADC12_IN0 - R_Ip_sense */
        ADC_SQR3_SQ2_N(3) | /* PA3 - ADC12_IN3 - R_Un_3x_sense */
        ADC_SQR3_SQ3_N(4) | /* PA4 - ADC12_IN4 - L_Ip_sense */
        ADC_SQR3_SQ4_N(5) | /* PA5 - ADC12_IN5 - L_Un_3x_sense */
        /* TODO: move these two channels to slow ADC! */
        ADC_SQR3_SQ5_N(6) | /* PA6 - ADC12_IN6 - R_AUX_ADC */
        ADC_SQR3_SQ6_N(7),  /* PA7 - ADC12_IN7 - L_AUX_ADC */
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

static float l_vbatt = 0.0, r_vbatt = 0.0;

AnalogResult AnalogSample()
{
    bool l_heater = !palReadPad(L_HEATER_PORT, L_HEATER_PIN);
    bool r_heater = !palReadPad(R_HEATER_PORT, R_HEATER_PIN);

    adcConvert(&ADCD1, &convGroup, adcBuffer, ADC_OVERSAMPLE);

    if ((l_heater) && (!palReadPad(L_HEATER_PORT, L_HEATER_PIN)))
        l_vbatt = AverageSamples(adcBuffer, 6) / BATTERY_INPUT_DIVIDER;
    if ((r_heater) && (!palReadPad(R_HEATER_PORT, R_HEATER_PIN)))
        r_vbatt = AverageSamples(adcBuffer, 7) / BATTERY_INPUT_DIVIDER;

    return
    {
        .ch = {
            {
                /* left */
                .NernstVoltage = AverageSamples(adcBuffer, 3) * NERNST_INPUT_GAIN,
                .PumpCurrentVoltage = AverageSamples(adcBuffer, 2),
                .BatteryVoltage = l_vbatt,
            },
            {
                /* right */
                .NernstVoltage = AverageSamples(adcBuffer, 1) * NERNST_INPUT_GAIN,
                .PumpCurrentVoltage = AverageSamples(adcBuffer, 0),
                .BatteryVoltage = r_vbatt,
            },
        },
        /* Dual board has separate internal virtual ground = 3.3V / 2
         * VirtualGroundVoltageInt is used to calculate Ip current only as it
         * is used as offset for diffirential amp */
        .VirtualGroundVoltageInt = HALF_VCC,
    };
}

static struct {
    uint8_t        pad[128];
} config;

uint8_t *GetConfiguratiuonPtr()
{
    return (uint8_t *)&config;
}

int GetConfiguratiuonSize()
{
    return sizeof(config);
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

/* TS stuff */

void SaveConfiguration() {

}

#define TS_SIGNATURE "rusEFI 2022.05.29.wideband_rev2"

const char *getTsSignature() {
    return TS_SIGNATURE;
}