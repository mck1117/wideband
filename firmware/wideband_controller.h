#pragma once

#include "pid.h"

#include <cstddef>

struct ChannelAnalogResult;

enum class HeaterState
{
    Preheat,
    WarmupRamp,
    ClosedLoop,
    Stopped,
};

enum class HeaterAllow {
    // no CAN message telling us what to do has been rx'd
    Unknown,

    // We got a message, it said HEAT
    Allowed,

    // We got a message, it said NO HEAT
    NotAllowed,
};

class WidebandController
{
public:
    WidebandController();

    // SAMPLING: Convert ADC information in to O2 sensor information.
    void ProcessSample(const ChannelAnalogResult& result, float virtualGroundVoltageInt);
    float GetNernstDc() const;
    float GetNernstAc() const;
    float GetSensorInternalResistance() const;
    float GetSensorTemperature() const;
    float GetPumpNominalCurrent() const;
    float GetInternalBatteryVoltage() const;

    // LAMBDA CONVERSION: Convert to a useful lambda value.
    float GetLambda() const;
    float GetPhi(float pumpCurrent) const;

    // HEATER CONTROL: Keep the sensor hot
    float GetHeaterVoltage(HeaterAllow heaterAllowState, float batteryVoltage);
    HeaterState GetHeaterState() const;
    bool IsRunningClosedLoop() const;
private:
    // Sampling internal state
    float r_2 = 0;
    float r_3 = 0;

    // sampling results
    float NernstAc = 0;
    float NernstDc = 0;
    float PumpCurrentSenseVoltage = 0;

#ifdef BATTERY_INPUT_DIVIDER
    float InternalBatteryVoltage = 0;
#endif

    // Heater state
    HeaterState heatState;
    Pid heaterPid;
    HeaterState GetNextHeaterState(HeaterState state, HeaterAllow heaterAllowState, float batteryVoltage, float sensorEsr) const;
    float GetVoltageForState(HeaterState state, float heaterEsr);
};

WidebandController& GetController(size_t idx = 0);
