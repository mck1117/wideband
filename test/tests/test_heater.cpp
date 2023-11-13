#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "heater_control.h"

struct MockHeater : public HeaterControllerBase {
    MockHeater() : HeaterControllerBase(0) { }

    MOCK_METHOD(void, SetDuty, (float), (const, override));
};

TEST(Heater, Basic)
{
    MockHeater dut;
}
