/**
 * Implementation for hardware-serial TunerStudio ports
 */

#include "tunerstudio_io.h"
#include "hal.h"

#if HAL_USE_SERIAL
void SerialTsChannel::start(uint32_t baud) {
	SerialConfig cfg = {
		.speed = baud,
		.cr1 = 0,
		.cr2 = USART_CR2_STOP1_BITS | USART_CR2_LINEN,
		.cr3 = 0
	};

	sdStart(m_driver, &cfg);
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
void UartTsChannel::start(uint32_t baud) {
	m_config.txend1_cb 		= NULL;
	m_config.txend2_cb 		= NULL;
	m_config.rxend_cb 		= NULL;
	m_config.rxchar_cb		= NULL;
	m_config.rxerr_cb		= NULL;
	m_config.timeout_cb		= NULL;
	m_config.speed 			= baud;
	m_config.cr1 			= 0;
	m_config.cr2 			= 0/*USART_CR2_STOP1_BITS*/ | USART_CR2_LINEN;
	m_config.cr3 			= 0;

	uartStart(m_driver, &m_config);
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
