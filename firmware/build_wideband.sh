#!/bin/bash

# first build the bootloader
cd bootloader
./build_bootloader.sh
cd ..

# delete the elf to force a re-link (it might not pick up the bootloader otherwise)
rm -r build/
rm ../for_rusefi/wideband_image.h

make -j12

# Copy the bin without the bootloader (the image consumed by rusEfi has no bootloader on it)
dd if=build/wideband.bin of=build/wideband_noboot.bin skip=6144 bs=1

# Convert to a header file, and tack "static const " on the front of it
xxd -i build/wideband_noboot.bin \
    | cat <(echo -n "static const ") - \
    > ../for_rusefi/wideband_image.h
