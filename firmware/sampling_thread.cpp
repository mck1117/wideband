#include "ch.h"
#include "hal.h"

#include "io_pins.h"
#include "livedata.h"

#include "sampling.h"
#include "port.h"

static Sampler samplers[AFR_CHANNELS];

const ISampler& GetSampler(int ch)
{
    return samplers[ch];
}

static THD_WORKING_AREA(waSamplingThread, 256);

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

    while(true)
    {
        auto result = AnalogSampleFinish();

        // Toggle the pin after sampling so that any switching noise occurs while we're doing our math instead of when sampling
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

#if defined(TS_ENABLED) && TS_ENABLED != 0
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
