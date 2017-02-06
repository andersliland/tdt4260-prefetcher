# Hardware prefetching algorithm

Hardware prefetchers are not always your friend. They operate in one of the three states:
* Not detecting any patterns so quite
* Detecting the correct pattern so improving performance
* Detecting the wrong patterns so reducing performance.


### Instruction prefetching
Prefetching instruction lines from memory to cache to reduce instruction cache misses. This also assists in instruction decode and increases the instruction issue rate.

#### Instruction Prefetch Algorithms
* Long Cache Lines
* Next-Line Prefetching
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
* GHB, Global History Bufer
    * n entry FIFO queue implemented as a circular buffer
    * Stores n most recent L1 cache miss
* Stride directed prefetching
* Delta Correlation Prefetchers (advanced version of Stride) [Link](http://web.engr.oregonstate.edu/~benl/Projects/prefetch_report/final.html)
  * CZone Delta Correlation
    * based on assumption that access within a data structure is fairly regular, and similar data structures have spatial locality.
  * Program Counter Delta Correlation
  * Adaptive Program Counter
  * Delta Correlation Prediction Tables
* PC/DC, Program Counter/ Delta Correlation Prefetching (by Nesbit and Smith)
* RPT, Reference Prediction Tables (by Chen and Baer)
  * found to be top preformer in 2004, Perez, et al.

#### Adress based prefetcher
* Markov


#### Spatial locality prefetcher
* SMS, Spatial Memory Streaming

#### Linked data prefetcher
* CDP, Content Directed Prefetching
