#include "pwm.h"
#include "lambda_conversion.h"

#include "wideband_config.h"

#include "hal.h"

#include <rusefi/math.h>

#ifdef AUXOUT_DAC_PWM_DEVICE

// Rev2 low pass filter cut frequency is about 21Hz (sic!)
// 48Mhz / (2 ^ 12) ~= 12 KHz
PWMConfig auxPwmConfig = {
    48'000'000,
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

/* TODO: merge with some other communication thread? */
static THD_WORKING_AREA(waAuxOutThread, 256);
void AuxOutThread(void*)
{
    while(1)
    {
#if defined(AUXOUT_DAC_PWM_DEVICE)
        for (int ch = 0; ch < AFR_CHANNELS; ch++)
        {
            SetAuxDac(ch, GetLambda(ch));
        }

        chThdSleepMilliseconds(10);
#else
    SetAuxDac(0, 0.0);
    chThdSleepMilliseconds(2000);

    SetAuxDac(0, 1.0);
    chThdSleepMilliseconds(2000);

    SetAuxDac(0, 2.0);
    chThdSleepMilliseconds(2000);

    SetAuxDac(0, 3.0);
    chThdSleepMilliseconds(2000);
#endif
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

#if (defined(AUXOUT_DAC_PWM_DEVICE) || defined(AUXOUT_DAC_DEVICE))
    chThdCreateStatic(waAuxOutThread, sizeof(waAuxOutThread), NORMALPRIO, AuxOutThread, nullptr);
#endif /* AUXOUT_DAC_PWM_DEVICE) defined(AUXOUT_DAC_DEVICE */
}
