// CZone Delta Correlation Prefetcher
// Computer Acvhitecture NTNU 2017
// Anders Liland and Adrian Ribe


#include <iostream>
#include <list>
#include <map>
#include <stdint.h>
#include <string.h>
#include <vector>
#include <array>

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
const int CZoneMask = 2; // number of digits to mask away

enum State{CZone,pushGHB, key_first, key_second, traverse, prefetch };

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
    GHBEntry(Addr mem_addr, int delta);
    Addr mem_addr, pc;
    int delta, key_first, key_second;
    //struct GHBEntry *next, *prev; // no longer needed wince GHBEntry is in vector
    GHBEntry *next, *prev;
};
GHBEntry::GHBEntry() : mem_addr(0), pc(0), delta(0), next(NULL), prev(NULL){}
GHBEntry::GHBEntry(Addr mem_addr, int delta) : mem_addr(mem_addr), pc(0),delta(delta), next(NULL), prev(NULL){}


class GHBTable{
    public:
        Addr calculatePrefetchAddr(Addr mem_addr);
        Addr maskCZoneAddr(Addr mem_addr);
        State state;

    private:
        std::map<Addr, GHBEntry*> indexTable;
        std::map<Addr, GHBEntry*>::iterator indexTableIterator;
        std::list<GHBEntry> ghb_list;
        std::vector<int> delta_buffer;

        std::array<int, 2> key_register;
        //std::array<int , 2> compare_register;

};

static GHBTable * table;

Addr GHBTable::maskCZoneAddr(Addr mem_addr){
    return mem_addr/(10*CZoneMask) ;
}


Addr GHBTable::calculatePrefetchAddr(Addr mem_addr){
    Addr CZoneTag;
    Addr pf_addr = -1;
    GHBEntry *CZoneHead = NULL;
    GHBEntry *entry = new GHBEntry();

    //GHBEntry *entryPtr = &entry;
    //new(entryPtr) GHBEntry();

    for (;;){
    switch (state) {
        case CZone:
            //cout << "CZone" << endl;s
            CZoneTag = maskCZoneAddr(mem_addr); // mask CZone tag
            indexTableIterator = indexTable.find(CZoneTag);
            if (indexTableIterator != indexTable.end()){ // CZone tag found in Index Table
                    //CZoneTag = indexTableIterator->first;
                    CZoneHead = indexTableIterator->second; // ptr to newest element in same CZone in GHB

                    //cout << "Found CZoneTag: " << CZoneTag << " *CZoneHead: " << CZoneHead << " CZoneHead->mem_addr: " << CZoneHead->mem_addr << endl;
            }else{ // tag not found
                CZoneHead = entry;
                CZoneHead->mem_addr = mem_addr;
                indexTable.insert(std::pair<Addr, GHBEntry*>(CZoneTag, CZoneHead));
                //cout << "CZoneTag not found, add new: " << "CZoneTag: " << CZoneTag << " *CZoneHead: " << CZoneHead << " CZoneHead->mem_addr: " << CZoneHead->mem_addr << endl;
            }
            state = pushGHB;
        break;
        case pushGHB:
            entry->mem_addr = mem_addr;
            entry->delta = mem_addr - CZoneHead->mem_addr;
            entry->next = CZoneHead;
            entry->prev = NULL;
            cout << "mem_addr: " << entry->mem_addr << " Delta: " << entry->delta << " struct addr: " << entry << endl;

            //update value in Inde prt Table
            CZoneHead->prev = entry;
            CZoneHead = entry;
            indexTableIterator->second = entry; // update ptr in map to point to newest emelent in CZone
            //cout << "Update Index Table: CZoneTag " << CZoneTag << " *CZoneHead: " << CZoneHead << " CZoneHead->mem_addr: " << CZoneHead->mem_addr << endl;
            indexTable.insert(std::pair<Addr, GHBEntry*>(CZoneTag, CZoneHead));
            ghb_list.push_front(*entry);

            //if (ghb_list.size() >= GHB_LENGTH_MAX){ // ghb_list is a FIFO. Pop end when list is too long.
            //    cout << "popGHB: " << ghb_list.back().mem_addr << endl;
            //    ghb_list.pop_back();
            //}
            state = key_first;
        break;
        case key_first:
            key_register[0] = entry->delta;
            key_register[1] = CZoneHead->delta;
            //cout << "key_register[0] = " << key_register[0] << "\t" << "key_register[1] = " << key_register[1] << endl;

            state = traverse;
        break;
        case key_second:

        break;
        case traverse:
            //cout << "traverse" << endl;
            // (traverse GHB and add deltas to Comparison Register


            //GHBEntry *n = head;
            //while(n != NULL){
            //}
            state = prefetch;
        break;
        case prefetch:
            //cout << "prefetch" << endl;
            for(std::list<GHBEntry>::iterator it = ghb_list.begin(); it != ghb_list.end(); it++){
                //cout << "Addr: " <<  it->mem_addr << " Delta: " << it->delta << endl;

            }

            //PRINT_ELEMENTS(ghb_list, "ghb_list:\t");
            pf_addr = -1;
            state = CZone;
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

    int miss_addresses[9] = {1147,1149,1154,1156,1158,1163,1165,1170,1180};
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
