# tdt4260-prefetcher
TDT4260 Computer Architecture @ NTNU 2017
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
* Cache block size: 64 bytes
* Maximum pending prefetch requests: 100
* Largest physical address: 268 435 455


# Hardware prefetching algorithm

Hardware prefetchers are not always your friend. They operate in one of the three states:
* Not detecting any patterns so quite
* Detecting the correct pattern so improving performance
* Detecting the wrong patterns so reducing performance.


### Instruction prefetching
Prefetching instruction lines from memory to cache to reduce instruction cache misses. This also assists in instruction decode and increases the instruction issue rate.

#### Instruction Prefetch Algorithms
*  Long Cache Lines
  *   Next-Line Prefetching
* Target-Line Prefetching
* Hybrid Prefetching
* Wrong Path Prefetching

### Data Prefetching
This form of prefetch is aimed to reduce data cache misses by exploiting the program access pattern for data. This data access pattern could possibly be placed in the executable by the compiler.
Data prefetching has been proposed as a technique for hiding the access latency of data referencing patterns that defeat caching strategies. Rather than waiting for a cache miss to perform a memory fetch, data prefetching anticipates such misses and issues a fetch to the memory system in advance of the actual memory reference.

### Data Prefetch Algorithms
#### Sequential prefetcher
  * sequential prefetcher (easiest)
  * tagged sequiental prefetcher (easy)
  * Stream Prefetch
  * the stream buffer
  * the stride prediction table (SPT)
  * the stream cache

#### Instruction based prefetcher
* RPT, Chen 95
* GHB, Nesbit, 04 (GHB is a data structure, not a prefetch algorithm)
  * GHB G/DC hybrid
  * GBH PC/DC
* DCPT, Grannaes, 11


#### Adress based prefetcher
* Markov


#### Spatial locality prefetcher
* SMS, Spatial Memory Streaming

#### Linked data prefetcher
* CDP, Content Directed Prefetching


### Notation ###
X/Y prefetcher

x- method used for localising the miss address stream
y - method ised for detection adress pattern

# SPEC CPU2000

SPEC CPU2000 is a software benchmark product by the Standard Preformance Evaluation Corp. (SPEC), a non-profit group that includes computer vendors, system integrators, universities, research organizations, publishers and consultants around the world.

[SPEC2000 FAQ](http://www.spec.org/cpu2000/press/faq.html)

Contains two benchmark suites: (component-level benchmarks)
### CINT2000 - compute-intensive integer preformance
* bzip2_graphic
* bzip2_program
* bzip2_source
* twolf


### CFP2000 - compute-intensive floating point preformance
* ammp - Computational chemistry
* applu - Parabolic/elliptic partial differential equations
* apsi - Solves problems regarding temperature, wind, distribution of pollutants
* art110 - Neural Network simulation: adaptive resonance theory
* art470 - Neural Network simulation: adaptive resonance theory
* galgel - Fluid dynamics: analysis of oscillatory instabillity
* swim - Shallow water modeling
* wupwise - Quantum chromodynamics



# Alpha 21264 (Alpha 7)
[Alpha 21264 Wikipedia](https://en.wikipedia.org/wiki/Alpha_21264)
Digital Equipment Corporation RISC microporcessor.
Implemented the Alpha instruction set architecture
### microarchitecture
* supercalar
* out-of-order execution
* speculative execution
* seven stage pipeline
### cache
* L1
  * data 64KB
    * dual-porting: transfering data on both rising and falling edge
  * instruction 64KB
* L2 (off die)
  * 1 - 16 MB, SSRAM
  * direct-mapped
*
