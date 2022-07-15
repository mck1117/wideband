#include "port.h"


#include "wideband_config.h"

#include "hal.h"

#define ADC_CHANNEL_COUNT 5
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
        /* PA5 - ADC12_IN5 - Un_sense - no used */
        ADC_SMPR2_SMP_AN6(ADC_SAMPLE) |
        ADC_SMPR2_SMP_AN7(ADC_SAMPLE) |
        ADC_SMPR2_SMP_AN8(ADC_SAMPLE) |
        ADC_SMPR2_SMP_AN9(ADC_SAMPLE),
    .sqr1 = ADC_SQR1_NUM_CH(ADC_CHANNEL_COUNT),
    .sqr2 = 0,
    .sqr3 =
        ADC_SQR3_SQ1_N(0) | /* PA0 - ADC12_IN0 - Vm_sense */
        /* PA5 - ADC12_IN5 - Un_sense - no used */
        ADC_SQR3_SQ2_N(6) | /* PA6 - ADC12_IN6 - Ip_sense */
        ADC_SQR3_SQ3_N(7) | /* PA7 - ADC12_IN7 - Un_3x_sense */
        ADC_SQR3_SQ4_N(8) | /* PB0 - ADC12_IN8 - Vbatt_sense */
        ADC_SQR3_SQ5_N(9)   /* PB1 - ADC12_IN9 - Heater_sense */
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
        .ch = {
            {
                .NernstVoltage = AverageSamples(adcBuffer, 2) * NERNST_INPUT_GAIN,
                .PumpCurrentVoltage = AverageSamples(adcBuffer, 1),
                /* We also can measure output virtual ground voltage for diagnostic purposes */
                //.VirtualGroundVoltageExt = AverageSamples(adcBuffer, 0) / VM_INPUT_DIVIDER,
                .BatteryVoltage = AverageSamples(adcBuffer, 3) / BATTERY_INPUT_DIVIDER,
                /* .HeaterVoltage = AverageSamples(adcBuffer, 4) / HEATER_INPUT_DIVIDER, */
            },
        },
        /* Rev 2 board has separate internal virtual ground = 3.3V / 2
         * VirtualGroundVoltageInt is used to calculate Ip current only as it
         * is used as offset for diffirential amp */
        .VirtualGroundVoltageInt = HALF_VCC,
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

const char *getTsSignature() {
    return BOARD_NAME;
}

SensorType GetSensorType()
{
    /* TODO: load from settings */
#if defined(BOARD_SENSOR_LSU42)
    return SENSOR_TYPE_LSU42;
#elif defined(BOARD_SENSOR_LSUADV)
    return SENSOT_TYPE_LSUADV;
#else
    /* default is LSU4.9 */
    return SENSOR_TYPE_LSU49;
#endif
}

/* TODO: optimize */
void SetupESRDriver(SensorType sensor)
{
    switch (sensor) {
        case SENSOR_TYPE_LSU42:
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
        case SENSOR_TYPE_LSU49:
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
        case SENSOR_TYPE_LSUADV:
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
        case SENSOR_TYPE_LSU42:
            return 6800;
        case SENSOR_TYPE_LSU49:
            return 22000;
        case SENSOR_TYPE_LSUADV:
            return 47000;
    }
    return 0;
}
