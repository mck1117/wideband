/**
 * @file	tunerstudio.cpp
 * @brief	Binary protocol implementation
 *
 * This implementation would not happen without the documentation
 * provided by Jon Zeeff (jon@zeeff.com)
 *
 *
 * @brief Integration with EFI Analytics Tuner Studio software
 *
 * Tuner Studio has a really simple protocol, a minimal implementation
 * capable of displaying current engine state on the gauges would
 * require only two commands: queryCommand and ochGetCommand
 *
 * queryCommand:
 * 		Communication initialization command. TunerStudio sends a single byte H
 * 		ECU response:
 * 			One of the known ECU id strings.
 *
 * ochGetCommand:
 * 		Request for output channels state.TunerStudio sends a single byte O
 * 		ECU response:
 * 			A snapshot of output channels as described in [OutputChannels] section of the .ini file
 * 			The length of this block is 'ochBlockSize' property of the .ini file
 *
 * These two commands are enough to get working gauges. In order to start configuring the ECU using
 * tuner studio, three more commands should be implemented:
 *
 *
 * @date Oct 22, 2013
 * @author Andrey Belomutskiy, (c) 2012-2020
 *
 * This file is part of rusEfi - see http://rusefi.com
 *
 * rusEfi is free software; you can redistribute it and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * rusEfi is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * This file is part of rusEfi - see http://rusefi.com
 *
 * rusEfi is free software; you can redistribute it and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * rusEfi is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 *
 */

//#include "os_access.h"

#include <cstring>

/* configuration */
#include "port.h"

/* chprintf */
#include "chprintf.h"

#include "tunerstudio.h"
#include "tunerstudio_impl.h"
#include "byteswap.h"

#include "indication.h"

#include <rusefi/crc.h>

#ifndef EFI_BLUETOOTH_SETUP
	#define EFI_BLUETOOTH_SETUP 0
#endif

tunerstudio_counters_s tsState;

static void printErrorCounters() {
//	efiPrintf("TunerStudio size=%d / total=%d / errors=%d / H=%d / O=%d / P=%d / B=%d",
//			sizeof(engine->outputChannels), tsState.totalCounter, tsState.errorCounter, tsState.queryCommandCounter,
//			tsState.outputChannelsCommandCounter, tsState.readPageCommandsCounter, tsState.burnCommandCounter);
//	efiPrintf("TunerStudio W=%d / C=%d", tsState.writeValueCommandCounter,
//			tsState.writeChunkCommandCounter);
}

/* TunerStudio repeats connection attempts at ~1Hz rate.
 * So first char receive timeout should be less than 1s */
#define TS_COMMUNICATION_TIMEOUT	TIME_MS2I(500)	//0.5 Sec
#define TS_BT_COMMUNICATION_TIMEOUT TIME_MS2I(30 * 1000) // 30 Sec

void tunerStudioDebug(TsChannelBase* tsChannel, const char *msg) {
	(void)tsChannel;
	(void)msg;
//#if EFI_TUNER_STUDIO_VERBOSE
//	efiPrintf("%s: %s", tsChannel->name, msg);
//#endif /* EFI_TUNER_STUDIO_VERBOSE */
}

uint8_t* getWorkingPageAddr() {
	return GetConfigurationPtr();
}

void sendOkResponse(TsChannelBase *tsChannel, ts_response_format_e mode) {
	tsChannel->sendResponse(mode, NULL, 0);
}

void sendErrorCode(TsChannelBase *tsChannel, uint8_t code) {
	tsChannel->writeCrcPacket(code, nullptr, 0);
}

void TunerStudio::sendErrorCode(TsChannelBase* tsChannel, uint8_t code) {
	::sendErrorCode(tsChannel, code);
}

size_t getTunerStudioPageSize() {
	return GetConfigurationSize();
}

// Validate whether the specified offset and count would cause an overrun in the tune.
// Returns true if offset and count are in valid range
bool validateOffsetCount(size_t offset, size_t count) {
	if (offset + count > getTunerStudioPageSize())
		return false;
	return true;
}

/**
 * This command is needed to make the whole transfer a bit faster
 * @note See also handleWriteValueCommand
 */
