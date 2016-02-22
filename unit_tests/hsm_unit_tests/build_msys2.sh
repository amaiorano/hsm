#!/bin/bash

export SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"

echo ">>> Building for gcc"
cd $SCRIPT_DIR
mkdir build_gcc
cd build_gcc
export CC=`which gcc`
export CXX=`which g++`
cmake -G "MSYS Makefiles" -DHSM_DEBUG=ON ..
make -j

echo ">>> Building for clang"
cd $SCRIPT_DIR
mkdir build_clang
cd build_clang
export CC=`which clang`
export CXX=`which clang++`
cmake -G "MSYS Makefiles" -DHSM_DEBUG=ON ..
make -j

cd $SCRIPT_DIR
echo ">>> Running gcc version"
./build_gcc/hsm_unit_tests
echo ">>> Running clang version"
./build_clang/hsm_unit_tests

