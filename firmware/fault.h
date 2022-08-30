#pragma once

#include <cstdint>

#include "../for_rusefi/wideband_can.h"

void SetFault(int ch, wbo::Fault fault);
bool HasFault();
wbo::Fault GetCurrentFault(int ch);
