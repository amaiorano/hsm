#!/bin/bash

export SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"

cd $SCRIPT_DIR
mkdir build_gcc
cd build_gcc
export CC=`which gcc`
export CXX=`which g++`
cmake -G "Unix Makefiles" -DHSM_DEBUG=ON ..
make -j

cd $SCRIPT_DIR
mkdir build_clang
cd build_clang
export CC=`which clang`
export CXX=`which clang++`
cmake -G "Unix Makefiles" -DHSM_DEBUG=ON ..
make -j

