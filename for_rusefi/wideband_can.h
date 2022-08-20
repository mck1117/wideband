#pragma once

#define RUSEFI_WIDEBAND_VERSION (0xA0)

// ascii "rus"
#define WB_ACK 0x727573
#define WB_BL_ENTER 0xEF0'0000
#define WB_MSG_SET_INDEX 0xEF4'0000
#define WB_MGS_ECU_STATUS 0xEF5'0000
#define WB_DATA_BASE_ADDR 0x190

namespace wbo
{
enum class Fault : uint8_t
{
    None = 0,

    // First fault code at 3 so it's easier to see
    SensorDidntHeat = 3,
    SensorOverheat = 4,
    SensorUnderheat = 5,
};

struct StandardData
{
    // DO NOT move the version field - its position and format must be
    // fixed so that incompatible versions can be idenfitied.
    uint8_t Version;
    uint8_t Valid;

    uint16_t Lambda;
    uint16_t TemperatureC;

    uint16_t pad;
};

struct DiagData
{
    uint16_t Esr;
    uint16_t NernstDc;
    uint8_t PumpDuty;
    Fault Status;

    uint8_t HeaterDuty;
    uint8_t pad;
};

static const char* describeFault(Fault fault) {
    switch (fault) {
        case Fault::None:
            return "OK";
        case Fault::SensorDidntHeat:
            return "Sensor failed to heat";
        case Fault::SensorOverheat:
            return "Sensor overheat";
        case Fault::SensorUnderheat:
            return "Sensor underheat";
    }

    return "Unknown";
}

} // namespace wbo
