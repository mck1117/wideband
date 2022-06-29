#pragma once

#include "wideband_controller.h"

void StartHeaterControl();
float GetHeaterDuty();
const char* describeHeaterState(HeaterState state);
