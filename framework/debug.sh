#!/bin/sh

#Run compile script
./compile.sh

cd m5/build/ALPHA_SE/
ls
# GDB
gdb prefetcher --args m5/build/ALPHA_SE/m5.debug --remote-gdb-port=0 \
    -re --outdir=output/ammp-user m5/configs/example/se.py \
    --checkpoint-dir=lib/cp --checkpoint-restore=1000000000 \
    --at-instruction --caches --l2cache --standard-switch \
    --warmup-insts=10000000 --max-inst=1000000000 --l2size=1MB \
    --bench=ammp --prefetcher=on_access=true:policy=proxy
