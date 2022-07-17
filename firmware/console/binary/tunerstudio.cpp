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

#include <rusefi/crc.h>

#ifndef EFI_BLUETOOTH_SETUP
	#define EFI_BLUETOOTH_SETUP 0
#endif

#ifndef EFI_TEXT_LOGGING
	#define EFI_TEXT_LOGGING 0
#endif

static void printErrorCounters() {
//	efiPrintf("TunerStudio size=%d / total=%d / errors=%d / H=%d / O=%d / P=%d / B=%d",
//			sizeof(engine->outputChannels), tsState.totalCounter, tsState.errorCounter, tsState.queryCommandCounter,
//			tsState.outputChannelsCommandCounter, tsState.readPageCommandsCounter, tsState.burnCommandCounter);
//	efiPrintf("TunerStudio W=%d / C=%d / P=%d", tsState.writeValueCommandCounter,
//			tsState.writeChunkCommandCounter, tsState.pageCommandCounter);
}

/* TunerStudio repeats connection attempts at ~1Hz rate.
 * So first char receive timeout should be less than 1s */
#define TS_COMMUNICATION_TIMEOUT	TIME_MS2I(500)	//0.5 Sec

void tunerStudioDebug(TsChannelBase* tsChannel, const char *msg) {
	(void)tsChannel;
	(void)msg;
//#if EFI_TUNER_STUDIO_VERBOSE
//	efiPrintf("%s: %s", tsChannel->name, msg);
//#endif /* EFI_TUNER_STUDIO_VERBOSE */
}

