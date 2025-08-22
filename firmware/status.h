#pragma once

#include <cstdint>

#include "../for_rusefi/wideband_can.h"

void SetStatus(int ch, wbo::Status fault);
bool HasFault();
wbo::Status GetCurrentStatus(int ch);
