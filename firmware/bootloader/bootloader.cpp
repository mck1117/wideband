#include "ch.h"
#include "hal.h"

#include <cstring>

// These are defined in the linker script
extern uint32_t __appflash_start__;
extern uint32_t __ram_vectors_start__;
extern int __ram_vectors_size__;

void memcpy_wrapped(char* dst, const char* src, int size)
{
    //memcpy(dst, src, size);
    //return;

    for (size_t i = 0; i < size; i++)
    {
        char c = src[i];
        dst[i] = c;
    }
}

__attribute__((noreturn))
void jump_to(uint32_t address) {
    __asm__ __volatile__ ("bx r0");
}

__attribute__((noreturn))
void boot_app() {
    // Goodbye, ChibiOS
    chSysDisable();

    // Reset peripherals we might have used
    rccDisableCAN1();

    volatile uint32_t* appFlash = &__appflash_start__;

    // copy vector table to sram
    // TODO: use __ram_vectors_size__
    memcpy_wrapped(reinterpret_cast<char*>(&__ram_vectors_start__), reinterpret_cast<char*>(&__appflash_start__), 256);

    // The reset vector is at offset 4 (second uint32)
    uint32_t reset_vector = appFlash[1];

    // switch to use vectors in ram
    SYSCFG->CFGR1 |= 3;

    // TODO: is this necessary?
    //uint32_t app_msp = appLocation[0];
    //__set_MSP(app_msp);

    jump_to(reset_vector);
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
        chThdSleepMilliseconds(50);
    }

    boot_app();
}
