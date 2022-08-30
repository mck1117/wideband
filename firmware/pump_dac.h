#pragma once

#include <cstdint>

void InitPumpDac();
void SetPumpCurrentTarget(int ch, int32_t microamperes);
float GetPumpOutputDuty(int ch);
float GetPumpCurrent(int ch);
