#include <math.h>

#include "io_pins.h"
#include "wideband_config.h"
#include "bit.h"
#include "livedata.h"

#include "max3185x.h"

#if (EGT_CHANNELS > 0)

static const SPIConfig spi_config[EGT_CHANNELS] =
{
	{
		.circular = false,
		.slave = false,
		.data_cb = nullptr,
		.error_cb = nullptr,
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
		.slave = false,
		.data_cb = nullptr,
		.error_cb = nullptr,
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

static Max3185x instances[EGT_CHANNELS] = {Max3185x(&spi_config[0]), Max3185x(&spi_config[1])};

static Max3185xThread EgtThread(instances);

int Max3185x::spi_txrx(const uint8_t tx[], uint8_t rx[], size_t n)
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
	/* Bus deinit */
	spiStop(EGT_SPI_DRIVER);
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

// bits D17 and D3 are always expected to be zero
#define MAX31855_RESERVED_BITS	0x20008

Max3185xType Max3185x::detect()
{
	int ret;
	uint8_t rx[4];
	uint8_t tx[4];

	/* try to apply settings to max31956 and then read back settings */
	// Wr, register 0x00
	tx[0] = 0x00 | BIT(7);
	// CR0: 50Hz mode
	// Change the notch frequency only while in the "Normally Off" mode - not in the Automatic
	tx[1] = BIT(0);
	// CR1: 4 samples average, K type
	// The Thermocouple Voltage Conversion Averaging Mode settings should not be changed while
	// conversions are taking place.
	tx[2] = (2 << 4) | (3 << 0);

	// Stop any conversion
	ret = spi_txrx(tx, rx, 2);
	if (ret) {
		return UNKNOWN_TYPE;
	}

	// Apply notch frequency and averaging
	ret = spi_txrx(tx, rx, 3);
	if (ret) {
		return UNKNOWN_TYPE;
	}

	// Start Automatic Conversion mode
	// CR0: Automatic Conversion mode, OCFAULT = 2, 50Hz mode
	tx[1] = BIT(7) | BIT(0) | (2 << 4);
	// CR1: 4 samples average, K type
	tx[2] = (2 << 4) | (3 << 0);
	ret = spi_txrx(tx, rx, 3);
	if (ret) {
		return UNKNOWN_TYPE;
	}

	/* Now readback settings */
	tx[0] = 0x00;
	ret = spi_txrx(tx, rx, 4);
	if ((rx[1] == tx[1]) && (rx[2] == tx[2])) {
		return MAX31856_TYPE;
	}

	/* in case of max31855 we get standart reply with few reserved, always zero bits */
	uint32_t data = (rx[0] << 24) |
					(rx[1] << 16) |
					(rx[2] <<  8) |
					(rx[3] <<  0);

	/* MISO is constantly low or high */
	if ((data == 0xffffffff) || (data == 0x0)) {
		return UNKNOWN_TYPE;
	}

	if ((data & MAX31855_RESERVED_BITS) == 0x0) {
		return MAX31855_TYPE;
	}

	livedata.state = MAX3185X_NO_REPLY;

	return UNKNOWN_TYPE;
}

Max3185xState Max3185x::readPacket31855()
{
	uint32_t data;

	#define MAX33855_FAULT_BIT			BIT(16)
	#define MAX33855_OPEN_BIT			BIT(0)
	#define MAX33855_GND_BIT			BIT(1)
	#define MAX33855_VCC_BIT			BIT(2)

	int ret = spi_rx32(&data);

	if ((ret) ||
		((data & MAX31855_RESERVED_BITS) != 0) ||
		(data == 0x0) ||
		(data == 0xffffffff)) {
		return MAX3185X_NO_REPLY;
	} else if (data & MAX33855_FAULT_BIT) {
		if (data & MAX33855_OPEN_BIT) {
			return MAX3185X_OPEN_CIRCUIT;
		} else if (data & MAX33855_GND_BIT) {
			return MAX3185X_SHORT_TO_GND;
		} else if (data & MAX33855_VCC_BIT) {
			return MAX3185X_SHORT_TO_VCC;
		}
	}

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

	return MAX3185X_OK;
}

Max3185xState Max3185x::readPacket31856()
{
	uint8_t rx[1 + 6];
	/* read one dummy byte, Cold-Junction temperature MSB, LSB, Linearized TC temperature 3 bytes and Fault Status */
	const uint8_t tx[1 + 6] = {0x0a};

	int ret = spi_txrx(tx, rx, sizeof(rx));

	if (ret) {
		return MAX3185X_NO_REPLY;
	} else if (rx[6] & BIT(0)) {
		return MAX3185X_OPEN_CIRCUIT;
	} else if (rx[6] & BIT(1)) {
		return MAX3185X_SHORT_TO_VCC;
	}

	// Paranoid check.
	bool allZero = true;
	for (int i = 1; i < 6; i++) {
		if (rx[i] != 0x00) {
			allZero = false;
			break;
		}
	}
	if (allZero) {
		return MAX3185X_NO_REPLY;
	}

	if (1) {
		// 10 bit before point and 7 bits after
		int32_t tmp = (rx[3] << 11) | (rx[4] << 3) | (rx[5] >> 5);
		/* extend sign: move top bit 18 to 31 */
		tmp = tmp << 13;
		tmp = tmp >> 13;	/* shifting right signed is not a good idea */
		temperature = ((float)tmp) / 128.0;
	}
	if (1) {
		int16_t tmp = (rx[1] << 6) | (rx[2] >> 2);
		/* extend sign */
		tmp = tmp << 2;
		tmp = tmp >> 2;	/* shifting right signed is not a good idea */
		coldJunctionTemperature = ((float)tmp) / 64.0;
	}

	return MAX3185X_OK;
}

Max3185xState Max3185x::readPacket()
{
	if (type == UNKNOWN_TYPE) {
		type = detect();
		if (type == UNKNOWN_TYPE) {
			livedata.state = MAX3185X_NO_REPLY;
		}
	}

	if (type == MAX31855_TYPE) {
		livedata.state = readPacket31855();
	} else if (type == MAX31856_TYPE) {
		livedata.state = readPacket31856();
	}

	if (livedata.state == MAX3185X_OK) {
		/* update livedata: float to int */
		livedata.coldJunctionTemperature = coldJunctionTemperature;
		livedata.temperature = temperature;
	} else {
		coldJunctionTemperature = NAN;
		livedata.coldJunctionTemperature = 0;
		temperature = NAN;
		livedata.temperature = 0;
	}

	/* in case of communication problems - reinit */
	if (livedata.state == MAX3185X_NO_REPLY) {
		livedata.commErrors++;
		type = UNKNOWN_TYPE;
	}

	return MAX3185X_OK;
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
