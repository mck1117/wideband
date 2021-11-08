#pragma once

#define RUSEFI_WIDEBAND_VERSION (0xA0)

// ascii "rus"
#define WB_RUS 0x727573
#define WB_BL_ENTER 0xEF0'0000
#define WM_MSG_SET_INDEX 0xEF4'0000
#define WB_MGS_ECU_STATUS 0xEF5'0000

namespace wbo
{
struct StandardData
{
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

    uint16_t pad;
};
} // namespace wbo
