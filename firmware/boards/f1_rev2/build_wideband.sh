#!/bin/bash

set -e

cd openblt

# build bootloader
make -j12 BOARD=f1_rev2

# back out to the root
cd ../../..

export EXTRA_PARAMS="-DECHO_UART=TRUE"

# build app firmware!
make -j12 BOARD=f1_rev2
