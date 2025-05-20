#pragma once

struct PidConfig
{
    float kP;
    float kI;
    float kD;
    float clamp;
};

class Pid
{
public:
    Pid(const PidConfig& config, float periodMs) 
        : m_config(config)
        , m_periodSec(1e-3 * periodMs)
    {
    }

    float GetOutput(float setpoint, float observation);

private:
    const PidConfig& m_config;
    const float m_periodSec;

    float m_lastError = 0;
    float m_integrator = 0;
};
