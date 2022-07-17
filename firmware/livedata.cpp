#include "livedata.h"

#include "lambda_conversion.h"
#include "sampling.h"
#include "pump_dac.h"
#include "heater_control.h"

static struct livedata_common_s livedata_common;
static struct livedata_afr_s livedata_afr;

void SamplingUpdateLiveData()
{
    livedata_afr.afr = GetLambda();
    livedata_afr.temperature = GetSensorTemperature();
    livedata_afr.nernstVoltage = GetNernstDc();
    livedata_afr.pumpCurrentTarget = GetPumpCurrent();
    livedata_afr.pumpCurrentMeasured = GetPumpNominalCurrent();
    livedata_afr.heaterDuty = GetHeaterDuty();

    livedata_common.vbatt = GetInternalBatteryVoltage();
}

const struct livedata_common_s * getCommonLiveDataStructAddr()
{
    return &livedata_common;
}

const struct livedata_afr_s * getAfrLiveDataStructAddr()
{
    return &livedata_afr;
}
