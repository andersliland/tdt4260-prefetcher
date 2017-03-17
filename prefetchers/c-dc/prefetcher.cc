// CZone Delta Correlation Prefetcher
// Computer Acvhitecture NTNU 2017
// Anders Liland and Adrian Ribe


#include <iostream>
#include <list>
#include <map>
#include <stdint.h>
#include <string.h>
#include <vector>

#include "interface.hh"

#define BLOCK_SIZE 64
#define MAX_QUEUE_SIZE 100
#define MAX_PHYS_MEM_ADDR ((uint64_t)(256*1024*1024) - 1) // 268 MB


using std::cout;
using std::endl;
using std::vector;

//const int DELTA_CORRELATION_SEQUENCE_LENGTH = 2;
//const int CZONE_SIZE = 64; // KB
//const int PREFETCH_DEGREE = 4;
const int GHB_LENGTH_MAX = 10;

enum State{CZone,pushGHB,popGHB, key_first, key_second, traverse, prefetch };

/* PRINT_ELEMENTS()
 * - prints optional C-string optcstr followed by
 * - all elements of the collection coll
 * - separated by spaces
 */
template <class T>
inline void PRINT_ELEMENTS (const T& coll, const char* optcstr="")
{
    typename T::const_iterator pos;

    std::cout << optcstr;
    for (pos=coll.begin(); pos!=coll.end(); ++pos) {
        //std::cout << *pos << ' ';
    }
    std::cout << std::endl;
}


struct GHBEntry{
    GHBEntry();
    GHBEntry(Addr mem_addr);
    Addr mem_addr, pc;
    int delta, key_first, key_second;
    //struct GHBEntry *next, *prev; // no longer needed wince GHBEntry is in vector
    GHBEntry *next, *prev;
};
GHBEntry::GHBEntry() : mem_addr(0), pc(0), delta(0), next(NULL), prev(NULL){}
GHBEntry::GHBEntry(Addr mem_addr) : mem_addr(mem_addr), pc(0),delta(0), next(NULL), prev(NULL){}


class GHBTable{
    public:
        Addr calculatePrefetchAddr(Addr mem_addr);
        Addr maskCZoneAddr(Addr mem_addr);
        State state;

    private:
        std::map<Addr, GHBEntry> indexTable;
        std::map<Addr, GHBEntry>::iterator indexTableIterator;

        std::list<GHBEntry> ghb_list;
        std::vector<int> delta_buffer;
        std::vector<int> key_vector;
        std::vector<int> comparison_register;

};

static GHBTable * table;

Addr GHBTable::maskCZoneAddr(Addr mem_addr){
    cout << "mask CZone Addr" << endl;
    return mem_addr;
}


Addr GHBTable::calculatePrefetchAddr(Addr mem_addr){
    //static int i = 0;
    Addr CZoneTag;
    Addr pf_addr = -1;
    //Addr *itPtr; // should be type Addr? (unsigned long long)
    GHBEntry entry;
    GHBEntry *CZoneHead = NULL;
    for (;;){
    switch (state) {
        case CZone:
            //cout << "CZone" << endl;s
            CZoneTag = maskCZoneAddr(mem_addr); // mask CZone tag

            indexTableIterator = indexTable.find(CZoneTag);
            if (indexTableIterator != indexTable.end()){ // CZone tag found in Index Table
                    CZoneTag = indexTableIterator->first;
                    CZoneHead = &indexTableIterator->second; // ptr to newest element in same CZone in GHB
                    GHBEntry entry;
                    cout << "Found CZoneTag: " << CZoneTag << " GHBEntry addr: " << &CZoneHead << endl;
            }else{ // CZone tag not in Index Table
                GHBEntry entry;
                indexTable.insert(std::pair<Addr, GHBEntry>(CZoneTag, entry));
                cout << "CZone tag not found, add new: " << "CZoneTag: " << CZoneTag << " GHBEntry addr: " << &entry << endl;
            }
            state = pushGHB;
        break;
        case pushGHB:
            cout << "pushGHB" << endl;
            entry.mem_addr = mem_addr;

            if (CZoneHead != NULL){ // another element in same CZone exsist
                entry.next = CZoneHead;
                entry.prev = NULL;
                entry.delta = mem_addr - CZoneHead->mem_addr;
            }else{ // no element in same CZone exsist
                entry.next = NULL;
                entry.prev = NULL;
                //entry->delta = Invallid; Can delta be negative?
            }
            ghb_list.push_front(entry);
            state = popGHB;
        break;
        case popGHB:
            if (ghb_list.size() >= GHB_LENGTH_MAX){
                cout << "popGHB: " << ghb_list.back().mem_addr << endl;
                ghb_list.pop_back();
            }
            state = traverse;
        break;
        case key_first:

        break;
        case key_second:

        break;
        case traverse:
            cout << "traverse" << endl;
            // (traverse GHB and add deltas to Comparison Register
            for(std::list<GHBEntry>::iterator it = ghb_list.begin(); it != ghb_list.end(); it++){
                cout << "Addr: " <<  it->mem_addr << "Delta: " << it->delta << endl;

            }

            //GHBEntry *n = head;
            //while(n != NULL){
            //}
            state = prefetch;
        break;
        case prefetch:
            cout << "prefetch" << endl;

            //PRINT_ELEMENTS(ghb_list, "ghb_list:\t");
            pf_addr = -1;
            return pf_addr;

        break;
    }

}

    // Add two first deltas to Correlation Key Register

    //delta_buffer.push_back(table->add_begin(mem_addr));
    //delta_buffer.push_back(table->add_begin(mem_addr));

    //for(vector<int>::const_iterator i = delta_buffer.begin(); i != delta_buffer.end(); i++){
    //    cout << *i << ' ';
    //}
    //cout << endl;
    return -1;
}

// --------- PREFETCH SIMULATED FUNCTIONS ------------------------
void prefetch_init(void){
    std::cout << "prefetch_init" << std::endl;
    table = new GHBTable;

}

void prefetch_access(AccessStat stat){
    Addr pf_addr;
    if(stat.miss){
        pf_addr = table->calculatePrefetchAddr(stat.mem_addr);
        if(pf_addr < MAX_PHYS_MEM_ADDR && pf_addr != -1){
            cout << "Issue prefetch for address: " << pf_addr << endl;
            //issue_prefetch( pf_addr );
        }
    }
}
void prefetch_complete(Addr addr){
    cout << "prefetch_complete" << endl;
}


int main( ) {
    AccessStat stat;
    prefetch_init();

    int miss_addresses[9] = {47,49,54,56,58,63,65,70,80};
    for (int i = 0; i < 9; i++ ){
        stat.pc = 55;
        stat.mem_addr = miss_addresses[i];
        stat.time = 1;
        stat.miss = 1;
        prefetch_access(stat);
    }

    prefetch_complete(stat.mem_addr);
    return 0;
}
