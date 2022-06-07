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

int Max31855Thread::spi_rx(SPIDriver *spi, int ch, uint32_t *data)
{
	uint8_t rx[4];

	/* Acquire ownership of the bus. */
	spiAcquireBus(spi);
	/* Setup transfer parameters. */
	spiStart(spi, &spi_config[ch]);
	/* Slave Select assertion. */
	spiSelect(spi);
	//spiExchange(spi, 4, tx, rx);
	spiReceive(spi, 4, rx);
	/* Slave Select de-assertion. */
	spiUnselect(spi);
	/* Ownership release. */
	spiReleaseBus(spi);

	if (data) {
		*data = (rx[0] << 24) |
				(rx[1] << 16) |
				(rx[2] <<  8) |
				(rx[3] <<  0);
	}

	/* no errors for now */
	return 0;
}

int Max31855Thread::Read(int ch)
{
	int ret;
	uint32_t data;

	ret = spi_rx(&EGT_SPI_PORT, ch, &data);
	if (ret)
		return ret;

	if (data == 0xffffffff) {
		state = MAX31855_NO_REPLY;
		return -1;
	}

	if (data & BIT(16)) {
		if (data & BIT(0))
			state = MAX31855_OPEN_CIRCUIT;
		else if (data & BIT(1))
			state = MAX31855_SHORT_TO_GND;
		else if (data & BIT(2))
			state = MAX31855_SHORT_TO_VCC;

		int_temp[ch] = NAN;
		temp[ch] = NAN;

		return -1;
	}
	state = MAX31855_OK;

	/* D[15:4] */
	int16_t tmp = (data >> 4) & 0xfff;
	/* extend sign */
	tmp = tmp << 4;
	tmp = tmp >> 4;	/* shifting right signed is not a good idea */
	int_temp[ch] = (float)tmp * 0.0625;

	/* D[31:18] */
	tmp = (data >> 18) & 0x3fff;
	/* extend sign */
	tmp = tmp << 2;
	tmp = tmp >> 2;	/* shifting right signed is not a good idea */
	temp[ch] = (float) tmp * 0.25;

	return 0;
}

void Max31855Thread::ThreadTask() {
	state = MAX31855_NO_REPLY;

	while (true) {
		int ch;

		for (ch = 0; ch < EGT_CHANNELS; ch++) {
			int ret = Read(ch);

			if (ret)
				continue;
		}

        chThdSleepMilliseconds(500);
	}
}

Max31855Thread EgtThread("egt");

#endif /* HAL_USE_SPI */
