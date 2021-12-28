#!/bin/bash

# back out to the root
cd ../..

# build app firmware!
make -j12 BOARD=f1_dave
