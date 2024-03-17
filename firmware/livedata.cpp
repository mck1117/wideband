#include "wideband_config.h"
#include "livedata.h"

#include "lambda_conversion.h"
#include "sampling.h"
#include "pump_dac.h"
#include "heater_control.h"
#include "max3185x.h"
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

        const auto& sampler = GetSampler(ch);
        const auto& heater = GetHeaterController(ch);

        data->lambda = GetLambda(ch);
        data->temperature = sampler.GetSensorTemperature() * 10;
        data->nernstDc = sampler.GetNernstDc() * 1000;
        data->nernstAc = sampler.GetNernstAc() * 1000;
        data->pumpCurrentTarget = GetPumpCurrent(ch);
        data->pumpCurrentMeasured = sampler.GetPumpNominalCurrent();
        data->heaterDuty = GetHeaterDuty(ch) * 1000;    // 0.1 %
        data->heaterEffectiveVoltage = heater.GetHeaterEffectiveVoltage() * 100;
        data->esr = sampler.GetSensorInternalResistance();
        data->fault = (uint8_t)GetCurrentFault(ch);
        data->heaterState = (uint8_t)GetHeaterState(ch);
    }

    livedata_common.vbatt = GetSampler(0).GetInternalHeaterVoltage();
}

template<>
const livedata_common_s* getLiveData(size_t)
{
    return &livedata_common;
}

template<>
const struct livedata_afr_s * getLiveData(size_t ch)
{
    if (ch < AFR_CHANNELS)
    {
        return &livedata_afr[ch];
    }

    return nullptr;
}

static const FragmentEntry fragments[] = {
    decl_frag<livedata_common_s>{},
    decl_frag<livedata_afr_s, 0>{},
    decl_frag<livedata_afr_s, 1>{},
    decl_frag<livedata_egt_s, 0>{},
    decl_frag<livedata_egt_s, 1>{},
};

FragmentList getFragments() {
	return { fragments, efi::size(fragments) };
}
