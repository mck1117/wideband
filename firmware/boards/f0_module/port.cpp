#include "port.h"
#include "shared/flash.h"

#include "wideband_config.h"

#include "hal.h"

#define ADC_CHANNEL_COUNT 3

static adcsample_t adcBuffer[ADC_CHANNEL_COUNT * ADC_OVERSAMPLE];

ADCConversionGroup convGroup =
{
    false,
    ADC_CHANNEL_COUNT,
    nullptr,
    nullptr,
    ADC_CFGR1_CONT | ADC_CFGR1_RES_12BIT,                  // CFGR1
    ADC_TR(0, 0),       // TR
    ADC_SMPR_SMP_7P5,      // SMPR
    ADC_CHSELR_CHSEL0 | ADC_CHSELR_CHSEL2 | ADC_CHSELR_CHSEL3
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
        .NernstVoltage = AverageSamples(adcBuffer, 0) * NERNST_INPUT_GAIN,
        .PumpCurrentVoltage = AverageSamples(adcBuffer, 1),
        .VirtualGroundVoltageInt = AverageSamples(adcBuffer, 2),
    };
}

extern Configuration __configflash__start__;

Configuration GetConfiguration()
{
    const auto& cfg = __configflash__start__;

    // If config has been written before, return the stored configuration
    if (cfg.IsValid())
    {
        return cfg;
    }
    else
    {
        // Otherwise return a default config
        return {};
    }
}

void SetConfiguration(const Configuration& newConfig)
{
    // erase config page
    Flash::ErasePage(31);

    // Copy data to flash
    Flash::Write(
        reinterpret_cast<flashaddr_t>(&__configflash__start__),
        reinterpret_cast<const uint8_t*>(&newConfig),
        sizeof(Configuration)
    );
}

const CANConfig canConfig500 =
{
    CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP,
    CAN_BTR_SJW(0) | CAN_BTR_BRP(5)  | CAN_BTR_TS1(12) | CAN_BTR_TS2(1),
};
