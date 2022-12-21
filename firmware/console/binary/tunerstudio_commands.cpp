#include "tunerstudio_impl.h"
#include "tunerstudio.h"
#include "tunerstudio_io.h"
#include "byteswap.h"

/* configuration */
#include "port.h"

#include <cstring>

#include <rusefi/crc.h>
#include <rusefi/fragments.h>

void sendErrorCode(TsChannelBase *tsChannel, uint8_t code);
void sendOkResponse(TsChannelBase *tsChannel, ts_response_format_e mode);

FragmentList getFragments();

/**
 * @brief 'Output' command sends out a snapshot of current values
 * Gauges refresh
 */
void TunerStudio::cmdOutputChannels(TsChannelBase* tsChannel, uint16_t offset, uint16_t count) {
	// TODO: check against total framents size
//	if (offset + count > TS_TOTAL_OUTPUT_SIZE) {
//		sendErrorCode(tsChannel, TS_RESPONSE_OUT_OF_RANGE);
//		return;
//	}

	tsChannel->assertPacketSize(count, false);
	// this method is invoked too often to print any debug information
	uint8_t * scratchBuffer = (uint8_t *)tsChannel->scratchBuffer;
	/**
	 * collect data from all models
	 */
	copyRange(scratchBuffer + 3, getFragments(), offset, count);

	tsChannel->crcAndWriteBuffer(TS_RESPONSE_OK, count);
}

// Validate whether the specified offset and count would cause an overrun in the tune.
// Returns true if offset and count are in valid range
bool TunerStudio::validateScatterOffsetCount(size_t offset, size_t count) {
	if (offset + count > sizeof(highSpeedOffsets))
		return false;
	return true;
}

void TunerStudio::handleScatteredReadCommand(TsChannelBase* tsChannel) {
#ifdef HIGH_SPEED_OPTIMIZED
	uint8_t *buffer = (uint8_t *)tsChannel->scratchBuffer;
	tsChannel->writeHeader(TS_RESPONSE_OK, highSpeedTotalSize);
	for (size_t i = 0; i < highSpeedChunks; i++) {
		chDbgAssert(NULL != highSpeedPtrs[i], "NULL pointer in scatter list");
		// copy to temp buffer to avoid changes before transmission, couse it can affect CRC
		memcpy(buffer, highSpeedPtrs[i], highSpeedSizes[i]);
		tsChannel->writeBody(buffer, highSpeedSizes[i]);
	}
	tsChannel->writeTail();
#else
	size_t count = 0;
	uint8_t *buffer = (uint8_t *)tsChannel->scratchBuffer + 3;	/* reserve 3 bytes for header */

	for (size_t i = 0; i < HIGH_SPEED_COUNT; i++) {
		int packed = highSpeedOffsets[i];
		int type = packed >> 13;

		if (type == 0)
			continue;
		int size = 1 << (type - 1);

		int offset = packed & 0x1FFF;
		// write each data point and CRC incrementally
		copyRange(buffer + count, getFragments(), offset, size);
		count += size;
	}
	tsChannel->crcAndWriteBuffer(TS_RESPONSE_OK, count);
#endif
}

void TunerStudio::handleScatterListWriteCommand(TsChannelBase* tsChannel, uint16_t offset, uint16_t count, void *content)
{
	uint8_t * addr = (uint8_t *)highSpeedOffsets + offset;
	memcpy(addr, content, count);

#ifdef HIGH_SPEED_OPTIMIZED
	highSpeedChunks = 0;
	highSpeedTotalSize = 0;
	/* translate to CPU pointers */
	for (int i = 0; i < HIGH_SPEED_COUNT; i++) {
		int packed = highSpeedOffsets[i];
		int type = packed >> 13;

		if (type == 0)
			continue;

		int size = 1 << (type - 1);
		int offset = packed & 0x1FFF;
		highSpeedTotalSize += size;

		uint8_t *ptr;

		do {
			size_t availSize = getRangePtr(&ptr, getFragments(), offset, size);

			/* note: no need to check ptr for NULL */
			if ((highSpeedChunks == 0) ||	/* first chunk */
				((highSpeedChunks > 0) &&
				 (highSpeedPtrs[highSpeedChunks - 1] + highSpeedSizes[highSpeedChunks - 1] != ptr))) /* or not contiguous chunks */ {
				highSpeedPtrs[highSpeedChunks] = ptr;
				highSpeedSizes[highSpeedChunks] = size;
				highSpeedChunks++;
			} else {
				/* unite */
				highSpeedSizes[highSpeedChunks - 1] += size;
			}
		size -= availSize;
		offset += availSize;
		} while (size);
	}
#endif

	sendOkResponse(tsChannel, TS_CRC);
}

void TunerStudio::handleScatterListReadCommand(TsChannelBase* tsChannel, uint16_t offset, uint16_t count)
{
	if (!validateScatterOffsetCount(offset, count)) {
		tunerStudioError(tsChannel, "ERROR: out of range");
		sendErrorCode(tsChannel, TS_RESPONSE_OUT_OF_RANGE);
		return;
	}

	uint8_t * addr = (uint8_t *)highSpeedOffsets + offset;
	tsChannel->sendResponse(TS_CRC, addr, count);
}

void TunerStudio::handleScatterListCrc32Check(TsChannelBase *tsChannel, uint16_t offset, uint16_t count)
{
	// Ensure we are reading from in bounds
	if (!validateScatterOffsetCount(offset, count)) {
		tunerStudioError(tsChannel, "ERROR: out of range");
		sendErrorCode(tsChannel, TS_RESPONSE_OUT_OF_RANGE);
		return;
	}

	const uint8_t* start = (uint8_t *)highSpeedOffsets + offset;

	uint32_t crc = SWAP_UINT32(crc32(start, count));
	tsChannel->sendResponse(TS_CRC, (const uint8_t *) &crc, 4);
}
