#pragma once

#include <cstdint>

enum class HeaterState
{
    Preheat,
    WarmupRamp,
    ClosedLoop,
    Stopped,
    NoHeaterSupply,
};

void StartHeaterControl();
bool IsRunningClosedLoop(int ch);
float GetHeaterDuty(int ch);
float GetHeaterEffVoltage(int ch);
HeaterState GetHeaterState(int ch);
const char* describeHeaterState(HeaterState state);
