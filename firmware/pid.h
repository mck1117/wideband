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
        , m_frequencyHz(1 / m_periodSec)
    {
    }

    float GetOutput(float setpoint, float observation);

private:
    const PidConfig& m_config;
    const float m_periodSec;
    // 1 / period, so that differentiating is a multiply rather than a (slow without an FPU) divide
    const float m_frequencyHz;

    float m_lastError = 0;
    float m_integrator = 0;
};
