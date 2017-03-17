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
#define MAX_PHYS_MEM_ADDR ((uint64_t)(256*1024*1024) - 1)


using std::cout;
using std::endl;
using std::vector;

struct GHBEntry{

    GHBEntry();
    GHBEntry(Addr mem_addr);
    Addr mem_addr, pc;
    int delta;
    struct GHBEntry *next, *prev;
}*head;

GHBEntry::GHBEntry() : mem_addr(0), pc(0), delta(0), next(NULL), prev(NULL){}
GHBEntry::GHBEntry(Addr mem_addr) : mem_addr(mem_addr), pc(0),delta(0), next(NULL), prev(NULL){}


class GHBTable{
    public:
        GHBTable(){
            head = NULL;
        }
        const int DeltaCorrelationSequenceLength = 2;


        Addr calculatePrefetchAddr(Addr mem_addr);
        GHBEntry* create_list(Addr mem_addr);
        int add_begin(Addr mem_addr);
        void display_list();
        void append(Addr mem_addr);


        void computeDelta(Addr addr, GHBEntry * entry);
        bool correlationHit();

        void traverse_and_print();
    private:
        std::map<Addr, GHBEntry* > entries;
        std::vector<int> delta_buffer;
        std::vector<int> key_register;
        std::vector<int> comparison_register;
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

int GHBTable::add_begin(Addr addr){
    if(head == NULL){
        cout << "Creating GHB table and adding first element." << endl;
        GHBTable::create_list(addr);
        return -1;
    }
    struct GHBEntry *entry;
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

Addr GHBTable::calculatePrefetchAddr(Addr mem_addr){
    static int i = 0;

    // Add two first deltas to Correlation Key Register
    if (i < DeltaCorrelationSequenceLength) {
    int delta = table->add_begin(mem_addr);
    delta_buffer.push_back(delta);
    key_register.push_back(delta);
    i++;
    return -1;
    }
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

    // Print Correlation Key Register
    q = head;
    cout << "Key\t";
    while(q != NULL){
        cout << q->delta << "\t";
        q = q->next;
    }
    cout << "\tTAIL" << endl;

    // Print Correlation Comparison Register
    q = head;
    cout << "Compare\t";
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
