#!/bin/bash

help()
{
	echo ""
	echo "Usage:"
	echo " can_update.sh board [interface]"
	echo " Default interface is can0"
	exit -1;
}

if [ ! -d tools ]; then
	echo "Please run this script from firmware directory, not tools!"
	help
fi

if [ -z $1 ]; then
	echo "Please provide board name as argunment"
	help
fi

IFACE=can0

if [ ! -z $1 ]; then
	IFACE=$2
fi

if [ ! -d /sys/class/net/$IFACE ]; then
	echo "Interface $IFACE is not exist"
	help
fi

if [ $(</sys/class/net/$IFACE/operstate) != up ]; then
	echo "CAN interface $IFACE is not UP"
	echo " You may want to bring up $IFACE first"
	echo " ip link set $IFACE type can bitrate 500000 ; ifconfig $IFACE up"
	help
fi

FW_FILE=deliver/$1/wideband_update.srec

if [ ! -f $FW_FILE ]; then
	echo "FW_FILE $FW_FILE not found"
	help
fi

# This script will try to flash/update RusEFI part of firmware over can interface.
#
# You may want to bring can interface up first. For can0:
# sudo ip link set can0 type can bitrate 500000 ; sudo ifconfig can0 up
# Sometimes (after communication errors) you may like to restart interface with:
# make sure that you have termination resistor
# sudo ifconfig can0 down; sudo ip link set can0 type can bitrate 500000 ; sudo ifconfig can0 up

#echo This script assumes that you are using $IFACE interface and it is ready, otherwise read comments inside
#echo This script assumes that you have BootCommander somewhere in your PATH, otherwise read comments inside
#BootCommander -t=xcp_can -d=$IFACE $FW_FILE

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
	help
fi

ext/openblt/Host/BootCommander -t=xcp_can -d=$IFACE $FW_FILE
