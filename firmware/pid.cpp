#include "pid.h"

float Pid::GetOutput(float setpoint, float observation)
{
    float error = setpoint - observation;

    // Integrate error
    m_integrator += error * m_period * m_ki;

    // Differentiate error
    float errorDelta = error - m_lastError;
    float dEdt = errorDelta / m_period;
    m_lastError = error;

    // Clamp to +- 1
    if (m_integrator > m_clamp) m_integrator = m_clamp;
    if (m_integrator < -m_clamp) m_integrator = -m_clamp;

    // Multiply by gains and sum
    return m_kp * error + m_integrator + m_kd * dEdt;
}
