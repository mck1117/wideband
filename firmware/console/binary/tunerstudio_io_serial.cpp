/**
 * Implementation for hardware-serial TunerStudio ports
 */

#include <string.h>

#include "rusefi/arrays.h"
#include "tunerstudio_io.h"
#include "hal.h"
#include "chprintf.h"
#include "wideband_config.h"

#if HAL_USE_SERIAL

#ifndef BT_SERIAL_OVER_JDY33
	#define BT_SERIAL_OVER_JDY33	FALSE
#endif

#ifndef BT_BROADCAST_NAME
	#define BT_BROADCAST_NAME		"RusEFI WBO"
#endif

// JDY-33 has 9: 128000 which we do not
static const unsigned int baudRates[] = 	{	115200, 9600, 	38400,	2400,	4800,	19200,	57600 };
static const unsigned int baudRateCodes[] = {8,		4,		6,		2,		3,		5,		7 };
static_assert(efi::size(baudRates) == efi::size(baudRateCodes));

static const unsigned int btModuleTimeout = TIME_MS2I(100);

int SerialTsChannel::bt_read_line(char *str, size_t max_len)
{
	size_t i = 0;

	/* read until end of line */
	do {
		if (i >= max_len)
			return -1;
		if (readTimeout((uint8_t *)&str[i], 1, btModuleTimeout) != 1)
			return -1;
	} while (str[i++] != '\n');

	return i;
}

int SerialTsChannel::bt_wait_ok(void)
{
	/* wait for '+OK\r\n' */
	char tmp[6];
	if (readTimeout((uint8_t *)tmp, 5, btModuleTimeout) == 5) {
		if (strncmp(tmp, "+OK\r\n", 5) == 0)
			return 0;
	}

	return -1;
}

int SerialTsChannel::bt_disconnect(void)
{
	chprintf((BaseSequentialStream *)m_driver, "AT+DISC\r\n");

	return bt_wait_ok();
}

#ifndef PORT_EXTRA_SERIAL_CR2
#define PORT_EXTRA_SERIAL_CR2 0
#endif

int SerialTsChannel::start(uint32_t newBaud) {
	SerialConfig cfg = {
		.speed = newBaud,
		.cr1 = 0,
		.cr2 = USART_CR2_STOP1_BITS | PORT_EXTRA_SERIAL_CR2,
		.cr3 = 0
	};

	baud = newBaud;

	sdStart(m_driver, &cfg);

	return 0;
}

