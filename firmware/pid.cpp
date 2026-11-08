#include "pid.h"

float Pid::GetOutput(float setpoint, float observation)
{
    float error = setpoint - observation;

    // Integrate error
    m_integrator += error * m_periodSec * m_config.kI;

    // Clamp to +- 1
    if (m_integrator > m_config.clamp)
    {
        m_integrator = m_config.clamp;
    }
    if (m_integrator < -m_config.clamp)
    {
        m_integrator = -m_config.clamp;
    }

    // Differentiate error , avoid costly calculations if KD = 0
    if (m_config.kD == 0.0f) {
        m_lastError = error;
        return m_config.kP * error + m_integrator;
    }

    float dEdt = (error - m_lastError) / m_periodSec;
    m_lastError = error;

    return m_config.kP * error + m_integrator
        + m_config.kD * dEdt;
}