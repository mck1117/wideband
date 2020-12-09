#include "ch.h"
#include "hal.h"

#include "flash.h"

#include <cstring>

// These are defined in the linker script
extern uint32_t __appflash_start__;
extern uint32_t __appflash_size__;
extern uint32_t __ram_vectors_start__;
extern uint32_t __ram_vectors_size__;

__attribute__((noreturn))
void boot_app() {
    // Goodbye, ChibiOS
    chSysDisable();

    // Reset peripherals we might have used
    rccDisableCAN1();

    const uint32_t* appFlash = &__appflash_start__;

    // copy vector table to sram
    // TODO: use __ram_vectors_size__
    memcpy(reinterpret_cast<char*>(&__ram_vectors_start__), appFlash, 256);

    // The reset vector is at offset 4 (second uint32)
    uint32_t reset_vector = appFlash[1];

    // switch to use vectors in ram
    SYSCFG->CFGR1 |= 3;

    // TODO: is this necessary?
    //uint32_t app_msp = appLocation[0];
    //__set_MSP(app_msp);

    typedef void (*ResetVectorFunction)(void);
    ((ResetVectorFunction)reset_vector)();
}

void EraseAppPages()
{
    uint32_t appFlashAddr = (uint32_t)&__appflash_start__;
    uintptr_t blSize = (uintptr_t)(appFlashAddr - 0x08000000);
    size_t pageIdx = blSize / 1024;

    size_t appSizeKb = __appflash_size__ / 1024;

    for (int i = 0; i <= appSizeKb; i++)
    {
        Flash::ErasePage(pageIdx);
        pageIdx++;
    }
}

/*
 * Application entry point.
 */
int main(void) {
    halInit();
    chSysInit();

    palSetPadMode(GPIOB, 3, PAL_MODE_OUTPUT_PUSHPULL);

    for (size_t i = 0; i < 20; i++)
    {
        palTogglePad(GPIOB, 3);
        chThdSleepMilliseconds(40);
    }

    //EraseAppPages();

    boot_app();
}
