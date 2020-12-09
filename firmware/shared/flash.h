#pragma once

#include <cstdint>
#include <cstddef>

using flashaddr_t = uintptr_t;

// f0 has only 16 bit program width
using flashdata_t = uint16_t;

struct Flash {
    static void ErasePage(uint8_t pageIndex);
    static void Write(flashaddr_t address, const uint8_t* buffer, size_t size);
};
