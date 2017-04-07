#!/bin/sh
clear


cd m5
echo "##### COMPILING PREFETCHER #####"
scons -j2 ./build/ALPHA_SE/m5.opt CC=gcc CXX=g++ NO_FAST_ALLOC=False EXTRAS=../prefetcher

export M5_CPU2000=lib/cpu2000
