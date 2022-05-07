#include "flash.h"
#include "hal.h"


/**
 * @brief Wait for the flash operation to finish.
 */
static void flashWaitWhileBusy()
{
    while (FLASH->SR & FLASH_SR_BSY) ;
}

static void flashUnlock() {
    /* Check if unlock is really needed */
    if (!(FLASH->CR & FLASH_CR_LOCK))
        return;

    /* Write magic unlock sequence */
    FLASH->KEYR = 0x45670123;
    FLASH->KEYR = 0xCDEF89AB;
}

static void flashLock() {
    FLASH->CR |= FLASH_CR_LOCK;
}

void Flash::ErasePage(uint8_t pageIdx) {
    flashUnlock();

    // Wait for flash to be not busy
    flashWaitWhileBusy();

    // page erase mode
    FLASH->CR |= FLASH_CR_PER;

    // Set page number
    FLASH->AR = pageIdx * 1024;

    // Start the erase operation
    FLASH->CR |= FLASH_CR_STRT;

    // Must wait at least one cycle before reading FLASH_SR_BSY
    __asm__ __volatile__("nop");

    // Wait for flash to be not busy
    flashWaitWhileBusy();

    // clear page erase bit
    FLASH->CR &= ~FLASH_CR_PER;

    // Relock flash
    flashLock();
}

static void flashWriteData(flashaddr_t address, const flashdata_t data) {
    /* Enter flash programming mode */
    FLASH->CR |= FLASH_CR_PG;

    /* Write the data */
    *(flashdata_t*) address = data;

    /* Wait for completion */
    flashWaitWhileBusy();

    /* Exit flash programming mode */
    FLASH->CR &= ~FLASH_CR_PG;
}

void Flash::Write(flashaddr_t address, const uint8_t* buffer, size_t size) {
    /* Unlock flash for write access */
    flashUnlock();

    /* Wait for any busy flags */
    flashWaitWhileBusy();

    //Copy data directly from buffer's data to flash
    while (size >= sizeof(flashdata_t)) {
        flashWriteData(address, *(const flashdata_t*) buffer);
        address += sizeof(flashdata_t);
        buffer += sizeof(flashdata_t);
        size -= sizeof(flashdata_t);
    }

    /* Lock flash again */
    flashLock();
}
