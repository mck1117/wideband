#include "tunerstudio_impl.h"
#include "tunerstudio.h"
#include "tunerstudio_io.h"

/* configuration */
#include "port.h"

#include <rusefi/fragments.h>

void sendErrorCode(TsChannelBase *tsChannel, uint8_t code);

size_t getTunerStudioPageSize() {
	return GetConfigurationSize();
}

// Validate whether the specified offset and count would cause an overrun in the tune.
// Returns true if an overrun would occur.
bool validateOffsetCount(size_t offset, size_t count, TsChannelBase* tsChannel) {
	if (offset + count > getTunerStudioPageSize()) {
		tunerStudioError(tsChannel, "ERROR: out of range");
		sendErrorCode(tsChannel, TS_RESPONSE_OUT_OF_RANGE);
		return true;
	}

	return false;
}

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
