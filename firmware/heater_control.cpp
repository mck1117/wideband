#include "heater_control.h"

#include "hal.h"
#include "pwm.h"
#include "sampling.h"

// 400khz / 1024 = 390hz PWM
// TODO: this is wired to an inverted output, what do?
Pwm heaterPwm(PWMD1, 0, 400'000, 1024);

void StartHeaterControl()
{
    heaterPwm.Start();
    heaterPwm.SetDuty(0);
}