void TunerStudio::handleWriteChunkCommand(TsChannelBase* tsChannel, ts_response_format_e mode, uint16_t offset, uint16_t count,
		void *content) {
	(void)content;

	tsState.writeChunkCommandCounter++;

	if (!validateOffsetCount(offset, count)) {
		tunerStudioError(tsChannel, "ERROR: out of range");
		sendErrorCode(tsChannel, TS_RESPONSE_OUT_OF_RANGE);
		return;
	}

	uint8_t * addr = (uint8_t *) (getWorkingPageAddr() + offset);
	memcpy(addr, content, count);

	sendOkResponse(tsChannel, mode);
}

void TunerStudio::handleCrc32Check(TsChannelBase *tsChannel, ts_response_format_e mode, uint16_t offset, uint16_t count) {
	tsState.crc32CheckCommandCounter++;

	// Ensure we are reading from in bounds
	if (!validateOffsetCount(offset, count)) {
		tunerStudioError(tsChannel, "ERROR: out of range");
		sendErrorCode(tsChannel, TS_RESPONSE_OUT_OF_RANGE);
		return;
	}

	const uint8_t* start = getWorkingPageAddr() + offset;

	uint32_t crc = SWAP_UINT32(crc32(start, count));
	tsChannel->sendResponse(mode, (const uint8_t *) &crc, 4);
}

void TunerStudio::handlePageReadCommand(TsChannelBase* tsChannel, ts_response_format_e mode, uint16_t offset, uint16_t count) {
	tsState.readPageCommandsCounter++;

	if (!validateOffsetCount(offset, count)) {
		tunerStudioError(tsChannel, "ERROR: out of range");
		sendErrorCode(tsChannel, TS_RESPONSE_OUT_OF_RANGE);
		return;
	}

	const uint8_t* addr = getWorkingPageAddr() + offset;
	tsChannel->sendResponse(mode, addr, count);
}

static void sendResponseCode(ts_response_format_e mode, TsChannelBase *tsChannel, const uint8_t responseCode) {
	if (mode == TS_CRC) {
		tsChannel->writeCrcPacket(responseCode, nullptr, 0);
	}
}

/**
 * 'Burn' command is a command to commit the changes
 */
static void handleBurnCommand(TsChannelBase* tsChannel, ts_response_format_e mode) {
	tsState.burnCommandCounter++;

	int ret = SaveConfiguration();
	if (ret) {
		tunerStudioError(tsChannel, "ERROR: failed to save settings");
	}

	/* TODO: reply error? */
	sendResponseCode(mode, tsChannel, TS_RESPONSE_BURN_OK);
}

static void handleIoTestCommand(TsChannelBase* tsChannel, ts_response_format_e mode, uint16_t subsystem, uint16_t /* index */) {
	/* index is not used yet */

	switch (subsystem) {
	/* DFU */
	case 0xba:
		/* Send ok to make TS happy, wait until sent */
		sendOkResponse(tsChannel, TS_CRC);
		chThdSleepMilliseconds(100);
		rebootToDfu();
		break;

	case 0xbb:
		/* Send ok to make TS happy, wait until sent */
		sendOkResponse(tsChannel, TS_CRC);
		chThdSleepMilliseconds(100);
		rebootNow();
		break;

	case 0xbc:
		/* Send ok to make TS happy, wait until sent */
		sendOkResponse(tsChannel, TS_CRC);
		chThdSleepMilliseconds(100);
		/* Jump to OpenBLT if present */
		rebootToOpenblt();
		break;

	default:
		tunerStudioError(tsChannel, "Unexpected IoTest command");
	}
}

static bool isKnownCommand(char command) {
	return command == TS_HELLO_COMMAND || command == TS_READ_COMMAND || command == TS_OUTPUT_COMMAND
			|| command == TS_BURN_COMMAND
			|| command == TS_CHUNK_WRITE_COMMAND
			|| command == TS_GET_SCATTERED_GET_COMMAND
			|| command == TS_CRC_CHECK_COMMAND
			|| command == TS_GET_FIRMWARE_VERSION
			|| command == TS_IO_TEST_COMMAND;
}

/**
 * rusEfi own test command
 */

