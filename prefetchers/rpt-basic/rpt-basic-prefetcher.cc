/*
 * A sample prefetcher which does sequential one-block lookahead.
 * This means that the prefetcher fetches the next block _after_ the one that
 * was just accessed. It also ignores requests to blocks already in the cache.
 */
 // Code from https://github.com/andsild/TDT4260
#include <map>
#include "interface.hh"

#define ENTRY_LIMIT 100
using namespace std;

struct RPTentry{
    RPTentry(Addr pc);

    Addr pc;
    Addr prev_addr;
    int stride;
    RPTentry *next;
    RPTentry *prev;

};


struct RPTmap{
    RPTmap();

    RPTentry *find_entry(Addr pc);
    int AmountOfEntries;
    RPTentry *head_ptr, *tail_ptr;
    map <Addr, RPTentry* > entryMap;
};

// table initialised at the begining of execution
static RPTmap *table;

// construct entry
// not sure this is neccecary, struct init 0 by default
RPTentry::RPTentry(Addr pc): pc(pc), prev_addr(0), stride(0), next(0), prev(0) {}

// construct map
// same consern as above
RPTmap::RPTmap() : AmountOfEntries(0), head_ptr(0), tail_ptr() {}


RPTentry *RPTmap::find_entry(Addr pc){

    if(entryMap.find(pc) == entryMap.end())
    {// there is no entry in the RPT
        // create a new entry with program counter address
        RPTentry *newEntry = new RPTentry(pc);

        // table full
        if(AmountOfEntries == ENTRY_LIMIT){
            RPTentry *removeEntry = tail_ptr;
            tail_ptr = removeEntry->next;
            tail_ptr->prev = 0;
            entryMap.erase(removeEntry->pc);
            delete removeEntry;
        }


        // table not empty
        if(head_ptr != 0){
            head_ptr->next = newEntry;
            newEntry->prev = head_ptr;
        }else{ // table empty
            tail_ptr = newEntry;
            newEntry->prev = 0;
            newEntry->next = 0;
        }

        head_ptr = newEntry;

        //add the entry into the map. Set pc as key.
        entryMap[pc] = newEntry;


        if(AmountOfEntries < ENTRY_LIMIT) ++AmountOfEntries;

    }

    return entryMap[pc];
}




///////////////////////////////////////////////////////////////////////

void prefetch_init(void)
{
    /* Called before any calls to prefetch_access. */
    /* This is the place to initialize data structures. */
    DPRINTF(HWPrefetch, "Initialized Reference Prediction Table Basic prefetcher\n");
    // initiate map-tablestructure
    table = new RPTmap;
}

void prefetch_access(AccessStat stat)
{
    if (stat.miss) {
        // get entry from map and issue a prefetch
        RPTentry *entry = table->find_entry(stat.pc);
        int newStride = stat.mem_addr - entry->prev_addr;
        // check for pattern
        //MSHR - Miss Status and Handling Register
        // if current addr, and next addr (based on stide)
        if(entry->stride == newStride && !in_cache(stat.mem_addr + entry->stride) && !in_mshr_queue(stat.mem_addr + entry->stride)){
            issue_prefetch(stat.mem_addr + entry->stride);
        }
        entry->stride = newStride;
        entry->prev_addr = stat.mem_addr;
    }
}
void prefetch_complete(Addr addr) {
  // Called when a block requested by the prefetcher has been loaded.
}
