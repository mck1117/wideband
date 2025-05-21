#include "ch.h"
#include "hal.h"

#include "port_shared.h"
#include "flash.h"
#include "io_pins.h"
#include "../../for_rusefi/wideband_can.h"

#include <cstring>
#include <rusefi/crc.h>

// These are defined in the linker script
extern uint32_t __appflash_start__[64];
extern uint32_t __appflash_size__;
extern uint32_t __ram_vectors_start__[64];
extern uint32_t __ram_vectors_size__;

#define SWAP_UINT32(x) ((((x) >> 24) & 0xff) | (((x) << 8) & 0xff0000) | (((x) >> 8) & 0xff00) | (((x) << 24) & 0xff000000))

bool isAppValid() {
    const uint32_t* appFlash = 
    __appflash_start__;

    int appSize = 25600;

    uint32_t expectedCrc = appFlash[appSize / 4 - 1];
    uint32_t actualCrc = SWAP_UINT32(crc32(reinterpret_cast<const uint8_t*>(appFlash), appSize - 4));

    return actualCrc == expectedCrc;
}

__attribute__((noreturn))
void boot_app() {
    // Goodbye, ChibiOS
    chSysDisable();

    // Reset peripherals we might have used
    rccDisableCAN1();

    const uint32_t* appFlash = __appflash_start__;

    // The reset vector is at offset 4 (second uint32)
    uint32_t reset_vector = appFlash[1];

#ifdef STM32F0XX
    // copy vector table to sram
    // TODO: use __ram_vectors_size__
    memcpy(reinterpret_cast<char*>(&__ram_vectors_start__), appFlash, 256);

    // M0 core version, newer cores do same thing a bit nicer
    // switch to use vectors in ram
    SYSCFG->CFGR1 |= 3;
#endif

    // TODO: is this necessary?
    //uint32_t app_msp = appLocation[0];
    //__set_MSP(app_msp);

    typedef void (*ResetVectorFunction)(void);
    ((ResetVectorFunction)reset_vector)();

    while(1);
}

uintptr_t appFlashAddr = (uintptr_t)__appflash_start__;

void EraseAppPages()
{
    uintptr_t blSize = (uintptr_t)(appFlashAddr - 0x08000000);
    size_t pageIdx = blSize / 1024;

    // size_t appSizeKb = __appflash_size__ / 1024;
    size_t appSizeKb = 25;

    for (size_t i = 0; i < appSizeKb; i++)
    {
        Flash::ErasePage(pageIdx);
        pageIdx++;
    }
}


void WaitForBootloaderCmd()
{
    while(true)
    {
        CANRxFrame frame;
        msg_t result = canReceiveTimeout(&CAND1, CAN_ANY_MAILBOX, &frame, TIME_INFINITE);

        // Ignore non-ok results
        if (result != MSG_OK) 
        {
            continue;
        }

        // Ignore std frames, only listen to ext
        if (frame.IDE != CAN_IDE_EXT)
        {
            continue;
        }

        // if we got a bootloader-init message, here we go!
        if (frame.DLC == 0 && frame.EID == WB_BL_ENTER)
        {
            return;
        }
    }
}

void sendAck()
{
    CANTxFrame frame;

    frame.IDE = CAN_IDE_EXT;
    frame.EID = WB_ACK;   // ascii "rus"
    frame.RTR = CAN_RTR_DATA;
    frame.DLC = 0;

    canTransmitTimeout(&CAND1, CAN_ANY_MAILBOX, &frame, TIME_INFINITE);
}

void sendNak()
{
    // TODO: implement
}

bool bootloaderBusy = false;

void RunBootloaderLoop()
{
    // First ack that the bootloader is alive
    sendAck();

    while (true)
    {
        CANRxFrame frame;
        msg_t result = canReceiveTimeout(&CAND1, CAN_ANY_MAILBOX, &frame, TIME_INFINITE);

        // Ignore non-ok results
        if (result != MSG_OK) 
        {
            continue;
        }

        // Ignore std frames, only listen to ext
        if (frame.IDE != CAN_IDE_EXT)
        {
            continue;
        }

        // 29-bit extended ID:
        //  0 xxxy zzzz
        // xx = header, always equals 0xEF
        //  y = opcode
        // zzzz = extra 2 data bytes hidden in the address!

        uint16_t header = frame.EID >> 20;

        // All rusEfi bootloader packets start with 0x0EF, ignore other traffic on the bus
        if (header != WB_BL_HEADER)
        {
            continue;
        }

        uint8_t opcode = (frame.EID >> 16) & 0x0F;
        uint16_t embeddedData = frame.EID & 0xFFFF;

        switch (opcode) {
            case WB_OPCODE_START: // opcode 0 is simply the "enter BL" command, but we're already here.  Send an ack.
                sendAck();
                break;
            case WB_OPCODE_ERASE: // opcode 1 is "erase app flash"
                // embedded data must be 0x5A5A
                if (embeddedData == WB_ERASE_TAG)
                {
                    EraseAppPages();
                    sendAck();
                }
                else
                {
                    sendNak();
                }

                break;
            case WB_OPCODE_DATA: // opcode 2 is "write flash data"
                // Embedded data is the flash address

                // Don't allow misaligned writes
                if (embeddedData % sizeof(flashdata_t) != 0 || frame.DLC % sizeof(flashdata_t) != 0)
                {
                    sendNak();
                }
                // Don't allow out of bounds writes
                else if (embeddedData >= 26 * 1024)
                {
                    sendNak();
                }
                else
                {
                    Flash::Write(appFlashAddr + embeddedData, &frame.data8[0], frame.DLC);
                    sendAck();
                }

                break;
            case WB_OPCODE_REBOOT: // opcode 3 is "boot app"
                sendAck();

                // Let the message get out
                chThdSleepMilliseconds(100);

                // Clear the flag
                bootloaderBusy = false;
                // Kill this thread
                return;
            default:
                sendNak();
                break;
        }
    }
}

THD_WORKING_AREA(waBootloaderThread, 512);
THD_FUNCTION(BootloaderThread, arg)
{
    (void)arg;

    // turn on CAN
    canStart(&CAND1, &GetCanConfig());

    WaitForBootloaderCmd();

    // We've rx'd a BL command, don't load the app!
    bootloaderBusy = true;

    RunBootloaderLoop();
}

/*
 * Application entry point.
 */
int main(void) {
    halInit();
    chSysInit();

    chThdCreateStatic(waBootloaderThread, sizeof(waBootloaderThread), NORMALPRIO + 1, BootloaderThread, nullptr);

    palSetPadMode(LED_BLUE_PORT, LED_BLUE_PIN, PAL_MODE_OUTPUT_PUSHPULL);

    palSetPadMode(LED_GREEN_PORT, LED_GREEN_PIN, PAL_MODE_OUTPUT_PUSHPULL);
    palTogglePad(LED_GREEN_PORT, LED_GREEN_PIN);

    for (size_t i = 0; i < 20; i++)
    {
        palTogglePad(LED_BLUE_PORT, LED_BLUE_PIN);
        palTogglePad(LED_GREEN_PORT, LED_GREEN_PIN);
        chThdSleepMilliseconds(40);
    }

    // Block until booting the app is allowed and CRC matches
    while (bootloaderBusy || !isAppValid())
    {
        palTogglePad(LED_BLUE_PORT, LED_BLUE_PIN);
        palTogglePad(LED_GREEN_PORT, LED_GREEN_PIN);
        chThdSleepMilliseconds(200);
    }

    boot_app();
}
