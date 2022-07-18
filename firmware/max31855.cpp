#include <math.h>

#include "hal.h"
#include "io_pins.h"
#include "wideband_config.h"
#include "bit.h"

#include "max31855.h"

#ifdef HAL_USE_SPI

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
	state = MAX31855_NO_REPLY;

	int ret = spi_rx(&data);
	if (ret)
		return ret;

	if (data == 0xffffffff) {
		state = MAX31855_NO_REPLY;
		return -1;
	}

	if (data & BIT(16)) {
		if (data & BIT(0)) {
			state = MAX31855_OPEN_CIRCUIT;
		} else if (data & BIT(1)) {
			state = MAX31855_SHORT_TO_GND;
		} else if (data & BIT(2)) {
			state = MAX31855_SHORT_TO_VCC;
		}

		cold_joint_temperature = NAN;
		temperature = NAN;

		return -1;
	}
	state = MAX31855_OK;

	/* D[15:4] */
	int16_t tmp = (data >> 4) & 0xfff;
	/* extend sign */
	tmp = tmp << 4;
	tmp = tmp >> 4;	/* shifting right signed is not a good idea */
	cold_joint_temperature = (float)tmp * 0.0625;

	/* D[31:18] */
	tmp = (data >> 18) & 0x3fff;
	/* extend sign */
	tmp = tmp << 2;
	tmp = tmp >> 2;	/* shifting right signed is not a good idea */
	temperature = (float) tmp * 0.25;

	return 0;
}

void Max31855Thread::ThreadTask() {

	while (true) {
		for (int ch = 0; ch < EGT_CHANNELS; ch++) {
		    Max31855 current = max31855[ch];
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

#endif /* HAL_USE_SPI */
