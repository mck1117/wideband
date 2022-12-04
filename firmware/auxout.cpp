#include "pwm.h"
#include "lambda_conversion.h"
#include "port.h"
#include "io_pins.h"

#include "wideband_config.h"

#include "max31855.h"

#include "hal.h"

#include <rusefi/math.h>
#include <rusefi/interpolation.h>

#ifdef AUXOUT_DAC_PWM_DEVICE

// Rev2 low pass filter cut frequency is about 21Hz (sic!)
// 48Mhz / (2 ^ 12) ~= 12 KHz
// 64mhz / (2 ^ 12) ~= 16 KHz
static const PWMConfig auxPwmConfig = {
    STM32_SYSCLK,
    1 << 12,
    nullptr,
    {
        {PWM_OUTPUT_ACTIVE_HIGH | PWM_COMPLEMENTARY_OUTPUT_ACTIVE_HIGH, nullptr},
        {PWM_OUTPUT_ACTIVE_HIGH | PWM_COMPLEMENTARY_OUTPUT_ACTIVE_HIGH, nullptr},
        {PWM_OUTPUT_ACTIVE_HIGH | PWM_COMPLEMENTARY_OUTPUT_ACTIVE_HIGH, nullptr},
        {PWM_OUTPUT_ACTIVE_HIGH | PWM_COMPLEMENTARY_OUTPUT_ACTIVE_HIGH, nullptr}
    },
    0,
    0,
#if STM32_PWM_USE_ADVANCED
    0
#endif
};

static Pwm auxDac(AUXOUT_DAC_PWM_DEVICE);

static const uint8_t auxOutPwmCh[] = {
    AUXOUT_DAC_PWM_CHANNEL_0,
#if (AFR_CHANNELS > 1)
    AUXOUT_DAC_PWM_CHANNEL_1,
#endif
};

void SetAuxDac(int channel, float voltage)
{
    voltage = voltage / AUXOUT_GAIN;
    auto duty = voltage / VCC_VOLTS;
    duty = 1.0 - duty;
    duty = clampF(0, duty, 1);

    auxDac.SetDuty(auxOutPwmCh[channel], duty);
}

#endif

#ifdef AUXOUT_DAC_DEVICE

class Dac
{
public:
    Dac(DACDriver& driver);

    void Start(DACConfig& config);
    void SetVoltage(int channel, float duty);
    float GetLastVoltage(int channel);

private:
    DACDriver* const m_driver;
    float m_voltageFloat[2];
};

Dac::Dac(DACDriver& driver)
    : m_driver(&driver)
{
}

void Dac::Start(DACConfig& config)
{
    dacStart(m_driver, &config);
}

void Dac::SetVoltage(int channel, float voltage) {
    voltage = clampF(0, voltage, VCC_VOLTS);
    m_voltageFloat[channel] = voltage;

    dacPutChannelX(m_driver, channel, voltage / VCC_VOLTS * (1 << 12));
}

float Dac::GetLastVoltage(int channel)
{
    return m_voltageFloat[channel];
}

static DACConfig auxDacConfig = {
  .init         = 2047U,
  .datamode     = DAC_DHRM_12BIT_RIGHT,
  .cr           = 0
};

static Dac auxDac(AUXOUT_DAC_DEVICE);

static const uint8_t auxOutDacCh[] = {
    AUXOUT_DAC_CHANNEL_0,
#if (AFR_CHANNELS > 1)
    AUXOUT_DAC_CHANNEL_1,
#endif
};

void SetAuxDac(int channel, float voltage)
{
    voltage = voltage / AUXOUT_GAIN;

    auxDac.SetVoltage(auxOutDacCh[channel], voltage);
}

#endif /* AUXOUT_DAC_DEVICE */

#if (defined(AUXOUT_DAC_PWM_DEVICE) || defined(AUXOUT_DAC_DEVICE))

static float AuxGetInputSignal(int sel)
{
    switch (sel)
    {
        case 0:
        case 1:
            return GetLambda(sel);
#if HAL_USE_SPI
        case 2:
        case 3:
            return getEgtDrivers()[sel - 2].temperature;
#endif
        default:
            return 0.0;

    }
    return 0.0;
}

/* TODO: merge with some other communication thread? */
static THD_WORKING_AREA(waAuxOutThread, 256);
void AuxOutThread(void*)
{
    const auto cfg = GetConfiguration();

    while(1)
    {
        for (int ch = 0; ch < AFR_CHANNELS; ch++)
        {
            auto cfg = GetConfiguration();
            float input = AuxGetInputSignal(cfg->auxInput[ch]);
            float voltage = interpolate2d(input, cfg->auxOutBins[ch], cfg->auxOutValues[ch]);

            SetAuxDac(ch, voltage);
        }

        chThdSleepMilliseconds(10);
    }
}

void InitAuxDac()
{
#if defined(AUXOUT_DAC_PWM_DEVICE)
    auxDac.Start(auxPwmConfig);

    SetAuxDac(0, 0.0);
    SetAuxDac(1, 0.0);
#endif
#if defined(AUXOUT_DAC_DEVICE)
    auxDac.Start(auxDacConfig);

    SetAuxDac(0, 0.0);
#endif

    chThdCreateStatic(waAuxOutThread, sizeof(waAuxOutThread), NORMALPRIO, AuxOutThread, nullptr);
}

#else /* (AUXOUT_DAC_PWM_DEVICE || AUXOUT_DAC_DEVICE) */

void InitAuxDac()
{
}

#endif
