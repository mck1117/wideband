#include "port.h"


#include "wideband_config.h"

#include "hal.h"

void PortPrepareAnalogSampling()
{
    adcStart(&ADCD1, nullptr);
}

#define ADC_CHANNEL_COUNT 8
#define ADC_SAMPLE ADC_SAMPLE_7P5

static adcsample_t adcBuffer[ADC_CHANNEL_COUNT * ADC_OVERSAMPLE];

const ADCConversionGroup convGroup =
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

static float GetMaxSample(adcsample_t* buffer, size_t idx)
{
    adcsample_t max = 0;

    for (size_t i = 0; i < ADC_OVERSAMPLE; i++)
    {
        if (buffer[idx] > max)
        {
            max = buffer[idx];
        }

        idx += ADC_CHANNEL_COUNT;
    }

    constexpr float scale = VCC_VOLTS / ADC_MAX_COUNT;

    return (float)max * scale;
}

static float l_heater_voltage = 0;
static float r_heater_voltage = 0;

AnalogResult AnalogSample()
{
    bool l_heater = !palReadPad(L_HEATER_PORT, L_HEATER_PIN);
    bool r_heater = !palReadPad(R_HEATER_PORT, R_HEATER_PIN);

    adcConvert(&ADCD1, &convGroup, adcBuffer, ADC_OVERSAMPLE);

    bool l_heater_new = !palReadPad(L_HEATER_PORT, L_HEATER_PIN);
    bool r_heater_new = !palReadPad(R_HEATER_PORT, R_HEATER_PIN);

    if (l_heater && l_heater_new)
    {
        float vbatt_raw = GetMaxSample(adcBuffer, 6) / HEATER_INPUT_DIVIDER;
        l_heater_voltage = HEATER_FILTER_ALPHA * vbatt_raw + (1.0 - HEATER_FILTER_ALPHA) * l_heater_voltage;
    }

    if (r_heater && r_heater_new)
    {
        float vbatt_raw = GetMaxSample(adcBuffer, 7) / HEATER_INPUT_DIVIDER;
        r_heater_voltage = HEATER_FILTER_ALPHA * vbatt_raw + (1.0 - HEATER_FILTER_ALPHA) * r_heater_voltage;
    }

    return
    {
        .ch = {
            {
                /* left */
                .NernstVoltage = AverageSamples(adcBuffer, 3) * (1.0 / NERNST_INPUT_GAIN),
                .PumpCurrentVoltage = AverageSamples(adcBuffer, 2),
                .HeaterSupplyVoltage = l_heater_voltage,
            },
            {
                /* right */
                .NernstVoltage = AverageSamples(adcBuffer, 1) * (1.0 / NERNST_INPUT_GAIN),
                .PumpCurrentVoltage = AverageSamples(adcBuffer, 0),
                .HeaterSupplyVoltage = r_heater_voltage,
            },
        },
        /* Dual board has separate internal virtual ground = 3.3V / 2
         * VirtualGroundVoltageInt is used to calculate Ip current only as it
         * is used as offset for diffirential amp */
        .VirtualGroundVoltageInt = HALF_VCC,
    };
}

/* TODO: optimize */
void SetupESRDriver(SensorType sensor)
{
    switch (sensor) {
        case SensorType::LSU42:
            /* disable bias */
            palSetPadMode(NERNST_49_BIAS_PORT, NERNST_49_BIAS_PIN,
                PAL_MODE_INPUT);
            /* disable all others ESR drivers */
            palSetPadMode(NERNST_49_ESR_DRIVER_PORT, NERNST_49_ESR_DRIVER_PIN,
                PAL_MODE_INPUT);
            palSetPadMode(NERNST_ADV_ESR_DRIVER_PORT, NERNST_ADV_ESR_DRIVER_PIN,
                PAL_MODE_INPUT);
            /* enable LSU4.2 */
            palSetPadMode(NERNST_42_ESR_DRIVER_PORT, NERNST_42_ESR_DRIVER_PIN,
                PAL_MODE_OUTPUT_PUSHPULL);
        break;
        case SensorType::LSU49:
            /* disable all others ESR drivers */
            palSetPadMode(NERNST_42_ESR_DRIVER_PORT, NERNST_42_ESR_DRIVER_PIN,
                PAL_MODE_INPUT);
            palSetPadMode(NERNST_ADV_ESR_DRIVER_PORT, NERNST_ADV_ESR_DRIVER_PIN,
                PAL_MODE_INPUT);
            /* enable LSU4.2 */
            palSetPadMode(NERNST_49_ESR_DRIVER_PORT, NERNST_49_ESR_DRIVER_PIN,
                PAL_MODE_OUTPUT_PUSHPULL);
            /* enable bias */
            palSetPadMode(NERNST_49_BIAS_PORT, NERNST_49_BIAS_PIN,
                PAL_MODE_OUTPUT_PUSHPULL);
            palSetPad(NERNST_49_BIAS_PORT, NERNST_49_BIAS_PIN);
        break;
        case SensorType::LSUADV:
            /* disable bias */
            palSetPadMode(NERNST_49_BIAS_PORT, NERNST_49_BIAS_PIN,
                PAL_MODE_INPUT);
            /* disable all others ESR drivers */
            palSetPadMode(NERNST_49_ESR_DRIVER_PORT, NERNST_49_ESR_DRIVER_PIN,
                PAL_MODE_INPUT);
            palSetPadMode(NERNST_42_ESR_DRIVER_PORT, NERNST_42_ESR_DRIVER_PIN,
                PAL_MODE_INPUT);
            /* enable LSU4.2 */
            palSetPadMode(NERNST_ADV_ESR_DRIVER_PORT, NERNST_ADV_ESR_DRIVER_PIN,
                PAL_MODE_OUTPUT_PUSHPULL);
        break;
    }
}

int GetESRSupplyR()
{
    switch (GetSensorType()) {
        case SensorType::LSU42:
            return 6800;
        case SensorType::LSU49:
            return 22000;
        case SensorType::LSUADV:
            return 47000;
    }
    return 0;
}
