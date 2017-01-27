#!/bin/sh

cd m5
scons -j2 CC=gcc CXX=g++ NO_FAST_ALLOC=False EXTRAS=../prefetcher build/ALPHA_SE/m5.opt
