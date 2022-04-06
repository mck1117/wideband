#pragma once

#include <cstdint>

void InitCan();
void SendCanData(float lambda, uint16_t measuredResistance);
void SendRusefiFormat(uint8_t idx);

enum class HeaterAllow {
    Unknown,
    Allowed,
    NotAllowed,
};

HeaterAllow GetHeaterAllowed();

float GetRemoteBatteryVoltage();
