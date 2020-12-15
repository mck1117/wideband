#pragma once

#include <cstdint>

void InitPumpDac();
void SetPumpCurrentTarget(int32_t microamperes);
uint16_t GetPumpOutputDuty();
