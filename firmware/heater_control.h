#pragma once

#include <cstdint>

enum class HeaterState
{
    Preheat,
    WarmupRamp,
    ClosedLoop,
    Stopped,
};

void StartHeaterControl();
bool IsRunningClosedLoop();
float GetHeaterDuty();
HeaterState GetHeaterState();
const char* describeHeaterState(HeaterState state);
