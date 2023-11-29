#! /bin/bash

source setup_rk3566_env.sh

rm -rf ./build
mkdir -p build
cd build
#cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 .. 
cmake ../ -DPL_3566=1 -DCMAKE_EXPORT_COMPILE_COMMANDS=1
make  -j4 
 

cp wyq_app ../release_wyq/





