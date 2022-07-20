#pragma once

#include "wideband_config.h"
#include "thread_controller.h"

#if HAL_USE_SPI

#define MAX31855_THREAD_STACK 	(512)
#define MAX31855_THREAD_PRIO	(NORMALPRIO + 1)

typedef enum {
	MAX31855_OK = 0,
	MAX31855_OPEN_CIRCUIT = 1,
	MAX31855_SHORT_TO_GND = 2,
	MAX31855_SHORT_TO_VCC = 3,
	MAX31855_NO_REPLY = 4,
} Max31855State;

class Max31855 {
public:
    Max31855(SPIConfig *spi) {
        this->spi = spi;
    }
	Max31855State state;
	float cold_joint_temperature;
	float temperature;
	int readPacket();
private:
    SPIConfig *spi;
	int spi_rx(uint32_t *data);
};

class Max31855Thread : public ThreadController<MAX31855_THREAD_STACK> {
public:
    Max31855Thread(Max31855 max31855[EGT_CHANNELS])
    : ThreadController("egt", MAX31855_THREAD_PRIO)
    {
        this->max31855 = max31855;
    }

	void ThreadTask() override;
private:
    Max31855 *max31855;
};

void StartEgt();
Max31855* getEgtDrivers();

#endif // HAL_USE_SPI
