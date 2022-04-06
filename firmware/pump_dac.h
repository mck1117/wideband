#pragma once

#include <cstdint>

void InitPumpDac();
void SetPumpCurrentTarget(int32_t microamperes);
float GetPumpOutputDuty();
int32_t GetPumpCurrent();
