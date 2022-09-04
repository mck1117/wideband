#include "wideband_config.h"
#include "livedata.h"

#include "lambda_conversion.h"
#include "sampling.h"
#include "pump_dac.h"
#include "heater_control.h"
#include "max31855.h"
#include "fault.h"

#include <rusefi/arrays.h>
#include <rusefi/fragments.h>

static livedata_common_s livedata_common;
static livedata_afr_s livedata_afr[AFR_CHANNELS];

void SamplingUpdateLiveData()
{
    for (int ch = 0; ch < AFR_CHANNELS; ch++)
    {
        volatile struct livedata_afr_s *data = &livedata_afr[ch];

        data->afr = GetLambda(ch);
        data->temperature = GetSensorTemperature(ch);
        data->nernstVoltage = GetNernstDc(ch);
        data->pumpCurrentTarget = GetPumpCurrent(ch);
        data->pumpCurrentMeasured = GetPumpNominalCurrent(ch);
        data->heaterDuty = GetHeaterDuty(ch);
        data->fault = (uint8_t)GetCurrentFault(ch);
        data->heaterState = (uint8_t)GetHeaterState(ch);
    }

    livedata_common.vbatt = GetInternalBatteryVoltage(0);
}

const livedata_common_s * getCommonLiveDataStructAddr()
{
    return &livedata_common;
}

const struct livedata_afr_s * getAfrLiveDataStructAddr(const int ch)
{
    if (ch < AFR_CHANNELS)
        return &livedata_afr[ch];
    return NULL;
}

static const FragmentEntry fragments[] = {
    getCommonLiveDataStructAddr(),
    getAfrLiveDataStructAddr(0),
    getAfrLiveDataStructAddr(1),
    getEgtLiveDataStructAddr(0),
    getEgtLiveDataStructAddr(1)
};

FragmentList getFragments() {
	return { fragments, efi::size(fragments) };
}
