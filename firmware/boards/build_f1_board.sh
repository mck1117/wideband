#!/bin/bash

set -eo pipefail

if [ ! "$USE_OPENBLT" ]; then
  USE_OPENBLT=no
fi

if [ $USE_OPENBLT = "yes" ]; then
  cd openblt

  echo ""
  echo "Building bootloader"
  make clean
  make -j12 BOARD=${BOARD} || exit 1

  # back to board dir
  cd ..
fi

cd ../..

echo ""
echo "Build application"
# make clean
make -j12 BOARD=${BOARD} || exit 1

DELIVER_DIR=deliver/${BOARD}
mkdir -p ${DELIVER_DIR}
rm -f ${DELIVER_DIR}/*

if uname | grep "NT"; then
  HEX2DFU=./ext/encedo_hex2dfu/hex2dfu.exe
  SREC_CAT=srec_cat.exe
else
  HEX2DFU=./ext/encedo_hex2dfu/hex2dfu.bin
  chmod u+x $HEX2DFU
  SREC_CAT=srec_cat
fi

echo ""
echo "Creating deliveries:"

if [ $USE_OPENBLT = "yes" ]; then
  echo "Srec for CAN update"
  cp -v build/wideband.srec ${DELIVER_DIR}/wideband_update.srec

  echo ""
  echo "Invoking hex2dfu for incremental Wideband image (for DFU util)"
  $HEX2DFU -i build/wideband.hex -C 0x1C -o ${DELIVER_DIR}/wideband_update.dfu

  echo ""
  echo "Invoking hex2dfu for OpenBLT (for DFU util)"
  $HEX2DFU -i boards/${BOARD}/openblt/bin/openblt_${BOARD}.hex -o ${DELIVER_DIR}/openblt.dfu

  echo ""
  echo "OpenBLT bin (for DFU another util)"
  cp -v boards/${BOARD}/openblt/bin/openblt_${BOARD}.bin ${DELIVER_DIR}/openblt.bin

  echo ""
  echo "OpenBLT elf for debugging"
  cp -v boards/${BOARD}/openblt/bin/openblt_${BOARD}.elf ${DELIVER_DIR}/openblt.elf

  OPENBLT_HEX=boards/${BOARD}/openblt/bin/openblt_${BOARD}.hex
  echo ""
  echo "Invoking hex2dfu for composite OpenBLT+Wideband image (for DFU util)"
  $HEX2DFU -i ${OPENBLT_HEX} -i build/wideband.hex -C 0x1C -o ${DELIVER_DIR}/wideband.dfu -b ${DELIVER_DIR}/wideband.bin
  echo "Creating composite hex file"
  $SREC_CAT build/wideband.hex -binary -offset 0x20000000 -o ${DELIVER_DIR}/wideband.hex -Intel
else
  echo "Bin for raw flashing"
  cp build/wideband.bin ${DELIVER_DIR}

  cp build/wideband.hex ${DELIVER_DIR}

  echo "elf for debugging"
  cp build/wideband.elf ${DELIVER_DIR}

  echo "Invoking hex2dfu for DFU file"
  $HEX2DFU -i build/wideband.hex -o ${DELIVER_DIR}/wideband.dfu
fi

echo ""
echo "${DELIVER_DIR} folder content:"
ls -l ${DELIVER_DIR}
