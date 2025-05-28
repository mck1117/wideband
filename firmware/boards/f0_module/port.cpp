#include "port.h"
#include "shared/flash.h"

#include "wideband_config.h"

#include "hal.h"

#define ADC_CHANNEL_COUNT 3

void PortPrepareAnalogSampling()
{
    adcStart(&ADCD1, nullptr);
}

static adcsample_t adcBuffer[ADC_CHANNEL_COUNT * ADC_OVERSAMPLE];

const ADCConversionGroup convGroup =
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
        .ch =
        {
            {
                .NernstVoltage = AverageSamples(adcBuffer, 0) * (1.0 / NERNST_INPUT_GAIN),
                .PumpCurrentVoltage = AverageSamples(adcBuffer, 1),
                .HeaterSupplyVoltage = 0,
            },
        },
        .VirtualGroundVoltageInt = AverageSamples(adcBuffer, 2),

        // TODO!
        .McuTemp = 0,
    };
}

// Returns:
// low -> 0
// floating -> 1
// high -> 2
static uint8_t readSelPin(ioportid_t port, iopadid_t pad)
{
    // If we pull the pin down, does the input follow?
    palSetPadMode(port, pad, PAL_MODE_INPUT_PULLDOWN);
    chThdSleepMilliseconds(1);
    auto pd = palReadPad(port, pad);

    // If we pull the pin up, does the input follow?
    palSetPadMode(port, pad, PAL_MODE_INPUT_PULLUP);
    chThdSleepMilliseconds(1);
    auto pu = palReadPad(port, pad);

    // If the pin changed with pullup/down state, it's floating
    if (pd != pu)
    {
        return 1;
    }

    if (pu)
    {
        // Pin was high
        return 2;
    }
    else
    {
        // Pin was low
        return 0;
    }
}

extern Configuration __configflash__start__;

int InitConfiguration()
{
    return 0;
}

static Configuration config;

Configuration* GetConfiguration()
{
    const auto& cfg = __configflash__start__;

    // If config has been written before, use the stored configuration
    if (cfg.IsValid())
    {
        config = cfg;
    }

    // Now, override the index with a hardware-strapped option (if present)
    auto sel1 = readSelPin(ID_SEL1_PORT, ID_SEL1_PIN);
    auto sel2 = readSelPin(ID_SEL2_PORT, ID_SEL2_PIN);

    // See https://github.com/mck1117/wideband/issues/11 to explain this madness
    switch (3 * sel1 + sel2) {
        case 0: config.afr[0].RusEfiIdOffset = 2; break;
        case 1: config.afr[0].RusEfiIdOffset = 0; break;
        case 2: config.afr[0].RusEfiIdOffset = 3; break;
        case 3: config.afr[0].RusEfiIdOffset = 4; break;
        case 4: /* both floating, do nothing */ break;
        case 5: config.afr[0].RusEfiIdOffset = 1; break;
        case 6: config.afr[0].RusEfiIdOffset = 5; break;
        case 7: config.afr[0].RusEfiIdOffset = 6; break;
        case 8: config.afr[0].RusEfiIdOffset = 7; break;
        default: break;
    }

    return &config;
}

void SetConfiguration()
{
    // erase config page
    Flash::ErasePage(31);

    // Copy data to flash
    Flash::Write(
        reinterpret_cast<flashaddr_t>(&__configflash__start__),
        reinterpret_cast<const uint8_t*>(&config),
        sizeof(Configuration)
    );
}

SensorType GetSensorType()
{
    return SensorType::LSU49;
}

void SetupESRDriver(SensorType)
{
    // NOP
}

int GetESRSupplyR()
{
    // Nernst AC injection resistor value
    return 22000;
}

void ToggleESRDriver(SensorType)
{
    palTogglePad(NERNST_49_ESR_DRIVER_PORT, NERNST_49_ESR_DRIVER_PIN);
}