static void handleTestCommand(TsChannelBase* tsChannel) {
	tsState.testCommandCounter++;
	char testOutputBuffer[64];
	/**
	 * this is NOT a standard TunerStudio command, this is my own
	 * extension of the protocol to simplify troubleshooting
	 */
	tunerStudioDebug(tsChannel, "got T (Test)");
	chsnprintf(testOutputBuffer, sizeof(testOutputBuffer), BOARD_NAME "\r\n");
	tsChannel->write((const uint8_t*)testOutputBuffer, strlen(testOutputBuffer));

	chsnprintf(testOutputBuffer, sizeof(testOutputBuffer), __DATE__ "\r\n");
	tsChannel->write((const uint8_t*)testOutputBuffer, strlen(testOutputBuffer));

/*
	if (hasFirmwareError()) {
		const char* error = getCriticalErrorMessage();
		chsnprintf(testOutputBuffer, sizeof(testOutputBuffer), "error=%s\r\n", error);
		tsChannel->write((const uint8_t*)testOutputBuffer, strlen(testOutputBuffer));
	}
*/

	tsChannel->flush();
}

/**
 * this command is part of protocol initialization
 *
 * Query with CRC takes place while re-establishing connection
 * Query without CRC takes place on TunerStudio startup
 */
void TunerStudio::handleQueryCommand(TsChannelBase* tsChannel, ts_response_format_e mode) {
	tsState.queryCommandCounter++;

	const char *signature = getTsSignature();
	tsChannel->sendResponse(mode, (const uint8_t *)signature, strlen(signature) + 1);
}

/**
 * handle non CRC wrapped command
 *
 * @return true if legacy command was processed, false otherwise
 */
bool TunerStudio::handlePlainCommand(TsChannelBase* tsChannel, uint8_t command) {
	// Bail fast if guaranteed not to be a plain command
	if (command == 0) {
		return false;
	} else if (command == TS_HELLO_COMMAND || command == TS_QUERY_COMMAND) {
		// We interpret 'Q' as TS_HELLO_COMMAND, since TS uses hardcoded 'Q' during ECU detection (scan all serial ports)
		//efiPrintf("Got naked Query command");
		handleQueryCommand(tsChannel, TS_PLAIN);
		return true;
	} else if (command == TS_TEST_COMMAND || command == 'T') {
		handleTestCommand(tsChannel);
		return true;
	} else if (command == TS_COMMAND_F) {
		/**
		 * http://www.msextra.com/forums/viewtopic.php?f=122&t=48327
		 * Response from TS support: This is an optional command		 *
		 * "The F command is used to find what ini. file needs to be loaded in TunerStudio to match the controller.
		 * If you are able to just make your firmware ignore the command that would work.
		 * Currently on some firmware versions the F command is not used and is just ignored by the firmware as a unknown command."
		 */

		tunerStudioDebug(tsChannel, "not ignoring F");
		tsChannel->write((const uint8_t *)TS_PROTOCOL, strlen(TS_PROTOCOL));
		tsChannel->flush();
		return true;
	} else {
		// This wasn't a valid command
		return false;
	}
}

TunerStudio tsInstance;

