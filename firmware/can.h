#pragma once

#include <cstdint>

void InitCan();
void SendCanData(float lambda, uint16_t measuredResistance);
void SendRusefiFormat(uint8_t ch);

enum class HeaterAllow {
    // no CAN message telling us what to do has been rx'd
    Unknown,

    // We got a message, it said HEAT
    Allowed,

    // We got a message, it said NO HEAT
    NotAllowed,
};

HeaterAllow GetHeaterAllowed();

float GetRemoteBatteryVoltage();
