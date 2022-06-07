#pragma once

#include "wideband_config.h"
#include "thread_controller.h"

#define MAX31855_THREAD_STACK 	(512)
#define MAX31855_THREAD_PRIO	(NORMALPRIO + 1)

typedef enum {
	MAX31855_OK = 0,
	MAX31855_OPEN_CIRCUIT = 1,
	MAX31855_SHORT_TO_GND = 2,
	MAX31855_SHORT_TO_VCC = 3,
	MAX31855_NO_REPLY = 4,
} Max31855State;

class Max31855Thread : public ThreadController<MAX31855_THREAD_STACK> {
private:
	int spi_rx(SPIDriver *spi, int ch, uint32_t *data);
	int Read(int ch);
public:
	Max31855State state;
	float int_temp[EGT_CHANNELS];
	float temp[EGT_CHANNELS];

	Max31855Thread(const char* name)
		: ThreadController(name, MAX31855_THREAD_PRIO)
	{
	}
	void ThreadTask() override;
};

extern Max31855Thread EgtThread;
