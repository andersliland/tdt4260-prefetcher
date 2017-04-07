#!/bin/sh
clear

# ammp applu apsi art110 art470 bzip2_graphic bzip2_program bzip2_source galgel swim twolf wupwise

benchmark="ammp"
num_instructions="1000"
num_warmup="1000"

echo "##### RUN #####"

export M5_CPU2000=lib/cpu2000
m5/build/ALPHA_SE/m5.opt --remote-gdb-port=0 -re \
--outdir=output/ammp-user m5/configs/example/se.py \
--checkpoint-dir=lib/cp --checkpoint-restore=1000000000 \
--at-instruction --caches --l2cache --standard-switch \
--warmup-insts=$num_warmup --max-inst=$num_instructions --l2size=1MB \
--membus-width=8 --membus-clock=400MHz --mem-latency=30ns \
--bench=$benchmark --prefetcher=on_access=true:policy=proxy

#  --trace-flags=HWPrefetch
echo "##### FINISHED #####"
