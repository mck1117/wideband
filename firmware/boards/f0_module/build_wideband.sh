#!/bin/bash

set -o pipefail

# first build the bootloader
cd bootloader
./build_bootloader.sh

# back out to the root
cd ../../..

BOARD=f0_module
DELIVER_DIR=deliver/${BOARD}

# delete the elf to force a re-link (it might not pick up the bootloader otherwise)
rm -rf build/
rm -f ../for_rusefi/wideband_image.h

mkdir -p ${DELIVER_DIR}
rm -rf ${DELIVER_DIR}/*

# build app firmware!
make -j12 BOARD=${BOARD}


# Copy the bin without the bootloader (the image consumed by rusEfi has no bootloader on it)
dd if=build/wideband.bin of=build/wideband_noboot_no_pad.bin skip=6144 bs=1

# extend the image out to full size (32k (flash) - 6k (bootloader) - 1k (config) - 4 (crc) = 25k - 4 = 0x63FC)
arm-none-eabi-objcopy -I binary -O binary --gap-fill 0xFF --pad-to 0x63FC build/wideband_noboot_no_pad.bin build/wideband_fullsize_nocrc.bin

# compute the crc and write that to a file (in binary)
crc32 build/wideband_fullsize_nocrc.bin | xxd -r -p - > build/wideband_crc.bin
[ $? -eq 0 ] || { echo "crc32 computation failed"; exit 1; }

# Now glue the image and CRC together
cat build/wideband_fullsize_nocrc.bin build/wideband_crc.bin > build/wideband_image.bin

# For good measure build a complete bin that includes the bootloader
cat boards/f0_module/bootloader/build/wideband_bootloader.padded.bin build/wideband_image.bin > ${DELIVER_DIR}/wideband_image_with_bl.bin

# Convert to a header file, and tack "static const " on the front of it
xxd -i build/wideband_image.bin \
    | cat <(echo -n "static const ") - \
    > ${DELIVER_DIR}/wideband_image.h

cp ${DELIVER_DIR}/wideband_image_with_bl.bin ../for_rusefi/
cp ${DELIVER_DIR}/wideband_image.h ../for_rusefi/
