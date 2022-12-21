/**
 * @file tunerstudio_impl.h
 */

#pragma once

#include <cstdint>
#include <cstddef>

#define HIGH_SPEED_COUNT 32

#define HIGH_SPEED_OPTIMIZED

struct TsChannelBase;

typedef enum {
	TS_PLAIN = 0,
	TS_CRC = 1
} ts_response_format_e;

class TunerStudioBase {
public:

protected:
	virtual void cmdOutputChannels(TsChannelBase* tsChannel, uint16_t offset, uint16_t count) = 0;
};

class TunerStudio : public TunerStudioBase {
public:
	int handleCrcCommand(TsChannelBase* tsChannel, char *data, size_t incomingPacketSize);
	bool handlePlainCommand(TsChannelBase* tsChannel, uint8_t command);

	void cmdOutputChannels(TsChannelBase* tsChannel, uint16_t offset, uint16_t count) override;
	void handleQueryCommand(TsChannelBase* tsChannel, ts_response_format_e mode);
	void handleExecuteCommand(TsChannelBase* tsChannel, char *data, int incomingPacketSize);
	void handlePageSelectCommand(TsChannelBase *tsChannel, ts_response_format_e mode);
	void handleWriteChunkCommand(TsChannelBase* tsChannel, ts_response_format_e mode, uint16_t offset, uint16_t count,
			void *content);
	void handleCrc32Check(TsChannelBase *tsChannel, ts_response_format_e mode, uint16_t offset, uint16_t count);
	void handleWriteValueCommand(TsChannelBase* tsChannel, ts_response_format_e mode, uint16_t offset, uint8_t value);
	void handlePageReadCommand(TsChannelBase* tsChannel, ts_response_format_e mode, uint16_t offset, uint16_t count);
	// Scatter mode support
	void handleScatteredReadCommand(TsChannelBase* tsChannel);
	void handleScatterListWriteCommand(TsChannelBase* tsChannel, uint16_t offset, uint16_t count, void *content);
	void handleScatterListReadCommand(TsChannelBase* tsChannel, uint16_t offset, uint16_t count);
	void handleScatterListCrc32Check(TsChannelBase *tsChannel, uint16_t offset, uint16_t count);

private:
	void sendErrorCode(TsChannelBase* tsChannel, uint8_t code);

	bool validateScatterOffsetCount(size_t offset, size_t count);
	uint16_t highSpeedOffsets[HIGH_SPEED_COUNT];
#ifdef HIGH_SPEED_OPTIMIZED
	uint8_t *highSpeedPtrs[HIGH_SPEED_COUNT];
	size_t highSpeedSizes[HIGH_SPEED_COUNT];
	size_t highSpeedChunks;
	size_t highSpeedTotalSize;
#endif
};
