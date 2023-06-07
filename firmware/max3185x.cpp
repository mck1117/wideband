#include <math.h>

#include "io_pins.h"
#include "wideband_config.h"
#include "bit.h"
#include "livedata.h"

#include "max3185x.h"

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

static Max3185x instances[] = {&spi_config[0], &spi_config[1]};

static Max3185xThread EgtThread(instances);

int Max3185x::spi_txrx(uint8_t tx[], uint8_t rx[], size_t n)
{
	/* Acquire ownership of the bus. */
	spiAcquireBus(EGT_SPI_DRIVER);
	/* Setup transfer parameters. */
	spiStart(EGT_SPI_DRIVER, spi);
	/* Slave Select assertion. */
	spiSelect(EGT_SPI_DRIVER);
	spiExchange(EGT_SPI_DRIVER, n, tx, rx);
	/* Slave Select de-assertion. */
	spiUnselect(EGT_SPI_DRIVER);
	/* Ownership release. */
	spiReleaseBus(EGT_SPI_DRIVER);

	/* no errors for now */
	return 0;
}

int Max3185x::spi_rx32(uint32_t *data)
{
	int ret;
	/* dummy */
	uint8_t tx[4] = {0};
	uint8_t rx[4];

	ret = spi_txrx(tx, rx, 4);
	if (ret) {
		return ret;
	}
	if (data) {
		*data =	(rx[0] << 24) |
				(rx[1] << 16) |
				(rx[2] <<  8) |
				(rx[3] <<  0);
	}
	return 0;
}

int Max3185x::detect()
{
	uint8_t rx[4];
	/* read MASK, CJHF, CJLF */
	uint8_t tx[4] = {0x02, 0x00, 0x00, 0x00};
	uint32_t data;

	int ret = spi_txrx(tx, rx, 4);
	if (ret)
		return ret;
	data =	(rx[0] << 24) |
			(rx[1] << 16) |
			(rx[2] <<  8) |
			(rx[3] <<  0);
	/* MASK, CJHF, CJLF defaults: 0xff, 0x7f, 0xc0 */
	if ((data & 0x00ffffff) == 0x00ff7fc0) {
		/* configure */
		/* CR0: 50 Hz mode
		 * Change the notch frequency only while in the "Normally Off" mode - not in the Automatic
		 * Conversion mode.*/
		tx[0] = 0x80;
		tx[1] = 0x01;
		spi_txrx(tx, rx, 2);
		/* CR0: Automatic Conversion mode, OCFAULT = 2, 50Hz mode */
		tx[1] = BIT(7) | BIT(0) | 2 << 4;
		/* CR1: 4 samples average, K type */
		tx[2] = (2 << 4) | (3 << 0);
		spi_txrx(tx, rx, 3);
		type = MAX31856_TYPE;
		return 0;
	}
	if (data != 0xffffffff) {
		type = MAX31855_TYPE;
		return 0;
	}

	livedata.state = MAX3185X_NO_REPLY;
	type = UNKNOWN_TYPE;
	return -1;
}

int Max3185x::readPacket31855()
{
	uint32_t data;

	int ret = spi_rx32(&data);

	/* TODO: also check for 0x00000000? */
	if ((ret) || (data == 0xffffffff)) {
		livedata.state = MAX3185X_NO_REPLY;

		ret = -1;
	} else if (data & BIT(16)) {
		if (data & BIT(0)) {
			livedata.state = MAX3185X_OPEN_CIRCUIT;
		} else if (data & BIT(1)) {
			livedata.state = MAX3185X_SHORT_TO_GND;
		} else if (data & BIT(2)) {
			livedata.state = MAX3185X_SHORT_TO_VCC;
		}

		ret = -1;
	}

	if (ret) {
		coldJunctionTemperature = NAN;
		livedata.coldJunctionTemperature = 0;
		temperature = NAN;
		livedata.temperature = 0;
	} else {
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

		/* update livedata: float to int */
		livedata.coldJunctionTemperature = coldJunctionTemperature;
		livedata.temperature = temperature;

		livedata.state = MAX3185X_OK;
	}

	return ret;
}

int Max3185x::readPacket31856()
{
	uint8_t rx[7];
	/* read Cold-Junction temperature MSB, LSB, Linearized TC temperature 3 bytes and Fault Status */
	uint8_t tx[7] = {0x0a};

	int ret = spi_txrx(tx, rx, 7);

	if (rx[6] & BIT(0)) {
		livedata.state = MAX3185X_OPEN_CIRCUIT;
		ret = -1;
	} else if (rx[6] & BIT(1)) {
		livedata.state = MAX3185X_SHORT_TO_VCC;
		ret = -1;
	}

	if (ret) {
		coldJunctionTemperature = NAN;
		livedata.coldJunctionTemperature = 0;
		temperature = NAN;
		livedata.temperature = 0;
	} else {
		/* update livedata: float to int */
		coldJunctionTemperature = (float)(rx[1] << 8 | rx[2]) / 256.0;
		temperature = (float)((rx[3] << 11) | (rx[4] << 3) | (rx[5] >> 5)) / 128.0;
		livedata.coldJunctionTemperature = coldJunctionTemperature;
		livedata.temperature = temperature;

		livedata.state = MAX3185X_OK;
	}

	return ret;
}

int Max3185x::readPacket()
{
	int ret;

	if (type == UNKNOWN_TYPE) {
		ret = detect();
		if (ret < 0) {
			return ret;
		}
	}

	if (type == MAX31855_TYPE) {
		return readPacket31855();
	} else if (type == MAX31856_TYPE) {
		return readPacket31856();
	}

	return -1;
}

void Max3185xThread::ThreadTask() {

	while (true) {
		for (int ch = 0; ch < EGT_CHANNELS; ch++) {
		    Max3185x &current = max3185x[ch];
			current.readPacket();
		}

        chThdSleepMilliseconds(500);
	}
}

void StartEgt() {
    EgtThread.Start();
}

Max3185x* getEgtDrivers() {
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
