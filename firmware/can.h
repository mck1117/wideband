#pragma once

#include <cstdint>

#include "wideband_controller.h"

void InitCan();
void SendCanData(float lambda, uint16_t measuredResistance);
void SendRusefiFormat(uint8_t idx);

HeaterAllow GetHeaterAllowed();

float GetRemoteBatteryVoltage();
