#pragma once

#include <cstdint>

void InitCan();
void SendCanData(float lambda, uint16_t measuredResistance);
void SendEmulatedAemXseries(uint8_t idx);
void SendRusefiFormat(uint8_t idx);
