#pragma once

#include <cstdint>

void StartHeaterControl();
bool IsRunningClosedLoop();
void SetBatteryVoltage(float vbatt);
void SetHeaterAllowed(bool allowed);
uint16_t GetHeaterDuty();