static int tsProcessOne(TsChannelBase* tsChannel) {
	static bool in_sync = false;

	if (!tsChannel->isReady()) {
		chThdSleepMilliseconds(10);
		return -1;
	}

	tsState.totalCounter++;

	uint8_t firstByte;
	int received = tsChannel->readTimeout(&firstByte, 1, TS_COMMUNICATION_TIMEOUT);

	if (received != 1) {
//			tunerStudioError("ERROR: no command");
#if EFI_BLUETOOTH_SETUP
		// no data in a whole second means time to disconnect BT
		// assume there's connection loss and notify the bluetooth init code
		bluetoothSoftwareDisconnectNotify();
#endif  /* EFI_BLUETOOTH_SETUP */
		in_sync = false;
		return -1;
	}

	if (tsInstance.handlePlainCommand(tsChannel, firstByte)) {
		return -1;
	}

	uint8_t secondByte;
	/* second byte should be received within minimal delay */
	received = tsChannel->readTimeout(&secondByte, 1, TIME_MS2I(10));
	if (received != 1) {
		tunerStudioError(tsChannel, "TS: ERROR: no second byte");
		in_sync = false;
		return -1;
	}

	uint16_t incomingPacketSize = firstByte << 8 | secondByte;

	if (incomingPacketSize == 0 || incomingPacketSize > (sizeof(tsChannel->scratchBuffer) - CRC_WRAPPING_SIZE)) {
		//efiPrintf("process_ts: channel=%s invalid size: %d", tsChannel->name, incomingPacketSize);
		//tunerStudioError(tsChannel, "process_ts: ERROR: CRC header size");
		if (in_sync) {
			/* send error only if previously we where in sync */
			sendErrorCode(tsChannel, TS_RESPONSE_UNDERRUN);
		}
		in_sync = false;
		return -1;
	}

	char command;
	int expectedSize = incomingPacketSize + CRC_VALUE_SIZE;

	/* NOTE: This part is very sensetive to execution time. With -O0 optimization
	 * we will get a lot of underrun error here because of lost of one byte here */

	if (in_sync == false) {
		/* receive only command byte to check if it is supported */
		received = tsChannel->readTimeout((uint8_t*)(tsChannel->scratchBuffer), 1, TS_COMMUNICATION_TIMEOUT);

		command = tsChannel->scratchBuffer[0];
		if (!isKnownCommand(command)) {
			/* do not report any error as we are not in sync */
			return -1;
		}

		received = tsChannel->readTimeout((uint8_t*)(tsChannel->scratchBuffer) + 1, expectedSize - 1, TS_COMMUNICATION_TIMEOUT);
		if (received != expectedSize - 1) {
			/* do not report any error as we are not in sync */
			return -1;
		}
	} else {
		/* receive full packet, only after check if command is supported
		 * otherwise it will break syncronization and cause rain of errors */
		received = tsChannel->readTimeout((uint8_t*)(tsChannel->scratchBuffer), expectedSize, TS_COMMUNICATION_TIMEOUT);
		if (received != expectedSize) {
			//efiPrintf("Got only %d bytes while expecting %d for command %c", received,
			//		expectedSize, command);
			//tunerStudioError(tsChannel, "ERROR: not enough bytes in stream");
			if (in_sync) {
				/* send error only if previously we where in sync */
				sendErrorCode(tsChannel, TS_RESPONSE_UNDERRUN);
			}
			in_sync = false;
			return -1;
		}

		command = tsChannel->scratchBuffer[0];
		if (!isKnownCommand(command)) {
			//efiPrintf("unexpected command %x", command);
			if (in_sync) {
				/* send error only if previously we where in sync */
				sendErrorCode(tsChannel, TS_RESPONSE_UNRECOGNIZED_COMMAND);
			}
			in_sync = false;
			return -1;
		}
	}

	uint32_t expectedCrc = *(uint32_t*) (tsChannel->scratchBuffer + incomingPacketSize);

	expectedCrc = SWAP_UINT32(expectedCrc);

	uint32_t actualCrc = crc32(tsChannel->scratchBuffer, incomingPacketSize);
	if (actualCrc != expectedCrc) {
		//efiPrintf("TunerStudio: CRC %x %x %x %x", tsChannel->scratchBuffer[incomingPacketSize + 0],
		//		tsChannel->scratchBuffer[incomingPacketSize + 1], tsChannel->scratchBuffer[incomingPacketSize + 2],
		//		tsChannel->scratchBuffer[incomingPacketSize + 3]);

		//efiPrintf("TunerStudio: command %c actual CRC %x/expected %x", tsChannel->scratchBuffer[0],
		//		actualCrc, expectedCrc);
		tunerStudioError(tsChannel, "ERROR: CRC issue");
		if (in_sync) {
			/* send error only if previously we where in sync */
			sendErrorCode(tsChannel, TS_RESPONSE_CRC_FAILURE);
		}
		in_sync = false;
		return -1;
	}

	/* we were able to receive command with correct crc and size! */
	in_sync = true;

	int success = tsInstance.handleCrcCommand(tsChannel, tsChannel->scratchBuffer, incomingPacketSize);

	if (!success) {
		//efiPrintf("got unexpected TunerStudio command %x:%c", command, command);
		return -1;
	}

	return 0;
}

void TunerstudioThread::ThreadTask() {
	bool btInitAttempted = false;
	sysinterval_t btTimeout = 0;
	auto channel = setupChannel();

	// No channel configured for this thread, cancel.
	if (!channel || !channel->isConfigured()) {
		return;
	}

	// Until the end of time, process incoming messages.
	while (true) {
		if (tsProcessOne(channel) == 0) {
			onDataArrived(true);
			btTimeout = 0;
		} else {
			btTimeout += TS_COMMUNICATION_TIMEOUT;

			if ((btTimeout >= TS_BT_COMMUNICATION_TIMEOUT) &&
				(btInitAttempted == false)) {
				// Try to init BT module
				channel->reStart();
				// Try this only once
				btInitAttempted = true;
			}
			onDataArrived(false);
		}
	}
}

