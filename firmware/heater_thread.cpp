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
    .channels = {{PWM_OUTPUT_ACTIVE_HIGH | PWM_COMPLEMENTARY_OUTPUT_ACTIVE_LOW, nullptr},
                 {PWM_OUTPUT_ACTIVE_HIGH | PWM_COMPLEMENTARY_OUTPUT_ACTIVE_LOW, nullptr},
                 {PWM_OUTPUT_ACTIVE_HIGH | PWM_COMPLEMENTARY_OUTPUT_ACTIVE_LOW, nullptr},
                 {PWM_OUTPUT_ACTIVE_HIGH | PWM_COMPLEMENTARY_OUTPUT_ACTIVE_LOW, nullptr}},
    .cr2 = 0,
#if STM32_PWM_USE_ADVANCED
    .bdtr = 0,
#endif
    .dier = 0};

HeaterController heaterControllers[AFR_CHANNELS] = {
    {0, HEATER_PWM_CHANNEL_0},

#if AFR_CHANNELS >= 2
    {1, HEATER_PWM_CHANNEL_1},
#endif

#if AFR_CHANNELS >= 3
    {2, HEATER_PWM_CHANNEL_2},
#endif

#if AFR_CHANNELS >= 4
    {3, HEATER_PWM_CHANNEL_3},
#endif
};

const HeaterController& GetHeaterController(int ch)
{
    return heaterControllers[ch];
}

/*
 * Keep the ChibiOS 64 bit timestamp (chVTGetTimeStamp, used by Timer) current.
 *
 * The timestamp is extended in software from the system timer counter (16 bits on some boards),
 * and ONLY when it's read: if nobody reads it for a full counter wrap, that wrap is silently lost and
 * every Timer reads short. Nothing in the kernel refreshes it for us - sleeps, timeouts and the timer
 * interrupt all use the wrapping system time directly - and our other Timer users only read it on some
 * code paths. So this thread deliberately reads it every HEATER_CONTROL_PERIOD, in every state.
 *
 * eg. a 16 bit counter at 60khz wraps every 1.09 seconds.
 *
 * noinline: chVTGetTimeStamp() inlines a lock, debug checks and 64 bit math (~350 bytes per call site),
 * and this isn't hot, so keep one copy.
 */
__attribute__((noinline)) static void RefreshTimestamp()
{
    (void)chVTGetTimeStamp();
}

// Refresh well within one counter wrap
static_assert((uint64_t)HEATER_CONTROL_PERIOD * CH_CFG_ST_FREQUENCY / 1000 < (1ull << CH_CFG_ST_RESOLUTION) / 4,
              "Heater thread doesn't run often enough to keep the system timestamp current");

static THD_WORKING_AREA(waHeaterThread, 256);
static void HeaterThread(void*)
{
    chRegSetThreadName("Heater");

    // Wait for temperature sensing to stabilize so we don't
    // immediately think we overshot the target temperature.
    // Sleep in short chunks so we keep the timestamp current meanwhile (see RefreshTimestamp)
    for (int i = 0; i < 1000 / HEATER_CONTROL_PERIOD; i++)
    {
        RefreshTimestamp();
        chThdSleepMilliseconds(HEATER_CONTROL_PERIOD);
    }

    // Configure heater controllers for sensor type
    for (int i = 0; i < AFR_CHANNELS; i++)
    {
        auto& h = heaterControllers[i];
        switch (GetSensorType())
        {
        case SensorType::LSU42: h.Configure(730, 80); break;
        case SensorType::LSUADV: h.Configure(785, 300); break;
        case SensorType::LSU49:
        default: h.Configure(780, 300); break;
        }
    }

    while (true)
    {
        // Every loop, regardless of heater states
        RefreshTimestamp();

        auto heaterAllowState = GetHeaterAllowed();

        for (int i = 0; i < AFR_CHANNELS; i++)
        {
            const auto& sampler = GetSampler(i);
            auto& heater = heaterControllers[i];

            float duty = heater.Update(sampler, heaterAllowState);
            heaterPwm.SetDuty(heater.pwm_ch, duty);
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
