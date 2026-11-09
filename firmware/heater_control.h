#pragma once

#include <cstdint>

#include "wideband_config.h"

#include "can.h"
#include "pid.h"
#include "timer.h"
#include "sampling.h"

enum class HeaterState
{
    Preheat,
    WarmupRamp,
    ClosedLoop,
    Stopped,
};

struct IHeaterController
{
    // Run one control step, returns the heater duty cycle to apply (0-1)
    virtual float Update(const BoardSampler& sampler, HeaterAllow heaterAllowState) = 0;
    virtual bool IsRunningClosedLoop() const = 0;
    virtual float GetHeaterEffectiveVoltage() const = 0;
    virtual HeaterState GetHeaterState() const = 0;
    virtual float GetTargetTemp() const = 0;
};

class HeaterControllerBase : public IHeaterController
{
public:
    HeaterControllerBase(int ch, int preheatTimeSec, int warmupTimeSec);
    void Configure(float targetTempC, float targetEsr);
    float Update(const BoardSampler& sampler, HeaterAllow heaterAllowState) override;

    bool IsRunningClosedLoop() const override;
    float GetHeaterEffectiveVoltage() const override;
    HeaterState GetHeaterState() const override;
    float GetTargetTemp() const override;

    bool GetIsHeatingEnabled(HeaterAllow heaterAllowState, float batteryVoltage);

    HeaterState
    GetNextState(HeaterState currentState, HeaterAllow haeterAllowState, float batteryVoltage, float sensorTemp);
    float GetVoltageForState(HeaterState state, float sensorEsr);

private:
    Pid m_pid;

    float rampVoltage = 0;
    float heaterVoltage = 0;
    HeaterState heaterState = HeaterState::Preheat;
#ifdef HEATER_MAX_DUTY
    int cycle;
#endif

    float m_targetEsr = 0;
    float m_targetTempC = 0;

    const uint8_t ch;

    const int m_preheatTimeSec;
    const int m_warmupTimeSec;

    Timer m_batteryStableTimer;
    Timer m_preheatTimer;
    Timer m_warmupTimer;

    // Stores the time since a non-over/underheat condition
    // If the timer reaches a threshold, an over/underheat has
    //    occured
    Timer m_underheatTimer;
    Timer m_overheatTimer;

    static const int batteryStabTimeCounter = HEATER_BATTERY_STAB_TIME / HEATER_CONTROL_PERIOD;
};

// A heater controller driving one channel of the heater PWM
class HeaterController final : public HeaterControllerBase
{
public:
    HeaterController(int ch, int pwm_ch)
        : HeaterControllerBase(ch, HEATER_PREHEAT_TIME, HEATER_WARMUP_TIMEOUT)
        , pwm_ch(pwm_ch)
    {
    }

    const uint8_t pwm_ch;
};

// Returns the concrete (final) type rather than IHeaterController so that calls are resolved at
// compile time instead of through the vtable.
const HeaterController& GetHeaterController(int ch);

void StartHeaterControl();
float GetHeaterDuty(int ch);
HeaterState GetHeaterState(int ch);
const char* describeHeaterState(HeaterState state);
