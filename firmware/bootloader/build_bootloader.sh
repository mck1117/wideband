#!/bin/bash

# clean & build the bootloader
make clean && make -j4

# convert bootloader bin output to a linkable .o file
# first pad the binary out with 0xFF
arm-none-eabi-objcopy -I binary -O binary --gap-fill 0xFF --pad-to 0x1800 build/wideband_bootloader.bin build/wideband_bootloader.padded.bin
# then convert it to a .o file
arm-none-eabi-objcopy -I binary -O elf32-littlearm -B arm --rename-section .data=.bl build/wideband_bootloader.padded.bin build/wideband_bootloader.o