void tunerStudioError(TsChannelBase* tsChannel, const char *msg) {
	tunerStudioDebug(tsChannel, msg);
	printErrorCounters();
	tsState.errorCounter++;
}

static void handleGetVersion(TsChannelBase* tsChannel) {
	char versionBuffer[32];
	chsnprintf(versionBuffer, sizeof(versionBuffer), BOARD_NAME);
	tsChannel->sendResponse(TS_CRC, (const uint8_t *) versionBuffer, strlen(versionBuffer) + 1);
}

int TunerStudio::handleCrcCommand(TsChannelBase* tsChannel, char *data, size_t incomingPacketSize) {
	bool handled = true;
	(void)incomingPacketSize;

	char command = data[0];

	/* commands with no arguments */
	switch(command)
	{
	case TS_GET_SCATTERED_GET_COMMAND:
		handleScatteredReadCommand(tsChannel);
		break;
	case TS_HELLO_COMMAND:
		tunerStudioDebug(tsChannel, "got Query command");
		handleQueryCommand(tsChannel, TS_CRC);
		break;
	case TS_GET_FIRMWARE_VERSION:
		handleGetVersion(tsChannel);
		break;
	case TS_BURN_COMMAND:
		handleBurnCommand(tsChannel, TS_CRC);
		break;
	case TS_TEST_COMMAND:
		[[fallthrough]];
	case 'T':
		handleTestCommand(tsChannel);
		break;
	default:
		/* noone of simple commands */
		handled = false;
	}

	if (handled)
		return true;

	/* check if we can extract subsystem and index for IoTest command */
	if (incomingPacketSize < sizeof(TunerStudioCmdPacketHeader)) {
		sendErrorCode(tsChannel, TS_RESPONSE_UNDERRUN);
		tunerStudioError(tsChannel, "ERROR: underrun");
		return false;
	} else {
		const TunerStudioCmdPacketHeader* header = reinterpret_cast<TunerStudioCmdPacketHeader*>(data);
		handled = true;

		switch (command) {
		case TS_IO_TEST_COMMAND:
			handleIoTestCommand(tsChannel, TS_CRC, SWAP_UINT16(header->subsystem), SWAP_UINT16(header->index));
			break;
		default:
			/* noone of simple commands */
			handled = false;
		}
	}

	/* check if we can extract page, offset and count */
	if (incomingPacketSize < sizeof(TunerStudioDataPacketHeader)) {
		sendErrorCode(tsChannel, TS_RESPONSE_UNDERRUN);
		tunerStudioError(tsChannel, "ERROR: underrun");
		return false;
	} else {
		const TunerStudioDataPacketHeader* header = reinterpret_cast<TunerStudioDataPacketHeader*>(data);

		switch(command)
		{
		case TS_OUTPUT_COMMAND:
			tsState.outputChannelsCommandCounter++;
			cmdOutputChannels(tsChannel, header->offset, header->count);
			break;
		case TS_CHUNK_WRITE_COMMAND:
			if (header->page == 0)
				handleWriteChunkCommand(tsChannel, TS_CRC, header->offset, header->count, data + sizeof(TunerStudioDataPacketHeader));
			else
				handleScatterListWriteCommand(tsChannel, header->offset, header->count, data + sizeof(TunerStudioDataPacketHeader));
			break;
		case TS_CRC_CHECK_COMMAND:
			if (header->page == 0)
				handleCrc32Check(tsChannel, TS_CRC, header->offset, header->count);
			else
				handleScatterListCrc32Check(tsChannel, header->offset, header->count);
			break;
		case TS_READ_COMMAND:
			if (header->page == 0)
				handlePageReadCommand(tsChannel, TS_CRC, header->offset, header->count);
			else
				handleScatterListReadCommand(tsChannel, header->offset, header->count);
			break;
		default:
			sendErrorCode(tsChannel, TS_RESPONSE_UNRECOGNIZED_COMMAND);
			tunerStudioError(tsChannel, "ERROR: ignoring unexpected command");
			return false;
		}
	}

	return true;
}

void startTunerStudioConnectivity(void) {
	/* just reset debug counters */
	/* actually already zeroed */
	memset(&tsState, 0, sizeof(tsState));
}
