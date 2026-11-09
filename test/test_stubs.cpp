#include "status.h"
#include "can.h"

#include "test_stubs.h"

void SetStatus(int, wbo::Status) {}

float GetRemoteBatteryVoltage()
{
    return 0;
}

SensorType g_testSensorType = SensorType::LSU49;

SensorType GetSensorType()
{
    return g_testSensorType;
}

int GetESRSupplyR()
{
    // Nernst AC injection resistor value
    return TestEsrSupplyR;
}
