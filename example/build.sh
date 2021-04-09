#!/bin/bash

echo " --- COMPILING ---"
../bin/llang ./ex.lll
echo " --- LINKING ---"
gcc ex.o
echo " --- RUNNING ---"
./a.out