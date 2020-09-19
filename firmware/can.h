#pragma once

#include <cstdint>

void InitCan();
void SendCanData(float lambda, uint16_t measuredResistance);
