#!/bin/bash

set -e

# back out to the root
cd ../..

# build app firmware!
make -j12 BOARD=f1_dave ECHO_UART=TRUE
