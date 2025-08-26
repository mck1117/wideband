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

// implement this for your board if you want some non-standard behavior
// default implementation simply calls SendRusefiFormat
void SendCanForChannel(uint8_t ch);
void SendCanEgtForChannel(uint8_t ch);

// Helpers to support both bxCAN and CANFD peripherals
#ifdef STM32G4XX
#define CAN_EXT(f) ((f).common.XTD)
#define CAN_SID(f) ((f).std.SID)
#define CAN_EID(f) ((f).ext.EID)
#define CAN_ID(f) ((f).common.XTD ? CAN_EID(f) : CAN_SID(f))

#define CAN_IDE_STD 0
#define CAN_IDE_EXT 1

#else
#define CAN_EXT(f) ((f).IDE)
#define CAN_SID(f) ((f).SID)
#define CAN_EID(f) ((f).EID)
#define CAN_ID(f) ((f).IDE ? CAN_EID(f) : CAN_SID(f))
#endif
