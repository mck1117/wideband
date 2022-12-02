#include <math.h>

#include "io_pins.h"
#include "wideband_config.h"
#include "bit.h"
#include "livedata.h"

#include "max31855.h"

#if (EGT_CHANNELS > 0)

static SPIConfig spi_config[2] =
{
	{
		.circular = false,
		.end_cb = NULL,
		.ssport = EGT_CS0_PORT,
		.sspad = EGT_CS0_PIN,
		.cr1 =
			/* SPI_CR1_LSBFIRST | */
			((3 << SPI_CR1_BR_Pos) & SPI_CR1_BR) |	/* div = 16 */
			/* SPI_CR1_CPOL | */ // = 0
			SPI_CR1_CPHA | // = 1
			0,
		.cr2 = 0
	},
	{
		.circular = false,
		.end_cb = NULL,
		.ssport = EGT_CS1_PORT,
		.sspad = EGT_CS1_PIN,
		.cr1 =
			/* SPI_CR1_LSBFIRST | */
			((3 << SPI_CR1_BR_Pos) & SPI_CR1_BR) |	/* div = 16 */
			/* SPI_CR1_CPOL | */ // = 0
			SPI_CR1_CPHA | // = 1
			0,
		.cr2 = 0
	}
};

static Max31855 instances[] = {&spi_config[0], &spi_config[1]};

static Max31855Thread EgtThread(instances);

int Max31855::spi_rx(uint32_t *data)
{
	uint8_t rx[4];

	/* Acquire ownership of the bus. */
	spiAcquireBus(EGT_SPI_DRIVER);
	/* Setup transfer parameters. */
	spiStart(EGT_SPI_DRIVER, spi);
	/* Slave Select assertion. */
	spiSelect(EGT_SPI_DRIVER);
	//spiExchange(spi, 4, tx, rx);
	spiReceive(EGT_SPI_DRIVER, 4, rx);
	/* Slave Select de-assertion. */
	spiUnselect(EGT_SPI_DRIVER);
	/* Ownership release. */
	spiReleaseBus(EGT_SPI_DRIVER);

	if (data) {
		*data = (rx[0] << 24) |
				(rx[1] << 16) |
				(rx[2] <<  8) |
				(rx[3] <<  0);
	}

	/* no errors for now */
	return 0;
}

int Max31855::readPacket()
{
	uint32_t data;

	int ret = spi_rx(&data);

	/* TODO: also check for 0x00000000? */
	if ((ret) || (data == 0xffffffff)) {
		livedata.state = MAX31855_NO_REPLY;

		ret = -1;
	} else if (data & BIT(16)) {
		if (data & BIT(0)) {
			livedata.state = MAX31855_OPEN_CIRCUIT;
		} else if (data & BIT(1)) {
			livedata.state = MAX31855_SHORT_TO_GND;
		} else if (data & BIT(2)) {
			livedata.state = MAX31855_SHORT_TO_VCC;
		}

		ret = -1;
	} else {
		livedata.state = MAX31855_OK;

		/* D[15:4] */
		int16_t tmp = (data >> 4) & 0xfff;
		/* extend sign */
		tmp = tmp << 4;
		tmp = tmp >> 4;	/* shifting right signed is not a good idea */
		coldJunctionTemperature = (float)tmp * 0.0625;

		/* D[31:18] */
		tmp = (data >> 18) & 0x3fff;
		/* extend sign */
		tmp = tmp << 2;
		tmp = tmp >> 2;	/* shifting right signed is not a good idea */
		temperature = (float) tmp * 0.25;
	}

	if (ret) {
		coldJunctionTemperature = NAN;
		livedata.coldJunctionTemperature = 0;
		temperature = NAN;
		livedata.temperature = 0;
	} else {
		/* update livedata: float to int */
		livedata.coldJunctionTemperature = coldJunctionTemperature;
		livedata.temperature = temperature;
	}

	return ret;
}

void Max31855Thread::ThreadTask() {

	while (true) {
		for (int ch = 0; ch < EGT_CHANNELS; ch++) {
		    Max31855 &current = max31855[ch];
			current.readPacket();
		}

        chThdSleepMilliseconds(500);
	}
}

void StartEgt() {
    EgtThread.Start();
}

Max31855* getEgtDrivers() {
    return instances;
}

template<>
const livedata_egt_s* getLiveData(size_t ch)
{
    if (ch < EGT_CHANNELS)
        return &getEgtDrivers()[ch].livedata;
    return NULL;
}

#else

template<>
const livedata_egt_s* getLiveData(size_t)
{
	return nullptr;
}

#endif /* EGT_CHANNELS > 0 */
