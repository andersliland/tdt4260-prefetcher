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
const int PREFETCH_DEGREE = 3;
const int GHB_LENGTH_MAX = 10;
const int CZoneMask = 3; // number of digits to mask away

static int Timestep = 0;

enum State{CZone,pushGHB, traverse, prefetch };

struct GHBEntry{
    GHBEntry();
    GHBEntry(Addr mem_addr, int delta);
    Addr mem_addr, pc;
    int delta, key_first, key_second;
    GHBEntry *next, *prev; // pointers used to make a linked list inside each CZone
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

        std::vector<int> delta_buffer;
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
    GHBEntry *CZoneHead = NULL;
    GHBEntry *entry = new GHBEntry();
    delta_buffer.clear(); // remove all values from previous calulation
    ghb_list.clear();
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

            // TODO: figure out why ghb_list is poped every time?
            cout << "GHB size " << ghb_list.size() << endl;
            if (ghb_list.size() >= GHB_LENGTH_MAX){ // ghb_list is a FIFO. Pop end when list is too long.
                //cout << "popGHB: " << ghb_list.back().mem_addr << endl;
                //ghb_list.pop_back();
            }
            state = traverse;
        break;
        case traverse:
        {
            // Print GHB list
            cout << "GHB:" << endl;
            for (std::list<GHBEntry>::const_iterator iterator = ghb_list.begin(), end = ghb_list.end(); iterator != end; ++iterator) {
                cout << "Mem_addr: " << iterator->mem_addr << " Delta: " << iterator->delta  << " CZoneTag: " <<  endl;
            }


            //traverse GHB and add deltas to Comparison Register
            std::list<GHBEntry>::iterator i = ghb_list.begin();
            std::advance(i,1); // advance itterator two elements from head of list

            // Traverse GHB backward, add deltas to Comparison register.
            for(std::list<GHBEntry>::iterator it = i; it != ghb_list.end(); it++){
                compare_register[1] = compare_register[0];
                compare_register[0] = it->delta;


                if(compare_register[0] == key_register[0] && compare_register[1] == key_register[1] && it->mem_addr != mem_addr ){ //correlation hits
                    state = prefetch;
                }
            }

            auto index = delta_buffer.begin();
            delta_buffer.insert(index, compare_register[1]);

            if( state == prefetch){ // continue to prefetch case
                cout << "Correlation Hit " << endl;
                break;
            }else{ // not prefetch for this miss address
                state = CZone;
                delta_buffer.clear();
                return delta_buffer; // return vector without any elements
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
    GHBEntry *entry2 = new GHBEntry();
    std::list<GHBEntry> ghb_list;
    ghb_list.clear();
    cout << "init ghb size " << ghb_list.size() << endl;
    ghb_list.push_front(*entry2);
    cout << "init ghb size " << ghb_list.size() << endl;


}

void prefetch_access(AccessStat stat){
    Addr pf_addr = 0, mem_addr = stat.mem_addr;
    std::vector<int> temp_delta_buffer;
    if(stat.miss){ //calculate prefetch address only on miss

        temp_delta_buffer = table->calculatePrefetchAddr(stat.mem_addr);
        if ( temp_delta_buffer.empty() ) {
            cout << "Delta buffer is empty. No delta correlation found for miss addr: " << stat.mem_addr << endl;
        } else {
            //temp_delta_buffer.erase(temp_delta_buffer.begin(),temp_delta_buffer.begin()+2); // remove two first deltas, according to algorithm
            int delta_buffer_size = temp_delta_buffer.size();
            cout << "Delta buffer size " << delta_buffer_size << endl;

            // DEBUG: print all deltas in vector
            int j = 0;
            for(auto val: temp_delta_buffer){
                cout << "temp_delta_buffer[" << j << "]: " << temp_delta_buffer[j] << endl;
                j++;
            }

            for(int i = 0; i < (PREFETCH_DEGREE && delta_buffer_size); i++){
                pf_addr = mem_addr + temp_delta_buffer[i];
                if(pf_addr < MAX_PHYS_MEM_ADDR){
                    cout << "Issue prefetch for address: " << pf_addr << endl;
                    //issue_prefetch( pf_addr );
                }else{
                    cout << "Not issuing prefetch" << endl;
                }
                mem_addr = pf_addr; // The prefetch addresses are cumulative
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
