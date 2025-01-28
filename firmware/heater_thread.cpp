#include "ch.h"
#include "hal.h"
#include "pwm.h"

#include "heater_control.h"
#include "port.h"
#include "sampling.h"

// 400khz / 1024 = 390hz PWM
static Pwm heaterPwm(HEATER_PWM_DEVICE);
static const PWMConfig heaterPwmConfig = {
    .frequency = 400'000,
    .period = 1024,
    .callback = nullptr,
    .channels = {
        {PWM_OUTPUT_ACTIVE_HIGH | PWM_COMPLEMENTARY_OUTPUT_ACTIVE_LOW, nullptr},
        {PWM_OUTPUT_ACTIVE_HIGH | PWM_COMPLEMENTARY_OUTPUT_ACTIVE_LOW, nullptr},
        {PWM_OUTPUT_ACTIVE_HIGH | PWM_COMPLEMENTARY_OUTPUT_ACTIVE_LOW, nullptr},
        {PWM_OUTPUT_ACTIVE_HIGH | PWM_COMPLEMENTARY_OUTPUT_ACTIVE_LOW, nullptr}
    },
    .cr2 = 0,
#if STM32_PWM_USE_ADVANCED
    .bdtr = 0,
#endif
    .dier = 0
};

class HeaterController : public HeaterControllerBase {
public:
    HeaterController(int ch, int pwm_ch)
        : HeaterControllerBase(ch, HEATER_PREHEAT_TIME, HEATER_WARMUP_TIMEOUT)
        , pwm_ch(pwm_ch)
    {
    }

    void SetDuty(float duty) const override
    {
        heaterPwm.SetDuty(pwm_ch, duty);
    }

// TODO: private:
public:
    const uint8_t pwm_ch;
};

HeaterController heaterControllers[AFR_CHANNELS] =
{
    { 0, HEATER_PWM_CHANNEL_0 },

#if AFR_CHANNELS >= 2
    { 1, HEATER_PWM_CHANNEL_1 },
#endif

#if AFR_CHANNELS >= 3
    { 2, HEATER_PWM_CHANNEL_2 },
#endif

#if AFR_CHANNELS >= 4
    { 3, HEATER_PWM_CHANNEL_3 },
#endif
};

const IHeaterController& GetHeaterController(int ch)
{
    return heaterControllers[ch];
}

static THD_WORKING_AREA(waHeaterThread, 256);
static void HeaterThread(void*)
{
    chRegSetThreadName("Heater");

    // Wait for temperature sensing to stabilize so we don't
    // immediately think we overshot the target temperature
    chThdSleepMilliseconds(1000);

    // Configure heater controllers for sensor type
    for (int i = 0; i < AFR_CHANNELS; i++)
    {
        auto& h = heaterControllers[i];
        switch (GetSensorType())
        {
            case SensorType::LSU42:
                h.Configure(730, 80);
                break;
            case SensorType::LSUADV:
                h.Configure(785, 300);
                break;
            case SensorType::LSU49:
            default:
                h.Configure(780, 300);
                break;
        }
    }

    while (true)
    {
        auto heaterAllowState = GetHeaterAllowed();

        for (int i = 0; i < AFR_CHANNELS; i++)
        {
            const auto& sampler = GetSampler(i);
            auto& heater = heaterControllers[i];

            heater.Update(sampler, heaterAllowState);
        }

        // Loop at ~20hz
        chThdSleepMilliseconds(HEATER_CONTROL_PERIOD);
    }
}

void StartHeaterControl()
{
    heaterPwm.Start(heaterPwmConfig);

    for (int i = 0; i < AFR_CHANNELS; i++)
    {
        heaterPwm.SetDuty(heaterControllers[i].pwm_ch, 0);
    }

    chThdCreateStatic(waHeaterThread, sizeof(waHeaterThread), NORMALPRIO + 1, HeaterThread, nullptr);
}

float GetHeaterDuty(int ch)
{
    return heaterPwm.GetLastDuty(heaterControllers[ch].pwm_ch);
}

HeaterState GetHeaterState(int ch)
{
    return heaterControllers[ch].GetHeaterState();
}
