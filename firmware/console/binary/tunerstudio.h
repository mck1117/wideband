/**
 * @file	tunerstudio.h
 *
 * @date Aug 26, 2013
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#pragma once
#include "tunerstudio_io.h"
//#include "electronic_throttle_generated.h"
//#include "knock_controller_generated.h"
//#include "FragmentEntry.h"

#include "thread_controller.h"

typedef struct {
	int queryCommandCounter;
	int outputChannelsCommandCounter;
	int readPageCommandsCounter;
	int burnCommandCounter;
	int pageCommandCounter;
	int writeValueCommandCounter;
	int crc32CheckCommandCounter;
	int writeChunkCommandCounter;
	int errorCounter;
	int totalCounter;
	int textCommandCounter;
	int testCommandCounter;
} tunerstudio_counters_s;

extern tunerstudio_counters_s tsState;

void tunerStudioDebug(TsChannelBase* tsChannel, const char *msg);
void tunerStudioError(TsChannelBase* tsChannel, const char *msg);

uint8_t* getWorkingPageAddr();

void requestBurn(void);

void startTunerStudioConnectivity(void);

#if defined __GNUC__
// GCC
#define pre_packed
#define post_packed __attribute__((packed))
#else
// IAR
#define pre_packed __packed
#define post_packed
#endif

typedef pre_packed struct
post_packed {
	short int offset;
	short int count;
} TunerStudioWriteChunkRequest;

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
