/*
 * STM32F042x6 memory setup.
 */

blsize = DEFINED(USE_BOOTLOADER) ? 6k : 0;

MEMORY
{
    blflash (rx) : org = 0x08000000, len = blsize
    appflash (rx) : org = 0x08000000 + blsize, len = 32k - blsize - 1k
    configflash (rx) : org = 0x08000000 + 32k - 1k, len = 1k
    flash1 (rx) : org = 0x00000000, len = 0
    flash2 (rx) : org = 0x00000000, len = 0
    flash3 (rx) : org = 0x00000000, len = 0
    flash4 (rx) : org = 0x00000000, len = 0
    flash5 (rx) : org = 0x00000000, len = 0
    flash6 (rx) : org = 0x00000000, len = 0
    flash7 (rx) : org = 0x00000000, len = 0
    ram_vectors (wx) : org = 0x20000000, len = 256
    ram0   (wx) : org = 0x20000100, len = 6k - 256
    ram1   (wx) : org = 0x00000000, len = 0
    ram2   (wx) : org = 0x00000000, len = 0
    ram3   (wx) : org = 0x00000000, len = 0
    ram4   (wx) : org = 0x00000000, len = 0
    ram5   (wx) : org = 0x00000000, len = 0
    ram6   (wx) : org = 0x00000000, len = 0
    ram7   (wx) : org = 0x00000000, len = 0
}

__appflash_start__ = ORIGIN(appflash);
__appflash_size__ = LENGTH(appflash);
__configflash__start__ = ORIGIN(configflash);
__ram_vectors_start__ = ORIGIN(ram_vectors);
__ram_vectors_size__ = LENGTH(ram_vectors);
