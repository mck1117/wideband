#pragma once

#include <cstdint>
#include <cstddef>

#include "port_shared.h"
#include "wideband_config.h"

struct AnalogChannelResult
{
    float NernstVoltage;
    float PumpCurrentVoltage;
    /* for dual version - this is voltage on Heater-, switches between zero and Vbatt with heater PWM,
        * used for both Vbatt measurement and Heater diagnostic */
    float HeaterSupplyVoltage;
};

struct AnalogResult
{
    AnalogChannelResult ch[AFR_CHANNELS];
    float VirtualGroundVoltageInt;

    #ifdef BOARD_HAS_VOLTAGE_SENSE
    float SupplyVoltage;
    #endif

    float McuTemp;
};

// Enable ADCs, configure pins, etc
void PortPrepareAnalogSampling();
AnalogResult AnalogSample();

enum class SensorType : uint8_t {
    LSU49 = 0,
    LSU42 = 1,
    LSUADV = 2,
};

enum class AuxOutputMode : uint8_t {
    Afr0 = 0,
    Afr1 = 1,
    Lambda0 = 2,
    Lambda1 = 3,
    Egt0 = 4,
    Egt1 = 5,
};

class Configuration {
private:
    // Increment this any time the configuration format changes
    // It is stored along with the data to ensure that it has been written before
    static constexpr uint32_t ExpectedTag = 0xDEADBE01;
    uint32_t Tag = ExpectedTag;

public:
    bool IsValid() const
    {
        return this->Tag == ExpectedTag;
    }
    void LoadDefaults();

    // Actual configuration data
    union {
        struct {
            uint8_t CanIndexOffset = 0;
            // AUX0 and AUX1 curves
            float auxOutBins[2][8];
            float auxOutValues[2][8];
            AuxOutputMode auxOutputSource[2];

            SensorType sensorType;
        } __attribute__((packed));

        // pad to 256 bytes including tag
        uint8_t pad[256 - sizeof(Tag)];
    };
};

int InitConfiguration();
Configuration* GetConfiguration();
void SetConfiguration();

/* TS stuff */
uint8_t *GetConfigurationPtr();
size_t GetConfigurationSize();
int SaveConfiguration();
const char *getTsSignature();

void rebootNow();
void rebootToOpenblt();
void rebootToDfu();

extern "C" void checkDfuAndJump();

// LSU4.2, LSU4.9 or LSU_ADV
SensorType GetSensorType();
void SetupESRDriver(SensorType sensor);
void ToggleESRDriver(SensorType sensor);
int GetESRSupplyR();
