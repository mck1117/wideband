#pragma once

#include <cstdint>

void StartHeaterControl();
bool IsRunningClosedLoop();
uint16_t GetHeaterDuty();
