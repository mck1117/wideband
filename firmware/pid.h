#pragma once

class Pid
{
public:
    Pid(float kP, float kI, float kD, float clamp, int periodMs) 
        : m_period(periodMs / 1000.0f)
        , m_kp(kP)
        , m_ki(kI)
        , m_kd(kD)
        , m_clamp(clamp)
    {
    }

    float GetOutput(float setpoint, float observation);

private:
    const float m_period;
    const float m_kp;
    const float m_ki;
    const float m_kd;
    const float m_clamp;

    float m_lastError = 0;
    float m_integrator = 0;
};
