#pragma once

void StartSampling();

float GetNernstAc(int ch);
float GetSensorInternalResistance(int ch);
float GetSensorTemperature(int ch);
float GetNernstDc(int ch);
float GetPumpNominalCurrent(int ch);
float GetInternalBatteryVoltage(int ch);
