#pragma once

class Pid
{
public:
    Pid(float kP, float kI, int periodMs) 
        : m_period(periodMs / 1000.0f)
        , m_kp(kP)
        , m_ki(kI)
    {
    }

    float GetOutput(float setpoint, float observation);

private:
    const float m_period;
    const float m_kp;
    const float m_ki;

    float m_lastError;
    float m_integrator;
};
