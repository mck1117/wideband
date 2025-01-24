#pragma once

struct PidConfig
{
    float kP;
    float kI;
    float kD;
    float clamp;
    int periodMs;
};

class Pid
{
public:
    Pid(const PidConfig& config) 
        : m_config(&config)
    {
    }

    float GetOutput(float setpoint, float observation);

private:
    const PidConfig* const m_config;

    float m_lastError = 0;
    float m_integrator = 0;
};
