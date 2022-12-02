#!/bin/bash

if [ ! -d tools ]; then
	echo "Please run this script from firmware directory, not tools!"
	exit -1
fi

if [ -z $1 ]; then
	echo "Please provide board name as argunment"
	exit -1
fi

FW_FILE=deliver/$1/wideband_update.srec

if [ ! -f $FW_FILE ]; then
	echo "FW_FILE $FW_FILE not found"
	exit -1
fi

# This script will try to flash/update RusEFI part of firmware over UART interface.

#echo This script assumes that you have BootCommander somewhere in your PATH, otherwise read comments inside
#BootCommander -t=xcp_rs232 -d=/dev/ttyUSB0 -b=115200 $FW_FILE

# OR
# You can build it from sources with:
# (cd ext/openblt/Host/Source/LibOpenBLT/ ; mkdir build ; cd build ; cmake .. ; make -j )
# and
# (cd ext/openblt/Host/Source/BootCommander/ ; mkdir build ; cd build ; cmake .. ; make -j )
# And run:

if [ ! -f ext/openblt/Host/BootCommander ]; then
	echo "Please build BootCommander first!"
	echo "Run: (cd ext/openblt/Host/Source/LibOpenBLT/ ; mkdir build ; cd build ; cmake .. ; make -j )"
	echo "Run: (cd ext/openblt/Host/Source/BootCommander/ ; mkdir build ; cd build ; cmake .. ; make -j )"
fi

ext/openblt/Host/BootCommander -t=xcp_rs232 -d=/dev/ttyUSB0 -b=115200 $FW_FILE
