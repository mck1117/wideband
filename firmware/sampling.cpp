#include "sampling.h"
#include "interpolation.h"

#include "ch.h"
#include "hal.h"

#include "wideband_config.h"
#include "wideband_controller.h"


#include "port.h"
#include "io_pins.h"

static const struct inter_point lsu49_r_to_temp[] =
{
    {   80, 1030 },
    {  150,  890 },
    {  200,  840 },
    {  250,  805 },
    {  300,  780 },
    {  350,  760 },
    {  400,  745 },
    {  450,  730 },
    {  550,  705 },
    {  650,  685 },
    {  800,  665 },
    { 1000,  640 },
    { 1200,  630 },
    { 2500,  565 },
    // approximated by the greatest measurable sensor resistance
    { 5000,  500 }
};

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

constexpr float f_abs(float x)
{
    return x > 0 ? x : -x;
}

static THD_WORKING_AREA(waSamplingThread, 256);

static void SamplingThread(void*)
{
    // GD32: Insert 20us delay after ADC enable
    chThdSleepMilliseconds(1);

    while(true)
    {
        auto result = AnalogSample();

        // Toggle the pin after sampling so that any switching noise occurs while we're doing our math instead of when sampling
        // TODO: should this be per-controller, or one toggle pin for the whole board?
        palTogglePad(NERNST_ESR_DRIVER_PORT, NERNST_ESR_DRIVER_PIN);

        for (size_t i = 0; i < AFR_CHANNELS; i++)
        {
            GetController(i).ProcessSample(result.ch[i], result.VirtualGroundVoltageInt);
        }
    }
}

void StartSampling()
{
    adcStart(&ADCD1, nullptr);
    chThdCreateStatic(waSamplingThread, sizeof(waSamplingThread), NORMALPRIO + 5, SamplingThread, nullptr);
}

void WidebandController::ProcessSample(const ChannelAnalogResult& result, float virtualGroundVoltageInt) {
    float r_1 = result.NernstVoltage;

    // r2_opposite_phase estimates where the previous sample would be had we not been toggling
    // AKA the absolute value of the difference between r2_opposite_phase and r2 is the amplitude
    // of the AC component on the nernst voltage.  We have to pull this trick so as to use the past 3
    // samples to cancel out any slope in the DC (aka actual nernst cell output) from the AC measurement
    // See firmware/sampling.png for a drawing of what's going on here
    float r2_opposite_phase = (r_1 + r_3) / 2;

    // Compute AC (difference) and DC (average) components
    float nernstAcLocal = f_abs(r2_opposite_phase - r_2);
    NernstDc = (r2_opposite_phase + r_2) / 2;

    NernstAc =
        (1 - ESR_SENSE_ALPHA) * NernstAc +
        ESR_SENSE_ALPHA * nernstAcLocal;

    // Exponential moving average (aka first order lpf)
    PumpCurrentSenseVoltage =
        (1 - PUMP_FILTER_ALPHA) * PumpCurrentSenseVoltage +
        PUMP_FILTER_ALPHA * (result.PumpCurrentVoltage - virtualGroundVoltageInt);

#ifdef BATTERY_INPUT_DIVIDER
    InternalBatteryVoltage = result.BatteryVoltage;
#endif

    // Shift history over by one
    r_3 = r_2;
    r_2 = r_1;
}

float WidebandController::GetNernstAc() const
{
    return NernstAc;
}

float WidebandController::GetSensorInternalResistance() const
{
    // Sensor is the lowside of a divider, top side is 22k, and 3.3v AC pk-pk is injected
    float totalEsr = ESR_SUPPLY_R / (VCC_VOLTS / GetNernstAc() - 1);

    // There is a resistor between the opamp and Vm sensor pin.  Remove the effect of that
    // resistor so that the remainder is only the ESR of the sensor itself
    return totalEsr - VM_RESISTOR_VALUE;
}

float WidebandController::GetSensorTemperature() const
{
    float esr = GetSensorInternalResistance();

    if (esr > 5000)
    {
        return 0;
    }

    return interpolate_1d_float(lsu49_r_to_temp, ARRAY_SIZE(lsu49_r_to_temp), esr);
}

float WidebandController::GetNernstDc() const
{
    return NernstDc;
}

float WidebandController::GetPumpNominalCurrent() const
{
    // Gain is 10x, then a 61.9 ohm resistor
    // Effective resistance with the gain is 619 ohms
    // 1000 is to convert to milliamperes
    constexpr float ratio = -1000 / (PUMP_CURRENT_SENSE_GAIN * LSU_SENSE_R);
    return PumpCurrentSenseVoltage * ratio;
}

float WidebandController::GetInternalBatteryVoltage() const
{
#ifdef BATTERY_INPUT_DIVIDER
    return InternalBatteryVoltage;
#else
    return 0;
#endif
}
