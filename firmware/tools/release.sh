#!/bin/bash

BOARD=$1
DROP=deliver/$BOARD/drop

echo Preparding $BOARD package in $DROP folder

mkdir -p $DROP

cp tools/readme.md $DROP
cp deliver/$BOARD/wideband.bin $DROP
cp deliver/$BOARD/wideband_update.srec $DROP
mkdir $DROP/bin
cp ext/openblt/Host/BootCommander.exe $DROP/bin
cp ext/openblt/Host/libopenblt.dll $DROP/bin

