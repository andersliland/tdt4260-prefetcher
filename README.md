# tdt4260-prefetcher
TDT4260 Computer Architecture @ NTNU 2017
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)



* Cache block size: 64 bytes
* Maximum pending prefetch requests: 100
* Largest physical address: 268 435 455


Hardware prefetchers are not always your friend. They operate in one of the three states:
-Not detecting any patterns so quite
-Detecting the correct pattern so improving performance
-Detecting the wrong patterns so reducing performance.


* Instruction
Prefetching instruction lines from memory to cache to reduce instruction cache misses. This also assists in instruction decode and increases the instruction issue rate.

Instruction Prefetch Algorithms
* Long Cache Lines
* Next-Line Prefetching
* Target-Line Prefetching
* Hybrid Prefetching
* Wrong Path Prefetching

* Data Prefetching
This form of prefetch is aimed to reduce data cache misses by exploiting the program access pattern for data. This data access pattern could possibly be placed in the executable by the compiler.
Data prefetching has been proposed as a technique for hiding the access latency of data referencing patterns that defeat caching strategies. Rather than waiting for a cache miss to perform a memory fetch, data prefetching anticipates such misses and issues a fetch to the memory system in advance of the actual memory reference.

Data Prefetch Algorithms
* the stream buffer@
* the stride prediction table (SPT)
* the stream cache
