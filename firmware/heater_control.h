#pragma once

#include <cstdint>

#include "wideband_config.h"

#include "can.h"
#include "pid.h"

enum class HeaterState
{
    Preheat,
    WarmupRamp,
    ClosedLoop,
    Stopped,
    NoHeaterSupply,
};

struct ISampler;

struct IHeaterController
{
    virtual void Update(const ISampler& sampler, HeaterAllow heaterAllowState) = 0;
    virtual bool IsRunningClosedLoop() const = 0;
    virtual float GetHeaterEffectiveVoltage() const = 0;
    virtual HeaterState GetHeaterState() const = 0;
};

class HeaterControllerBase : public IHeaterController
{
public:
    HeaterControllerBase(int ch);
    void Configure(float targetTempC, float targetEsr);
    void Update(const ISampler& sampler, HeaterAllow heaterAllowState) override;

    bool IsRunningClosedLoop() const override;
    float GetHeaterEffectiveVoltage() const override;
    HeaterState GetHeaterState() const override;

    virtual void SetDuty(float duty) const = 0;

protected:
    HeaterState GetNextState(HeaterAllow haeterAllowState, float batteryVoltage, float sensorTemp);
    float GetVoltageForState(float heaterEsr);

private:
    Pid heaterPid =
        {
            0.3f,      // kP
            0.3f,      // kI
            0.01f,     // kD
            3.0f,      // Integrator clamp (volts)
            HEATER_CONTROL_PERIOD
        };

    int timeCounter = preheatTimeCounter;
    int batteryStabTime = batteryStabTimeCounter;
    float rampVoltage = 0;
    float heaterVoltage = 0;
    HeaterState heaterState = HeaterState::Preheat;
#ifdef HEATER_MAX_DUTY
    int cycle;
#endif

    float m_targetEsr = 0;
    float m_targetTempC = 0;

// TODO: private:
public:
    const uint8_t ch;

    static const int preheatTimeCounter = HEATER_PREHEAT_TIME / HEATER_CONTROL_PERIOD;
    static const int batteryStabTimeCounter = HEATER_BATTERY_STAB_TIME / HEATER_CONTROL_PERIOD;
};

const IHeaterController& GetHeaterController(int ch);

void StartHeaterControl();
float GetHeaterDuty(int ch);
HeaterState GetHeaterState(int ch);
const char* describeHeaterState(HeaterState state);
