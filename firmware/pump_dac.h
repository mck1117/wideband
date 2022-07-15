#pragma once

#include <cstdint>

void InitPumpDac();
void SetPumpCurrentTarget(int32_t microamperes);
float GetPumpOutputDuty();
float GetPumpCurrent();
