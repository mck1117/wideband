#pragma once

#include "hal.h"
#include "port_shared.h"
#include "wideband_config.h"

struct AnalogResult
{
    struct {
        float NernstVoltage;
        float PumpCurrentVoltage;
        /* for dual version - this is voltage on Heater-, switches between zero and Vbatt with heater PWM,
         * used for both Vbatt measurement and Heater diagnostic */
        float BatteryVoltage;
    } ch[AFR_CHANNELS];
    float VirtualGroundVoltageInt;
};

AnalogResult AnalogSample();

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
            uint8_t auxInput[2];
        } __attribute__((packed));

        // pad to 256 bytes including tag
        uint8_t pad[256 - 4];
    };
};

int InitConfiguration();
Configuration& GetConfiguration();
void SetConfiguration(const Configuration& newConfig);

/* TS stuff */
uint8_t *GetConfiguratiuonPtr();
size_t GetConfiguratiuonSize();
void SaveConfiguration();
const char *getTsSignature();
