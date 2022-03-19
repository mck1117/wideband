#!/bin/bash

set -e

# back out to the root
cd ../..

export EXTRA_PARAMS="-DECHO_UART=TRUE"

# build app firmware!
make -j12 BOARD=f1_dave
