/**
 * @file	tunerstudio_io.cpp
 *
 * @date Mar 8, 2015
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

/* memcpy */
#include <cstring>

#include "tunerstudio_io.h"
#include "byteswap.h"

#include <rusefi/crc.h>

size_t TsChannelBase::read(uint8_t* buffer, size_t size) {
	return readTimeout(buffer, size, SR5_READ_TIMEOUT);
}

#define isBigPacket(size) ((size) > BLOCKING_FACTOR + 7)

void TsChannelBase::copyAndWriteSmallCrcPacket(uint8_t responseCode, const uint8_t* buf, size_t size) {
	auto scratchBuffer = this->scratchBuffer;

	// don't transmit too large a buffer
	chDbgAssert(!isBigPacket(size), "copyAndWriteSmallCrcPacket tried to transmit too large a packet");

	// If transmitting data, copy it in to place in the scratch buffer
	// We want to prevent the data changing itself (higher priority threads could write
	// tsOutputChannels) during the CRC computation.  Instead compute the CRC on our
	// local buffer that nobody else will write.
	if (size) {
		memcpy(scratchBuffer + 3, buf, size);
	}

	crcAndWriteBuffer(responseCode, size);
}

void TsChannelBase::crcAndWriteBuffer(uint8_t responseCode, size_t size) {
	chDbgAssert(!isBigPacket(size), "crcAndWriteBuffer tried to transmit too large a packet");

	auto scratchBuffer = this->scratchBuffer;
	// Index 0/1 = packet size (big endian)
	*(uint16_t*)scratchBuffer = SWAP_UINT16(size + 1);
	// Index 2 = response code
	scratchBuffer[2] = responseCode;

	// CRC is computed on the responseCode and payload but not length
	uint32_t crc = crc32(&scratchBuffer[2], size + 1); // command part of CRC

	// Place the CRC at the end
	*reinterpret_cast<uint32_t*>(&scratchBuffer[size + 3]) = SWAP_UINT32(crc);

	// Write to the underlying stream
	write(reinterpret_cast<uint8_t*>(scratchBuffer), size + 7, true);
	flush();
}

int TsChannelBase::writeHeader(uint8_t responseCode, size_t size)
{
	uint8_t buffer[3];

	// Index 0/1 = packet size (big endian)
	*(uint16_t*)buffer = SWAP_UINT16(size + 1);
	// Index 2 = response code
	buffer[2] = responseCode;

	// start calculating CRC
	// CRC is computed on the responseCode and payload but not length
	crcAcc = crc32(&buffer[2], sizeof(buffer) - 2);
	// save packet size
	packetSize = size; 	/* + 3 bytes for head + 4 bytes of CRC */

	// Write to the underlying stream
	write(buffer, sizeof(buffer), false);

	return sizeof(buffer);
}

int TsChannelBase::writeBody(uint8_t *buffer, size_t size)
{
	chDbgAssert(size <= packetSize, "writeBody packet size is more than provided in header");
	// append CRC
	crcAcc = crc32inc(buffer, crcAcc, size);
	// adjust packet size
	packetSize -= size;

	// Write to the underlying stream
	write(buffer, size, false);

	return size;
}

int TsChannelBase::writeTail(void)
{
	chDbgAssert(0 == packetSize, "writeTail unexpecded packet end");

	uint8_t buffer[4];

	// Place the CRC at the end
	*(uint32_t*)buffer = SWAP_UINT32(crcAcc);

	// Write to the underlying stream
	write(buffer, sizeof(buffer), true);

	return sizeof(buffer);
}

void TsChannelBase::writeCrcPacketLarge(uint8_t responseCode, const uint8_t* buf, size_t size) {
	uint8_t headerBuffer[3];
	uint8_t crcBuffer[4];

	*(uint16_t*)headerBuffer = SWAP_UINT16(size + 1);
	*(uint8_t*)(headerBuffer + 2) = responseCode;

	// Command part of CRC
	uint32_t crc = crc32((void*)(headerBuffer + 2), 1);
	// Data part of CRC
	crc = crc32inc((void*)buf, crc, size);
	*(uint32_t*)crcBuffer = SWAP_UINT32(crc);

	// Write header
	write(headerBuffer, sizeof(headerBuffer), false);

	// If data, write that
	if (size) {
		write(buf, size, false);
	}

	// Lastly the CRC footer
	write(crcBuffer, sizeof(crcBuffer), true);
	flush();
}

TsChannelBase::TsChannelBase(const char *name) {
	this->name = name;
}

void TsChannelBase::assertPacketSize(size_t size, bool allowLongPackets) {
	chDbgAssert(!(isBigPacket(size) && !allowLongPackets), "tried to send disallowed long packet");
}

/**
 * Adds size to the beginning of a packet and a crc32 at the end. Then send the packet.
 */
void TsChannelBase::writeCrcPacket(uint8_t responseCode, const uint8_t* buf, size_t size, bool allowLongPackets) {
	// don't transmit a null buffer...
	if (!buf) {
		size = 0;
	}

	assertPacketSize(size, allowLongPackets);

	if (isBigPacket(size)) {
		// for larger packets we do not use a buffer for CRC calculation meaning data is now allowed to modify while pending
		writeCrcPacketLarge(responseCode, buf, size);
	} else {
		// for small packets we use a buffer for CRC calculation
		copyAndWriteSmallCrcPacket(responseCode, buf, size);
	}
}

void TsChannelBase::sendResponse(ts_response_format_e mode, const uint8_t * buffer, int size, bool allowLongPackets /* = false */) {
	if (mode == TS_CRC) {
		writeCrcPacket(TS_RESPONSE_OK, buffer, size, allowLongPackets);
	} else {
		if (size > 0) {
			write(buffer, size, true);
			flush();
		}
	}
}
