#pragma once

#include "hal.h"

#include "wideband_config.h"
#include "thread_controller.h"

typedef enum {
	UNKNOWN_TYPE = 0,
	MAX31855_TYPE = 1,
	MAX31856_TYPE = 2,
} Max3185xType;

typedef enum {
	MAX3185X_OK = 0,
	MAX3185X_OPEN_CIRCUIT = 1,
	MAX3185X_SHORT_TO_GND = 2,
	MAX3185X_SHORT_TO_VCC = 3,
	MAX3185X_NO_REPLY = 4,
} Max3185xState;

/* livedata: +96/112 offset, size = 16 */
struct livedata_egt_s {
	union {
		struct {
			float temperature;
			float coldJunctionTemperature;
			uint8_t state;
			uint8_t pad0[3];
			uint32_t commErrors;
		} __attribute__((packed));
		uint8_t pad[16];
	};
};

// for all board. in case of no EGT - returns NULL
const struct livedata_egt_s * getEgtLiveDataStructAddr(const int ch);

#if (EGT_CHANNELS > 0)

#define MAX3185X_THREAD_STACK 	(512)
#define MAX3185X_THREAD_PRIO	(NORMALPRIO + 1)

class Max3185x {
public:
    Max3185x(const SPIConfig *spi) {
        this->spi = spi;
    }
	livedata_egt_s livedata;
	/* do we need float temperatures? */
	float coldJunctionTemperature;
	float temperature;
	Max3185xType type = UNKNOWN_TYPE;
	Max3185xState readPacket();
private:
    const SPIConfig *spi;
    Max3185xType detect();
	Max3185xState readPacket31855();
	Max3185xState readPacket31856();
	int spi_rx32(uint32_t *data);
	int spi_txrx(const uint8_t tx[], uint8_t rx[], size_t n);
};

class Max3185xThread : public ThreadController<MAX3185X_THREAD_STACK> {
public:
    Max3185xThread(Max3185x max3185x[EGT_CHANNELS])
    : ThreadController("egt", MAX3185X_THREAD_PRIO)
    {
        this->max3185x = max3185x;
    }

	void ThreadTask() override;
private:
    Max3185x *max3185x;
};

void StartEgt();
Max3185x* getEgtDrivers();

#endif // (EGT_CHANNELS > 0)
