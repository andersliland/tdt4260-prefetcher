// CZone Delta Correlation Prefetcher
// Computer Acvhitecture NTNU 2017
// Anders Liland and Adrian Ribe


#include <iostream>
#include <list>
#include <stdint.h>
#include <string.h>

#include "interface.hh"

#define BLOCK_SIZE 64
#define MAX_QUEUE_SIZE 100
#define MAX_PHYS_MEM_ADDR ((uint64_t)(256*1024*1024) - 1)


using namespace std;

class GHBEntry{

public:
    GHBEntry(Addr mem_addr);

private:
    Addr missAddress, lastAddress, lastPrefetch;
};

GHBEntry::GHBEntry(Addr mem_addr){

    cout << "mem_addr" << mem_addr << endl;
}

class GHBTable{
public:
    GHBTable();
    ~GHBTable();

private:
    list<GHBEntry *> table;

};

class IndexTableEntry{

};
class IndexTable{

};










// --------- PREFETCH SIMULATED FUNCTIONS ------------------------
void prefetch_init(void){
    std::cout << "prefetch_init" << std::endl;
}

void prefetch_access(AccessStat stat)
{
    std::cout << "There have been a cache access" << std::endl;
    std::cout << "Hit(0) or miss(1): " << stat.miss << std::endl;
    std::cout << "mem_addr " << stat.mem_addr << std::endl;

}


void prefetch_complete(Addr addr)
{
    std::cout << "prefetch_complete" << std::endl;
}


int main( ) {

    AccessStat AccessStat_instance;
    AccessStat_instance.pc = 123;
    AccessStat_instance.mem_addr = 24;
    AccessStat_instance.time = 2;
    AccessStat_instance.miss = 1;

    prefetch_init();
    prefetch_access(AccessStat_instance);



    prefetch_complete(AccessStat_instance.pc);

    return 0;
}
