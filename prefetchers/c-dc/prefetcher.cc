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

        Addr calculatePrefetchAddr(Addr mem_addr);
        void append(Addr mem_addr);
        void create_list(Addr mem_addr);
        int add_begin(Addr mem_addr);
        void display_list();

        void computeDelta(Addr currentAddress);
        bool correlationHit();

        void traverse_and_print();
    private:
        std::map<Addr, GHBEntry* > entries;
        std::vector<int> delta_buffer;
};

static GHBTable table;

void GHBTable::create_list(Addr mem_addr){
    struct GHBEntry *s, *temp;
    temp = new GHBEntry(mem_addr);
    //temp->mem_addr = NULL;
    temp->next = NULL;
    if (head == NULL)
    {
        temp->prev = NULL;
        head = temp;
    }else{
        s = head;
        while (s->next != NULL)
            s = s->next;
        s->next = temp;
        temp->prev = s;
    }
}

int GHBTable::add_begin(Addr currentAddress){
    if(head == NULL){
        cout << "First Create the list." << endl;
        GHBTable::create_list(currentAddress);
        return -1;
    }
    struct GHBEntry *temp;
    temp = new (struct GHBEntry);
    temp->prev = NULL;
    temp->mem_addr = currentAddress;
    temp->delta = currentAddress - head->mem_addr; // calculate delta
    temp->next = head;
    head->prev = temp;
    head = temp;
    cout << "Element inserted" << endl;
    return temp->delta;
}

Addr GHBTable::calculatePrefetchAddr(Addr mem_addr){

    int d = table.add_begin(mem_addr);
    delta_buffer.push_back(d);

    for(vector<int>::const_iterator i = delta_buffer.begin(); i != delta_buffer.end(); i++){
        cout << *i << ' ';
    }
    cout << endl;

    return 0;

}

void GHBTable::display_list(){
    struct GHBEntry * q;
    if(head == NULL){
        cout << "List empty, nothing to display" << endl;
        return;
    }
    q = head;
    cout << " GHB Table from head :" << endl;
    while(q != NULL){
        cout << q->mem_addr << " <-> ";
        q = q->next;
    }
    cout << "TAIL" << endl;
    q = head;
    while(q != NULL){
        cout << q->delta << " <-> ";
        q = q->next;
    }

    cout << "TAIL" << endl;
}

void GHBTable::computeDelta(Addr currentAddress){
    GHBEntry *entry = this->entries[currentAddress];
    entry->delta = currentAddress - entry->mem_addr;
}

// --------- PREFETCH SIMULATED FUNCTIONS ------------------------
void prefetch_init(void){
    std::cout << "prefetch_init" << std::endl;
}

void prefetch_access(AccessStat stat)
{
    Addr pf_addr;

    //GHBEntry *entry = table->get(stat.mem_addr);
    if(stat.miss){
        pf_addr = table.calculatePrefetchAddr(stat.mem_addr);

        if(pf_addr < MAX_PHYS_MEM_ADDR){
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
    //table.create_list();



    int miss_addresses[7] = {47,49,54,56,58,63,65};
    for (int i = 0; i < 7; i++ ){
        stat.pc = 55;
        stat.mem_addr = miss_addresses[i];
        stat.time = 1;
        stat.miss = 1;
        prefetch_access(stat);
    }

    table.display_list();


    prefetch_complete(stat.mem_addr);
    return 0;
}
