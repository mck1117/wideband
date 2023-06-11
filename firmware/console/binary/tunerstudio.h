/**
 * @file	tunerstudio.h
 *
 * @date Aug 26, 2013
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#pragma once
#include "tunerstudio_io.h"
#include "thread_controller.h"

typedef struct {
	int queryCommandCounter;
	int outputChannelsCommandCounter;
	int readPageCommandsCounter;
	int burnCommandCounter;
	int crc32CheckCommandCounter;
	int writeChunkCommandCounter;
	int errorCounter;
	int totalCounter;
	int textCommandCounter;
	int testCommandCounter;
} tunerstudio_counters_s;

void tunerStudioDebug(TsChannelBase* tsChannel, const char *msg);
void tunerStudioError(TsChannelBase* tsChannel, const char *msg);

uint8_t* getWorkingPageAddr();

void startTunerStudioConnectivity(void);

typedef struct {
	uint8_t cmd;
	uint16_t subsystem;
	uint16_t index;
} __attribute__((packed)) TunerStudioCmdPacketHeader;

typedef struct {
	uint8_t cmd;
	uint16_t page;
	uint16_t offset;
	uint16_t count;
} __attribute__((packed)) TunerStudioDataPacketHeader;

#define CONNECTIVITY_THREAD_STACK 	(512)
#define CONNECTIVITY_THREAD_PRIO	(NORMALPRIO + 1)

class TunerstudioThread : public ThreadController<CONNECTIVITY_THREAD_STACK> {
public:
	TunerstudioThread(const char* name)
		: ThreadController(name, CONNECTIVITY_THREAD_PRIO)
	{
	}

	// Initialize and return the channel to use for this thread.
	virtual TsChannelBase* setupChannel() = 0;

	void ThreadTask() override;

};
