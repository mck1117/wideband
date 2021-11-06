#pragma once

#define RUSEFI_WIDEBAND_VERSION 01

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
