/*
 * STM32F103xB memory setup.
 */

/* OpenBLT code */
_OpenBLT_Flash_Size = DEFINED(USE_BOOTLOADER) ? 8k : 0;

/* OpenBLT <-> main FW shared area same area used to pass some data between restarts
 * Now always enabled */
Shared_Params_Size = 16;

MEMORY
{
    /* empty section to keep linker happy */
    blflash (rx) : org = 0x08000000, len = 0
    appflash (rx) : org = (0x08000000 + _OpenBLT_Flash_Size), len = (256k - _OpenBLT_Flash_Size - 8k)
    configflash (rx) : org = (0x08000000 + 256k - 8k), len = 8k
    flash1 (rx) : org = 0x00000000, len = 0
    flash2 (rx) : org = 0x00000000, len = 0
    flash3 (rx) : org = 0x00000000, len = 0
    flash4 (rx) : org = 0x00000000, len = 0
    flash5 (rx) : org = 0x00000000, len = 0
    flash6 (rx) : org = 0x00000000, len = 0
    flash7 (rx) : org = 0x00000000, len = 0
    shared (wx) : org = 0x20000000, len = Shared_Params_Size
    ram0   (wx) : org = 0x20000000 + Shared_Params_Size, len = 48k - Shared_Params_Size
    ram1   (wx) : org = 0x00000000, len = 0
    ram2   (wx) : org = 0x00000000, len = 0
    ram3   (wx) : org = 0x00000000, len = 0
    ram4   (wx) : org = 0x00000000, len = 0
    ram5   (wx) : org = 0x00000000, len = 0
    ram6   (wx) : org = 0x00000000, len = 0
    ram7   (wx) : org = 0x00000000, len = 0
}

__configflash__start__ = ORIGIN(configflash);
