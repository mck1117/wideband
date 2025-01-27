#include "pid.h"

float Pid::GetOutput(float setpoint, float observation)
{
    float error = setpoint - observation;

    // Integrate error
    m_integrator += error * m_config->periodMs / 1000.0 * m_config->kI;

    // Differentiate error
    float errorDelta = error - m_lastError;
    float dEdt = errorDelta / m_config->periodMs / 1000.0;
    m_lastError = error;

    // Clamp to +- 1
    if (m_integrator > m_config->clamp) m_integrator = m_config->clamp;
    if (m_integrator < -m_config->clamp) m_integrator = -m_config->clamp;

    // Multiply by gains and sum
    return m_config->kP * error + m_integrator + m_config->kD * dEdt;
}
