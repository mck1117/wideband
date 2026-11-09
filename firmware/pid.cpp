#include "pid.h"

float Pid::GetOutput(float setpoint, float observation)
{
    float error = setpoint - observation;

    // Integrate error
    m_integrator += error * m_periodSec * m_config.kI;

    // Differentiate error, skipping the math entirely if there's no derivative gain
    float derivative = 0;
    if (m_config.kD != 0)
    {
        float dEdt = (error - m_lastError) * m_frequencyHz;
        derivative = m_config.kD * dEdt;
    }
    m_lastError = error;

    // Clamp to +- 1
    if (m_integrator > m_config.clamp)
    {
        m_integrator = m_config.clamp;
    }
    if (m_integrator < -m_config.clamp)
    {
        m_integrator = -m_config.clamp;
    }

    // Multiply by gains and sum
    return m_config.kP * error + m_integrator + derivative;
}
