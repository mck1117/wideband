#include "ch.h"
#include "hal.h"

#include "io_pins.h"
#include "livedata.h"

#include "sampling.h"
#include "port.h"
#include "analog.h"

static BoardSampler samplers[AFR_CHANNELS];

const BoardSampler& GetSampler(int ch)
{
    return samplers[ch];
}

static THD_WORKING_AREA(waSamplingThread, 256);

#ifdef ANALOG_PWM_DEVICE
// Update analog output once every N samples (6khz sample rate / 60 = 100hz)
#define ANALOG_OUT_DECIMATION 60
#endif

#ifdef SAMPLE_RATE_HZ
static_assert(CH_CFG_ST_FREQUENCY % SAMPLE_RATE_HZ == 0, "sample period must be an integer number of ticks");
static constexpr sysinterval_t samplePeriod = CH_CFG_ST_FREQUENCY / SAMPLE_RATE_HZ;
static_assert(samplePeriod > CH_CFG_ST_TIMEDELTA, "sample period too short for tickless timer");
#endif

#ifdef BOARD_HAS_VOLTAGE_SENSE
static float supplyVoltage = 0;

float GetSupplyVoltage()
{
    return supplyVoltage;
}
#endif

static float mcuTemp = 0;
float GetMcuTemperature()
{
    return mcuTemp;
}

static void SamplingThread(void*)
{
    chRegSetThreadName("Sampling");

    SetupESRDriver(GetSensorType());

    /* GD32: Insert 20us delay after ADC enable */
    chThdSleepMilliseconds(1);

    AnalogSampleStart();

#ifdef SAMPLE_RATE_HZ
    systime_t deadline = chVTGetSystemTimeX();
#endif

    while (true)
    {
#ifdef SAMPLE_RATE_HZ
        // Run the loop at a fixed rate. The conversion started last loop completes during this sleep.
        // If we're running late, this returns immediately.
        deadline = chThdSleepUntilWindowed(deadline, chTimeAddX(deadline, samplePeriod));
#endif

        auto result = AnalogSampleFinish();

        // Toggle the pin after sampling so that any switching noise occurs while we're doing our math instead of when
        // sampling
        ToggleESRDriver(GetSensorType());

        AnalogSampleStart();

#ifdef BOARD_HAS_VOLTAGE_SENSE
        supplyVoltage = result.SupplyVoltage;
#endif
        mcuTemp = result.McuTemp;

        for (int ch = 0; ch < AFR_CHANNELS; ch++)
        {
            samplers[ch].ApplySample(result.ch[ch], result.VirtualGroundVoltageInt);
        }

#ifdef ANALOG_PWM_DEVICE
        // Analog output is RC filtered, so it doesn't need updating every sample.
        // Lambda conversion + PWM update is too expensive to run per-sample on M0.
        static uint8_t analogOutDecimate = 0;
        if (++analogOutDecimate >= ANALOG_OUT_DECIMATION)
        {
            analogOutDecimate = 0;

            for (int ch = 0; ch < AFR_CHANNELS; ch++)
            {
                setAnalogOut(ch, samplers[ch].GetLambda());
            }
        }
#endif // ANALOG_PWM_DEVICE

#if defined(TS_ENABLED)
        /* tunerstudio */
        SamplingUpdateLiveData();
#endif
    }
}

void StartSampling()
{
    for (int i = 0; i < AFR_CHANNELS; i++)
    {
        samplers[i].Init();
    }

    PortPrepareAnalogSampling();

    chThdCreateStatic(waSamplingThread, sizeof(waSamplingThread), NORMALPRIO + 5, SamplingThread, nullptr);
}
