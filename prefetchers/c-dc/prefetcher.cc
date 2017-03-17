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

//const int DeltaCorrelationSequenceLength = 2;
//const int CZoneSize = 64; // KB
//const int PrefetchDegree = 4;
enum State{CZone,addToGHB, key_first, key_second, traverse, prefetch };

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
        std::cout << *pos << ' ';
    }
    std::cout << std::endl;
}


struct GHBEntry{
    GHBEntry();
    GHBEntry(Addr mem_addr);
    Addr mem_addr, pc;
    int delta, key_first, key_second;
    struct GHBEntry *next, *prev;
}*head;
GHBEntry::GHBEntry() : mem_addr(0), pc(0), delta(0), next(NULL), prev(NULL){}
GHBEntry::GHBEntry(Addr mem_addr) : mem_addr(mem_addr), pc(0),delta(0), next(NULL), prev(NULL){}

struct ITEntry{
    ITEntry();
    int *ptr;
};
ITEntry::ITEntry() : ptr(NULL){}




class GHBTable{
    public:
        GHBTable(){
            head = NULL;
        }
        Addr *itPtr;

        Addr calculatePrefetchAddr(Addr mem_addr);
        Addr maskCZoneAddr(Addr mem_addr);

        GHBEntry* create_list(Addr mem_addr);
        int add_begin(Addr mem_addr, GHBEntry * entry);
        void display_list();
        void append(Addr mem_addr);
        void computeDelta(Addr addr, GHBEntry * entry);
        bool correlationHit();
        void traverse_and_print();

    private:
        State state;
        std::map<Addr, GHBEntry*> indexTable;
        std::vector<std::map<Addr, int>> indexTable2; 
        std::vector<GHBEntry*> ghb_list;
        std::map<Addr, GHBEntry*>::iterator indexTableIterator;
        std::map<Addr, GHBEntry* > entries;
        std::vector<int> delta_buffer;
        std::vector<int> key_vector;
        std::vector<int> comparison_register;
        int key_register[2];
        int compare_register[2];
};

static GHBTable * table;

GHBEntry* GHBTable::create_list(Addr mem_addr){
    struct GHBEntry *s, *temp;
    temp = new GHBEntry(mem_addr);
    temp->next = NULL;
    if (head == NULL){ // list is empty
        temp->prev = NULL;
        head = temp;
        return temp;
    }else{ // list not empty
        s = head;
        while (s->next != NULL)
            s = s->next;
        s->next = temp;
        temp->prev = s;
        return s;
    }
}

int GHBTable::add_begin(Addr addr, GHBEntry *entry){
    if(head == NULL){
        cout << "Creating GHB table and adding first element." << endl;
        GHBTable::create_list(addr);
        return -1;
    }
    //struct GHBEntry *entry;
    entry = new (struct GHBEntry);
    entry->prev = NULL;
    entry->mem_addr = addr;
    entry->delta = addr - head->mem_addr; // calculate delta
    entry->next = head;
    head->prev = entry;
    head = entry;
    cout << "Element inserted\t" << "Addr: " << entry->mem_addr << " Delta: " << entry->delta << endl;
    return entry->delta;
}
Addr GHBTable::maskCZoneAddr(Addr mem_addr){
    cout << "mask CZone Addr" << endl;
    return mem_addr;
}


Addr GHBTable::calculatePrefetchAddr(Addr mem_addr){
    //static int i = 0;
    int delta = -1;
    Addr CZoneTag;
    Addr pf_addr = -1;
    GHBEntry *entry;
    for (;;){
    switch (state) {
        case CZone:
            //cout << "CZone" << endl;
            CZoneTag = maskCZoneAddr(mem_addr); // mask CZone tag

            indexTableIterator = indexTable.find(CZoneTag);
            if (indexTableIterator != indexTable.end()){ // CZone tag found in Index Table
                    CZoneTag = indexTableIterator->first;
                    entry = indexTableIterator->second; // index table prt allways points to newest element of same zone in GHB
                    cout << "CZone tag found: key: " << indexTableIterator->first << " value: " << indexTableIterator->second << endl;
            }else{ // CZone tag not in Index Table
                cout << "CZone tag not found, creating new and add to index Table" << endl;
                //struct ITEntry *e = new (struct ITEntry);
                entry = new (struct GHBEntry);
                indexTable.insert(std::pair<Addr, GHBEntry*>(CZoneTag, entry)); // should value be only a ptr?
                cout << "CZoneAddr " << CZoneTag << " Entry " << entry << endl;
            }
            state = addToGHB;
        break;
        case addToGHB:
            cout << "addToGHB" << endl;
            ghb_list.push_back(entry);



            PRINT_ELEMENTS(ghb_list, "ghb_list:    ");

            state = prefetch;
        break;
        case key_first:
            cout << "key_first" << endl;
            //delta = table->add_begin(mem_addr);
            delta_buffer.push_back(delta);
            key_register[1] = delta;
            state = key_second;
        break;
        case key_second:
            cout << "key_second" << endl;
            //delta = table->add_begin(mem_addr);
            delta_buffer.push_back(delta);
            key_register[0] = delta;
            state = traverse;
        break;
        case traverse:
            cout << "traverse" << endl;
            // (traverse GHB and add deltas to Comparison Register
            for(std::vector<GHBEntry*>::iterator it = ghb_list.begin(); it != ghb_list.end(); it++){


            }

            //GHBEntry *n = head;
            //while(n != NULL){
            //}
            state = prefetch;
        break;
        case prefetch:
            cout << "address" << endl;
            if (pf_addr == -1)
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

void GHBTable::display_list(){
    struct GHBEntry * q;
    if(head == NULL){
        cout << "List empty, nothing to display" << endl;
        return;
    }
    // Print GHB Table
    q = head;
    cout << "Addr\t";
    while(q != NULL){
        cout << q->mem_addr << "\t";
        q = q->next;
    }
    cout << "\tTAIL" << endl;
    // Print Delta Buffer
    q = head;
    cout << "Delta\t";
    while(q != NULL){
        cout << q->delta << "\t";
        q = q->next;
    }
    cout << "\tTAIL" << endl;

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

    table->display_list();


    prefetch_complete(stat.mem_addr);
    return 0;
}
