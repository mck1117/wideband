#!/bin/bash

BOARD=$1
FOLDER=rusefi-wideband-$BOARD
DROP=deliver/$BOARD/$FOLDER

set -e

echo Preparding $BOARD package in $DROP folder

rm -rf $DROP deliver/$BOARD/$BOARD.zip
mkdir -p $DROP

cp tools/readme.md $DROP
cp deliver/$BOARD/wideband.bin $DROP
cp deliver/$BOARD/wideband_update.srec $DROP
mkdir $DROP/bin
cp ext/openblt/Host/BootCommander.exe $DROP/bin
cp ext/openblt/Host/libopenblt.dll $DROP/bin
cp tools/update-via-pcan.bat $DROP

ls -l $DROP

cd deliver/$BOARD
zip -r $BOARD.zip $FOLDER
cd ../..