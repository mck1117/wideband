/**
 * @file	tunerstudio_io.h
 * @file TS protocol commands and methods are here
 *
 * @date Mar 8, 2015
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#pragma once

/* to get ChibiOS hal config options */
#include "hal.h"

#include "tunerstudio_impl.h"

/* TODO: find better place */
#define BLOCKING_FACTOR 256

#define TS_BURN_COMMAND 'B'
#define TS_CHUNK_WRITE_COMMAND 'C'
#define TS_COMMAND_F 'F'
#define TS_CRC_CHECK_COMMAND 'k'
#define TS_GET_FIRMWARE_VERSION 'V'
#define TS_HELLO_COMMAND 'S'
#define TS_OUTPUT_COMMAND 'O'
#define TS_PROTOCOL "001"
#define TS_QUERY_COMMAND 'Q'
#define TS_READ_COMMAND 'R'
#define TS_TEST_COMMAND 't'
#define TS_GET_SCATTERED_GET_COMMAND '9'
#define TS_IO_TEST_COMMAND 'Z'

#define TS_RESPONSE_BURN_OK 4
#define TS_RESPONSE_COMMAND_OK 7
#define TS_RESPONSE_CRC_FAILURE 0x82
#define TS_RESPONSE_FRAMING_ERROR 0x8D
#define TS_RESPONSE_OK 0
#define TS_RESPONSE_OUT_OF_RANGE 0x84
#define TS_RESPONSE_UNDERRUN 0x80
#define TS_RESPONSE_UNRECOGNIZED_COMMAND 0x83

class TsChannelBase {
public:
	TsChannelBase(const char *name);
	// Virtual functions - implement these for your underlying transport
	virtual void write(const uint8_t* buffer, size_t size, bool isEndOfPacket = false) = 0;
	virtual size_t readTimeout(uint8_t* buffer, size_t size, int timeout) = 0;

	// These functions are optional to implement, not all channels need them
	virtual void flush() { }
	virtual bool isConfigured() const { return true; }
	virtual bool isReady() const { return true; }
	virtual int reStart() { return 0; }
	virtual void stop() { }

	// Base functions that use the above virtual implementation
	size_t read(uint8_t* buffer, size_t size);

#ifdef EFI_CAN_SERIAL
	virtual	// CAN device needs this function to be virtual for small-packet optimization
#endif
	void writeCrcPacket(uint8_t responseCode, const uint8_t* buf, size_t size, bool allowLongPackets = false);
	void sendResponse(ts_response_format_e mode, const uint8_t * buffer, int size, bool allowLongPackets = false);

	/**
	 * See 'blockingFactor' in rusefi.ini
	 */
	char scratchBuffer[BLOCKING_FACTOR + 30];
	const char *name;

	void assertPacketSize(size_t size, bool allowLongPackets);
	void crcAndWriteBuffer(uint8_t responseCode, size_t size);
	void copyAndWriteSmallCrcPacket(uint8_t responseCode, const uint8_t* buf, size_t size);

	int writeHeader(uint8_t responseCode, size_t size);
	int writeBody(uint8_t *buffer, size_t size);
	int writeTail(void);

private:
	void writeCrcPacketLarge(uint8_t responseCode, const uint8_t* buf, size_t size);
	// CRC accumulator
	uint32_t crcAcc;
	// total size expected
	size_t packetSize;
};

// This class represents a channel for a physical async serial poart
class SerialTsChannelBase : public TsChannelBase {
public:
	SerialTsChannelBase(const char *name) : TsChannelBase(name) {};
	// Open the serial port with the specified baud rate
	virtual int start(uint32_t baud) = 0;
};

#if HAL_USE_SERIAL
// This class implements a ChibiOS Serial Driver
class SerialTsChannel : public SerialTsChannelBase {
public:
	SerialTsChannel(SerialDriver& driver) : SerialTsChannelBase("Serial"), m_driver(&driver) { }

	int start(uint32_t newBaud) override;
	// Close and open serial ports with specified baud rate
	// Also will do BT module setup if BT is enabled
	int reStart() override;
	void stop() override;

	void write(const uint8_t* buffer, size_t size, bool isEndOfPacket) override;
	size_t readTimeout(uint8_t* buffer, size_t size, int timeout) override;

private:
	int bt_read_line(char *str, size_t max_len);
	int bt_wait_ok(void);
	int bt_disconnect(void);

	uint32_t baud;
	SerialDriver* const m_driver;
};
#endif // HAL_USE_SERIAL

#if HAL_USE_UART
// This class implements a ChibiOS UART Driver
class UartTsChannel : public SerialTsChannelBase {
public:
	UartTsChannel(UARTDriver& driver) : SerialTsChannelBase("UART"), m_driver(&driver) { }

	int start(uint32_t newBaud) override;
	int reStart() override;
	void stop() override;

	void write(const uint8_t* buffer, size_t size, bool isEndOfPacket) override;
	size_t readTimeout(uint8_t* buffer, size_t size, int timeout) override;

protected:
	UARTDriver* const m_driver;
	UARTConfig m_config;
};
#endif // HAL_USE_UART

#define CRC_VALUE_SIZE 4
// todo: double-check this
#define CRC_WRAPPING_SIZE (CRC_VALUE_SIZE + 3)

// that's 1 second
#define BINARY_IO_TIMEOUT TIME_MS2I(1000)

// that's 1 second
#define SR5_READ_TIMEOUT TIME_MS2I(1000)
