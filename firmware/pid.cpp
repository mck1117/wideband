#include "pid.h"

float Pid::GetOutput(float setpoint, float observation)
{
    // TODO: is this backwards?
    float error = setpoint - observation;

    // Integrate error
    m_integrator += error * m_period;

    // Multiply by gains and sum
    return m_kp * error + m_ki * m_integrator;
}