uint8_t* getWorkingPageAddr() {
	return NULL; // todo
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

void TunerStudio::handlePageSelectCommand(TsChannelBase *tsChannel, ts_response_format_e mode) {
	tsState.pageCommandCounter++;

	sendOkResponse(tsChannel, mode);
}

#if 0
const void * getStructAddr(live_data_e structId) {
	switch (structId) {
	case LDS_output_channels:
		return reinterpret_cast<const uint8_t*>(&engine->outputChannels);

	case LDS_high_pressure_fuel_pump:
#if EFI_HPFP
		return static_cast<high_pressure_fuel_pump_s*>(&engine->module<HpfpController>().unmock());
#else
		return nullptr; // explicit null to confirm that this struct is handled
#endif // EFI_HPFP

	case LDS_launch_control_state:
#if EFI_LAUNCH_CONTROL
		return static_cast<launch_control_state_s*>(&engine->launchController);
#else
		return nullptr; // explicit null to confirm that this struct is handled
#endif // EFI_LAUNCH_CONTROL

	case LDS_injector_model:
		return static_cast<injector_model_s*>(&engine->module<InjectorModel>().unmock());

	case LDS_boost_control:
#if EFI_BOOST_CONTROL
		return static_cast<boost_control_s*>(&engine->boostController);
#else
		return nullptr; // explicit null to confirm that this struct is handled
#endif // EFI_BOOST_CONTROL

	case LDS_ac_control:
		return static_cast<ac_control_s*>(&engine->module<AcController>().unmock());
	case LDS_fan_control:
		return static_cast<fan_control_s*>(&engine->fan1);
	case LDS_fuel_pump_control:
		return static_cast<fuel_pump_control_s*>(&engine->module<FuelPumpController>().unmock());
	case LDS_main_relay:
		return static_cast<main_relay_s*>(&engine->module<MainRelayController>().unmock());
	case LDS_engine_state:
		return static_cast<engine_state_s*>(&engine->engineState);
	case LDS_tps_accel_state:
		return static_cast<tps_accel_state_s*>(&engine->tpsAccelEnrichment);
	case LDS_trigger_central:
		return static_cast<trigger_central_s*>(&engine->triggerCentral);
	case LDS_trigger_state:
#if EFI_SHAFT_POSITION_INPUT
		return static_cast<trigger_state_s*>(&engine->triggerCentral.triggerState);
#else
		return nullptr;
#endif // EFI_SHAFT_POSITION_INPUT
	case LDS_wall_fuel_state:
		return static_cast<wall_fuel_state_s*>(&engine->injectionEvents.elements[0].wallFuel);
	case LDS_idle_state:
		return static_cast<idle_state_s*>(&engine->module<IdleController>().unmock());
	case LDS_ignition_state:
		return static_cast<ignition_state_s*>(&engine->ignitionState);
	case LDS_electronic_throttle:
		// todo: figure out how to handle two units?
		return nullptr;

//#if EFI_ELECTRONIC_THROTTLE_BODY
//	case LDS_ETB_PID:
//		return engine->etbControllers[0]->getPidState();
//#endif /* EFI_ELECTRONIC_THROTTLE_BODY */
//
//#ifndef EFI_IDLE_CONTROL
//	case LDS_IDLE_PID:
//		return static_cast<pid_state_s*>(getIdlePid());
//#endif /* EFI_IDLE_CONTROL */
	default:
// huh?		firmwareError(OBD_PCM_Processor_Fault, "getStructAddr not implemented for %d", (int)structId);
		return nullptr;
	}
}

/**
 * Read internal structure for Live Doc
 * This is somewhat similar to read page and somewhat similar to read outputs
 * We can later consider combining this functionality
 */
static void handleGetStructContent(TsChannelBase* tsChannel, int structId, int size) {
	tsState.readPageCommandsCounter++;

	const void *addr = getStructAddr((live_data_e)structId);
	if (addr == nullptr) {
		// todo: add warning code - unexpected structId
		return;
	}
	tsChannel->sendResponse(TS_CRC, (const uint8_t *)addr, size);
}
#endif

bool validateOffsetCount(size_t offset, size_t count, TsChannelBase* tsChannel);

//extern bool rebootForPresetPending;

/**
 * This command is needed to make the whole transfer a bit faster
 * @note See also handleWriteValueCommand
 */
void TunerStudio::handleWriteChunkCommand(TsChannelBase* tsChannel, ts_response_format_e mode, uint16_t offset, uint16_t count,
		void *content) {
	(void)content;

	tsState.writeChunkCommandCounter++;

	if (validateOffsetCount(offset, count, tsChannel)) {
		return;
	}

	sendOkResponse(tsChannel, mode);
}

void TunerStudio::handleCrc32Check(TsChannelBase *tsChannel, ts_response_format_e mode, uint16_t offset, uint16_t count) {
	tsState.crc32CheckCommandCounter++;

	// Ensure we are reading from in bounds
	if (validateOffsetCount(offset, count, tsChannel)) {
		return;
	}

	const uint8_t* start = getWorkingPageAddr() + offset;

	uint32_t crc = SWAP_UINT32(crc32(start, count));
	tsChannel->sendResponse(mode, (const uint8_t *) &crc, 4);
}

/**
 * 'Write' command receives a single value at a given offset
 * @note Writing values one by one is pretty slow
 */
void TunerStudio::handleWriteValueCommand(TsChannelBase* tsChannel, ts_response_format_e mode, uint16_t offset, uint8_t value) {
	(void)tsChannel;
	(void)mode;
	(void)value;

	tsState.writeValueCommandCounter++;

	tunerStudioDebug(tsChannel, "got W (Write)"); // we can get a lot of these

	if (validateOffsetCount(offset, 1, tsChannel)) {
		return;
	}
}

void TunerStudio::handlePageReadCommand(TsChannelBase* tsChannel, ts_response_format_e mode, uint16_t offset, uint16_t count) {
	tsState.readPageCommandsCounter++;

	if (validateOffsetCount(offset, count, tsChannel)) {
		return;
	}

	const uint8_t* addr = getWorkingPageAddr() + offset;
	tsChannel->sendResponse(mode, addr, count);
}

void requestBurn(void) {
	//SaveConfiguration();
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

	//SaveConfiguration();

	sendResponseCode(mode, tsChannel, TS_RESPONSE_BURN_OK);
}

static bool isKnownCommand(char command) {
	return command == TS_HELLO_COMMAND || command == TS_READ_COMMAND || command == TS_OUTPUT_COMMAND
			|| command == TS_PAGE_COMMAND || command == TS_BURN_COMMAND || command == TS_SINGLE_WRITE_COMMAND
			|| command == TS_CHUNK_WRITE_COMMAND
			|| command == TS_CRC_CHECK_COMMAND
			|| command == TS_GET_FIRMWARE_VERSION
			|| command == TS_GET_CONFIG_ERROR;
}

/**
 * rusEfi own test command
 */

#define VCS_VERSION "unknown"

static void handleTestCommand(TsChannelBase* tsChannel) {
	tsState.testCommandCounter++;
	char testOutputBuffer[64];
	/**
	 * this is NOT a standard TunerStudio command, this is my own
	 * extension of the protocol to simplify troubleshooting
	 */
	tunerStudioDebug(tsChannel, "got T (Test)");
	tsChannel->write((const uint8_t*)VCS_VERSION, sizeof(VCS_VERSION));

	chsnprintf(testOutputBuffer, sizeof(testOutputBuffer),  __DATE__ "\r\n");
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
	auto channel = setupChannel();

	// No channel configured for this thread, cancel.
	if (!channel || !channel->isConfigured()) {
		return;
	}

	// Until the end of time, process incoming messages.
	while (true) {
		if (tsProcessOne(channel) == 0) {
			//onDataArrived(true);
		} else {
			//onDataArrived(false);
		}
	}
}

tunerstudio_counters_s tsState;

void tunerStudioError(TsChannelBase* tsChannel, const char *msg) {
	tunerStudioDebug(tsChannel, msg);
	printErrorCounters();
	tsState.errorCounter++;
}

static void handleGetVersion(TsChannelBase* tsChannel) {
	char versionBuffer[32];
	//chsnprintf(versionBuffer, sizeof(versionBuffer), "rusEFI v%d@%s", getRusEfiVersion(), VCS_VERSION);
	chsnprintf(versionBuffer, sizeof(versionBuffer), "rusEFI Wideband Rev2");
	tsChannel->sendResponse(TS_CRC, (const uint8_t *) versionBuffer, strlen(versionBuffer) + 1);
}

#if EFI_TEXT_LOGGING
static void handleGetText(TsChannelBase* tsChannel) {
	tsState.textCommandCounter++;

	printOverallStatus();

	size_t outputSize;
	const char* output = swapOutputBuffers(&outputSize);

	tsChannel->writeCrcPacket(TS_RESPONSE_COMMAND_OK, reinterpret_cast<const uint8_t*>(output), outputSize, true);
}
#endif // EFI_TEXT_LOGGING

#if 0
extern CommandHandler console_line_callback;
void TunerStudio::handleExecuteCommand(TsChannelBase* tsChannel, char *data, int incomingPacketSize) {
	data[incomingPacketSize] = 0;
	char *trimmed = efiTrim(data);

	(console_line_callback)(trimmed);

	tsChannel->writeCrcPacket(TS_RESPONSE_COMMAND_OK, nullptr, 0);
}
#endif

int TunerStudio::handleCrcCommand(TsChannelBase* tsChannel, char *data, int incomingPacketSize) {
	(void)incomingPacketSize;

	char command = data[0];
	data++;

	const uint16_t* data16 = reinterpret_cast<uint16_t*>(data);

	uint16_t offset = data16[0];
	uint16_t count = data16[1];

	switch(command)
	{
	case TS_OUTPUT_COMMAND:
		cmdOutputChannels(tsChannel, offset, count);
		break;
	case TS_HELLO_COMMAND:
		tunerStudioDebug(tsChannel, "got Query command");
		handleQueryCommand(tsChannel, TS_CRC);
		break;
	case TS_GET_FIRMWARE_VERSION:
		handleGetVersion(tsChannel);
		break;
#if EFI_TEXT_LOGGING
	case TS_GET_TEXT:
		handleGetText(tsChannel);
		break;
#endif // EFI_TEXT_LOGGING
#if 0
	case TS_EXECUTE:
		handleExecuteCommand(tsChannel, data, incomingPacketSize - 1);
		break;
#endif
	case TS_PAGE_COMMAND:
		handlePageSelectCommand(tsChannel, TS_CRC);
		break;
//	case TS_GET_STRUCT:
//		handleGetStructContent(tsChannel, offset, count);
//		break;
	case TS_CHUNK_WRITE_COMMAND:
		handleWriteChunkCommand(tsChannel, TS_CRC, offset, count, data + sizeof(TunerStudioWriteChunkRequest));
		break;
	case TS_SINGLE_WRITE_COMMAND:
		{
			uint8_t value = data[4];
			handleWriteValueCommand(tsChannel, TS_CRC, offset, value);
		}
		break;
	case TS_CRC_CHECK_COMMAND:
		handleCrc32Check(tsChannel, TS_CRC, offset, count);
		break;
	case TS_BURN_COMMAND:
		handleBurnCommand(tsChannel, TS_CRC);
		break;
	case TS_READ_COMMAND:
		handlePageReadCommand(tsChannel, TS_CRC, offset, count);
		break;
	case TS_TEST_COMMAND:
		[[fallthrough]];
	case 'T':
		handleTestCommand(tsChannel);
		break;
#if 0
	case TS_IO_TEST_COMMAND:
		{
			uint16_t subsystem = SWAP_UINT16(data16[0]);
			uint16_t index = SWAP_UINT16(data16[1]);

			if (engineConfiguration->debugMode == DBG_BENCH_TEST) {
				engine->outputChannels.debugIntField1++;
				engine->outputChannels.debugIntField2 = subsystem;
				engine->outputChannels.debugIntField3 = index;
			}

			executeTSCommand(subsystem, index);
			sendOkResponse(tsChannel, TS_CRC);
		}
		break;
	case TS_GET_CONFIG_ERROR: {
		const char* configError = getCriticalErrorMessage();
#if HW_CHECK_MODE
		// analog input errors are returned as firmware error in QC mode
		if (!hasFirmwareError()) {
			strcpy((char*)configError, "FACTORY_MODE_PLEASE_CONTACT_SUPPORT");
		}
#endif // HW_CHECK_MODE
		tsChannel->sendResponse(TS_CRC, reinterpret_cast<const uint8_t*>(configError), strlen(configError), true);
		break;
	}
#endif
	default:
		sendErrorCode(tsChannel, TS_RESPONSE_UNRECOGNIZED_COMMAND);
		tunerStudioError(tsChannel, "ERROR: ignoring unexpected command");
		return false;
	}

	return true;
}

void startTunerStudioConnectivity(void) {
	// Assert tune & output channel struct sizes
//	static_assert(sizeof(persistent_config_s) == TOTAL_CONFIG_SIZE, "TS datapage size mismatch");
// useful trick if you need to know how far off is the static_assert
//	char (*__kaboom)[sizeof(persistent_config_s)] = 1;
// another useful trick
//  static_assert(offsetof (engine_configuration_s,HD44780_e) == 700);

	memset(&tsState, 0, sizeof(tsState));
	
#if EFI_BLUETOOTH_SETUP
	// module initialization start (it waits for disconnect and then communicates to the module)
	// Usage:   "bluetooth_hc06 <baud> <name> <pincode>"
	// Example: "bluetooth_hc06 38400 rusefi 1234"
	// bluetooth_jdy 115200 alphax 1234
	addConsoleActionSSS("bluetooth_hc05", [](const char *baudRate, const char *name, const char *pinCode) {
		bluetoothStart(BLUETOOTH_HC_05, baudRate, name, pinCode);
	});
	addConsoleActionSSS("bluetooth_hc06", [](const char *baudRate, const char *name, const char *pinCode) {
		bluetoothStart(BLUETOOTH_HC_06, baudRate, name, pinCode);
	});
	addConsoleActionSSS("bluetooth_bk", [](const char *baudRate, const char *name, const char *pinCode) {
		bluetoothStart(BLUETOOTH_BK3231, baudRate, name, pinCode);
	});
	addConsoleActionSSS("bluetooth_jdy", [](const char *baudRate, const char *name, const char *pinCode) {
		bluetoothStart(BLUETOOTH_JDY_3x, baudRate, name, pinCode);
	});
	addConsoleAction("bluetooth_cancel", bluetoothCancel);
#endif /* EFI_BLUETOOTH_SETUP */
}