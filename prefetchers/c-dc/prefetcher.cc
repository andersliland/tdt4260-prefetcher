// CZone Delta Correlation Prefetcher
// Computer Acvhitecture NTNU 2017
// Anders Liland and Adrian Ribe


#include <iostream>
#include <algorithm>
#include <iterator>
#include <list>
#include <map>
#include <stdint.h>
#include <string.h>
#include <vector>
#include <array>

#include "interface.hh"

#define BLOCK_SIZE 64 // Size of cache blocks (cache lines) in bytes.
#define MAX_QUEUE_SIZE 100 // Maximum number of pending prefetch requests.
#define MAX_PHYS_MEM_ADDR ((uint64_t)(256*1024*1024) - 1) // 268 435 455


using std::cout;
using std::endl;
using std::vector;

//const int DELTA_CORRELATION_SEQUENCE_LENGTH = 2;
//const int CZONE_SIZE = 64; // number of bytes in CZone
const int PREFETCH_DEGREE = 4;
const int GHB_LENGTH_MAX = 10;
const int CZoneMask = 3; // number of digits to mask away

static int Timestep = 0;

enum State{CZone,pushGHB, traverse, prefetch };
std::vector<int> delta_buffer;


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
        std::vector<int> calculatePrefetchAddr(Addr mem_addr);
        Addr maskCZoneAddr(Addr mem_addr);
        State state;

    private:
        std::map<Addr, GHBEntry*> indexTable;
        std::map<Addr, GHBEntry*>::iterator indexTableIterator;
        std::list<GHBEntry> ghb_list;

        std::array<int, 2> key_register;
        std::array<int, 2> compare_register;

};

static GHBTable * table;

// Mask the 'CZoneMask' MSB of mem_addr
Addr GHBTable::maskCZoneAddr(Addr mem_addr){
    // mem_addr is 28 bit
    // return mem_addr >> (ADDR_BITWIDTH - CZoneMask)
    return mem_addr/(10*CZoneMask);
}


std::vector<int> GHBTable::calculatePrefetchAddr(Addr mem_addr){
    Addr CZoneTag;
    //Addr pf_addr = -1;
    GHBEntry *CZoneHead = NULL;
    GHBEntry *entry = new GHBEntry();
    while(true){
    switch (state) {

        // Check IndexTable for ZCone tag, and add to list if not pressent.
        case CZone:
            Timestep++;
            cout << "\n\n" << endl;
            cout << "TIMESTEP\t" << Timestep << endl;
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

        // Add miss addr to GHB list, calculate delta and add miss addr to Correlation Key Register
        case pushGHB:
            entry->mem_addr = mem_addr;
            entry->delta = mem_addr - CZoneHead->mem_addr;
            entry->next = CZoneHead;
            entry->prev = NULL;
            cout << "Addr: " <<  entry->mem_addr << " Delta: " << entry->delta<< endl;
            //cout << "mem_addr: " << entry->mem_addr << " Delta: " << entry->delta << " struct addr: " << entry << endl;
            key_register[1] = key_register[0];
            key_register[0] = entry->delta;
            cout << "key_register[0] = " << key_register[0] << endl;
            cout << "key_register[1] = " << key_register[1] << endl;
            //update value in Index prt Table
            CZoneHead->prev = entry;
            CZoneHead = entry; // make new entry top of its CZone
            indexTableIterator->second = entry; // update ptr in map to point to newest emelent in CZone
            //cout << "Update Index Table: CZoneTag " << CZoneTag << " *CZoneHead: " << CZoneHead << " CZoneHead->mem_addr: " << CZoneHead->mem_addr << endl;
            indexTable.insert(std::pair<Addr, GHBEntry*>(CZoneTag, CZoneHead));
            ghb_list.push_front(*entry);

            //cout << entry->mem_addr << endl;
            //cout << "GHB size " << ghb_list.size()  << endl;
            //PRINT_ELEMENTS(ghb_list, "ghb_list:\t");

            // TODO: figure out why ghb_list is poped every time?
            if (ghb_list.size() >= GHB_LENGTH_MAX){ // ghb_list is a FIFO. Pop end when list is too long.
                //cout << "popGHB: " << ghb_list.back().mem_addr << endl;
                //ghb_list.pop_back();
            }
            state = traverse;
        break;
        case traverse:
        {   //traverse GHB and add deltas to Comparison Register
            std::list<GHBEntry>::iterator i = ghb_list.begin();
            std::advance(i, 1); // advance itterator two elements from head of list

            // Traverse GHB backward, add deltas to Comparison register.
            for(std::list<GHBEntry>::iterator it = i; it != ghb_list.end(); it++){
                delta_buffer.push_back(compare_register[1]);
                compare_register[1] = compare_register[0];
                compare_register[0] = it->delta;
                cout << "compare_register[0] = " << compare_register[0] << endl;
                cout << "compare_register[1] = " << compare_register[1] << endl;
                cout << "\n" << endl;

                if(compare_register[0] == key_register[0] && compare_register[1] == key_register[1] && it->mem_addr != mem_addr ){ //correlation hits
                    state = prefetch;
                }
            }
            if( state == prefetch){ // continue to prefetch case
                cout << "Correlation Hit " << endl;
                break;
            }else{ // not prefetch for this miss address
                state = CZone;
                return {}; // list initialisation
            }

        }
        case prefetch:
            cout << "prefetch " << endl;
            state = CZone;
            return delta_buffer;
    } // switch
} // while
}

// --------- PREFETCH SIMULATED FUNCTIONS ------------------------
void prefetch_init(void){
    std::cout << "prefetch_init" << std::endl;
    table = new GHBTable;
    std::list<GHBEntry> ghb_list;
    ghb_list.clear();

}

void prefetch_access(AccessStat stat){
    Addr pf_addr;
    std::vector<int> temp_delta_buffer;
    if(stat.miss){
        temp_delta_buffer = table->calculatePrefetchAddr(stat.mem_addr);


        cout << temp_delta_buffer[1] << endl;

        for(int i = 0; i < PREFETCH_DEGREE; i++){
            if(pf_addr < MAX_PHYS_MEM_ADDR && pf_addr != -1){
                cout << "Issue prefetch for address: " << pf_addr << endl;
                //issue_prefetch( pf_addr );
            }else{
                cout << "Not issuing prefetch" << endl;
            }
        }
    }
}
void prefetch_complete(Addr addr){
    cout << "prefetch_complete" << endl;
}


int main( ) {
    AccessStat stat;
    prefetch_init();

    int pc[7] = {1,2,3,4,5,6,7};
    int miss_addresses[7] = {1147,1149,1154,1156,1158,1163,1165};
    for (int i = 0; i < 7; i++ ){
        stat.pc = pc[i];
        stat.mem_addr = miss_addresses[i];
        stat.time = 1;
        stat.miss = 1;
        prefetch_access(stat);
    }

    prefetch_complete(stat.mem_addr);
    return 0;
}
