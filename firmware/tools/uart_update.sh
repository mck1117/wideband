#!/bin/bash

FW_FILE=deliver/$1/wideband_update.srec

# This script will try to flash/update RusEFI part of firmware over UART interface.
#
n
# project root
cd ..

#echo This script assumes that you have BootCommander somewhere in your PATH, otherwise read comments inside
#BootCommander -t=xcp_rs232 -d=/dev/ttyUSB0 -b=115200 $FW_FILE

# OR
# You can build it from sources with:
# (cd ext/openblt/Host/Source/LibOpenBLT/ ; mkdir build ; cd build ; cmake .. ; make -j )
# and
# (cd ext/openblt/Host/Source/BootCommander/ ; mkdir build ; cd build ; cmake .. ; make -j )
# And run:

ext/openblt/Host/BootCommander -t=xcp_rs232 -d=/dev/ttyUSB0 -b=115200 $FW_FILE