/* this will also try to reinit BT module */
int SerialTsChannel::reStart() {
	int ret = 0;
	SerialConfig cfg = {
		.speed = baud,
		.cr1 = 0,
		.cr2 = USART_CR2_STOP1_BITS | PORT_EXTRA_SERIAL_CR2,
		.cr3 = 0
	};

	/* Stop first */
	sdStop(m_driver);

	if (BT_SERIAL_OVER_JDY33) {
		/* try BT setup */
		int retry = 3;
		bool done = false;
		size_t baudIdx;

		do {
			for (baudIdx = 0; baudIdx < efi::size(baudRates) && !done; baudIdx++) {
				cfg.speed = baudRates[baudIdx];
				sdStart(m_driver, &cfg);

				chprintf((BaseSequentialStream *)m_driver, "AT\r\n");
				if (bt_wait_ok() != 0) {
					/* try to diconnect in case device already configured and in silence mode */
					if (bt_disconnect() != 0) {
						/* try next baud rate */
						sdStop(m_driver);
						continue;
					}
				}
				done = true;
				break;
			}
		} while ((!done) && (--retry));

		if (retry <= 0) {
			ret = -1;
		}

		if (ret == 0) {
			/* find expected baudrate */
			for (baudIdx = 0; baudIdx < efi::size(baudRates); baudIdx++) {
				if (baud == baudRates[baudIdx]) {
					break;
				}
			}
			if (baudIdx == efi::size(baudRates)) {
				/* unknown baudrate */
				ret = -2;
			}
		}

		if (ret == 0) {
			int len;
			char tmp[64];
			/* setup */
			done = false;
			do {
				/* just a curious */
				chprintf((BaseSequentialStream *)m_driver, "AT+VERSION\r\n");
				len = bt_read_line(tmp, sizeof(tmp));
				if (len < 0) {
					/* retry */
					continue;
				}

				/* Reset settings to defaults */
				chprintf((BaseSequentialStream *)m_driver, "AT+DEFAULT\r\n");
				if (bt_wait_ok() != 0) {
					/* retry */
					continue;
				}

				/* SPP Broadcast name: up to 18 bytes */
				chprintf((BaseSequentialStream *)m_driver, "AT+NAME%s\r\n", BT_BROADCAST_NAME);
				if (bt_wait_ok() != 0) {
					/* retry */
					continue;
				}

				/* BLE Broadcast name: up to 18 bytes */
				chprintf((BaseSequentialStream *)m_driver, "AT+NAMB%s\r\n", BT_BROADCAST_NAME " BLE");
				if (bt_wait_ok() != 0) {
					/* retry */
					continue;
				}

				/* SPP connection with no password */
				chprintf((BaseSequentialStream *)m_driver, "AT+TYPE%d\r\n", 0);
				if (bt_wait_ok() != 0) {
					/* retry */
					continue;
				}

				/* Disable serial port status output */
				chprintf((BaseSequentialStream *)m_driver, "AT+ENLOG%d\r\n", 0);
				if (bt_wait_ok() != 0) {
					/* retry */
					continue;
				}

				chprintf((BaseSequentialStream *)m_driver, "AT+BAUD%d\r\n", baudRateCodes[baudIdx]);
				if (bt_wait_ok() != 0) {
					/* retry */
					continue;
				}

				/* BT module changes baud rate here */
				done = true;
			} while ((!done) && (--retry));

			if (retry <= 0) {
				sdStop(m_driver);
				ret = -3;
			}
		}

		if (ret == 0) {
			/* switch to new baudrate? */
			if (cfg.speed != baud) {
				sdStop(m_driver);

				if (ret == 0) {
					/* switch baudrate */
					cfg.speed = baud;
					sdStart(m_driver, &cfg);

					chThdSleepMilliseconds(10);
				}
			}
		}

		if (ret == 0) {
			/* now reset BT to apply new settings */
			chprintf((BaseSequentialStream *)m_driver, "AT+RESET\r\n");
			if (bt_wait_ok() != 0) {
				sdStop(m_driver);
				ret = -4;
			}
		}

		if (ret < 0) {
			/* set requested baudrate and wait for direct uart connection */
			cfg.speed = baud;
			sdStart(m_driver, &cfg);
		}
	} else {
		/* Direct uart connetion */
		sdStart(m_driver, &cfg);
	}

	return ret;
}

void SerialTsChannel::stop() {
	sdStop(m_driver);
}

void SerialTsChannel::write(const uint8_t* buffer, size_t size, bool) {
	chnWriteTimeout(m_driver, buffer, size, BINARY_IO_TIMEOUT);
}

size_t SerialTsChannel::readTimeout(uint8_t* buffer, size_t size, int timeout) {
	return chnReadTimeout(m_driver, buffer, size, timeout);
}
#endif // HAL_USE_SERIAL

#if (HAL_USE_UART == TRUE) && (UART_USE_WAIT == TRUE)
int UartTsChannel::start(uint32_t newBaud) {
	m_config.txend1_cb 		= NULL;
	m_config.txend2_cb 		= NULL;
	m_config.rxend_cb 		= NULL;
	m_config.rxchar_cb		= NULL;
	m_config.rxerr_cb		= NULL;
	m_config.timeout_cb		= NULL;
	m_config.speed 			= newBaud;
	m_config.cr1 			= 0;
	m_config.cr2 			= 0/*USART_CR2_STOP1_BITS*/ | USART_CR2_LINEN;
	m_config.cr3 			= 0;

	uartStart(m_driver, &m_config);

	return 0;
}

int UartTsChannel::reStart() {
	stop();
	/* TODO: add BT setup? */
	start(baud);
}

void UartTsChannel::stop() {
	uartStop(m_driver);
}

void UartTsChannel::write(const uint8_t* buffer, size_t size, bool) {
	uartSendTimeout(m_driver, &size, buffer, BINARY_IO_TIMEOUT);
}

size_t UartTsChannel::readTimeout(uint8_t* buffer, size_t size, int timeout) {
	uartReceiveTimeout(m_driver, &size, buffer, timeout);
	return size;
}
#endif // HAL_USE_UART
