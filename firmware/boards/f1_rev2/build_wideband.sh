#!/bin/bash

set -e

cd openblt

# build bootloader
make -j12 BOARD=f1_rev2

# back out to the root
cd ../../..

export EXTRA_PARAMS="-DECHO_UART=TRUE"

# build app firmware!
# set optimization level to 0 until ADC issue is fixed for GD32
make -j12 BOARD=f1_rev2 USE_OPT="-O0 -ggdb -fomit-frame-pointer -falign-functions=16 -fsingle-precision-constant"
