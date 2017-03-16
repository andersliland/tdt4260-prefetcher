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


using std::cout;
using std::endl;


class GHBEntry{

    friend class GHBTable;

private:
    static const int MAX_ENTRIES = 256;
    static const int ADDR_BITWIDTH = 10;

    Addr _missAddress, _lastAddress, _lastPrefetch;
    GHBEntry *_pNext, *_pPrev; // pointer to next node, _variableName means private member variable

public:
    // constructor with no arguments
    GHBEntry(void) : _pNext(NULL){}
    // constructor with a given value
    GHBEntry(Addr mem_addr): _missAddress(mem_addr), _pNext(NULL){}
    // constructor with a given value and a link of the next node (default value constructor)
    GHBEntry(Addr mem_addr, GHBEntry* next) : _missAddress(mem_addr), _pNext(next) {}

    // getters
    int getMissAddress(void)
    {return _missAddress;}

    GHBEntry* getNext(void)
    { return _pNext; }


};

//GHBEntry::GHBEntry(Addr mem_addr){
//    cout << "mem_addr" << mem_addr << endl;
//}

class GHBTable{
private:
    // pointer to head of node
    GHBEntry *_pHead;
    GHBEntry *_pTail;

    void push_head(GHBEntry*);
    void pop(void);


public:
    // constructor with no arguments
    GHBTable(void);

    // constructor with a given value of a list node
    GHBTable(Addr mem_addr);
    // destructor
    ~GHBTable(void);

    // traversing the list and printing the value of each node
    void traverse_and_print();
    void append(Addr pc, Addr mem_addr);
    void update(GHBEntry *);
};

GHBTable::GHBTable(){
    // initialise the head and tail node
    _pHead = _pTail = NULL;

}
GHBTable::GHBTable(Addr mem_addr){
    // create a new node, acting as both the head and tail node
    _pHead = new GHBEntry(mem_addr);
    _pTail = _pHead;

}

GHBTable::~GHBTable(){

    // empty for now
    // used to delete linked list
}

void GHBTable::traverse_and_print(){

    GHBEntry *p = _pHead;
    // is the list empty?
    if (_pHead == NULL) {
        cout << "The GHB Table is empty" << endl;
        return;
    }

    cout << "GHB Table: ";
    // a basic way of traversing a linked list
    while(p != NULL){
        // output value
        cout << p-> _missAddress;
        // the pointer moves along to the next one
        p = p-> _pNext;
    }

    cout << endl;


}

// Pushes an GHBTable entry to head of the table (head is the newest value in the table)
void GHBTable::push_head(GHBEntry * entry){
    if(entry->_pPrev == NULL){ // entry is already head
        return;
    } else if(entry-> _pNext == NULL){ // entry is tail
        this->_pTail == entry->_pPrev;
        entry->_pNext = this->_pHead;
    }else{
        entry->_pNext->_pPrev = entry->_pPrev;
        entry-> _pPrev -> _pNext = entry-> _pNext;

    }
    this->_pHead = entry;
}




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
    //std::cout << "There have been a cache access" << std::endl;
    if (stat.miss == 1) {
        std::cout << "cache miss at mem_addr: " << stat.mem_addr << std::endl;
    }else {
        std::cout << "cache hit  at mem_addr: " << stat.mem_addr << std::endl;
    }

    //    save value in GHB linked list

}


void prefetch_complete(Addr addr)
{
    std::cout << "prefetch_complete" << std::endl;
}


int main( ) {


    Addr temp = 10;

    // create an empty GHB Table
    GHBTable ghb_table(temp);
    ghb_table.traverse_and_print();
    ghb_table.traverse_and_print();




    prefetch_init();


    AccessStat stat;
    int miss_addresses[7] = {47,49,54,56,58,63,65};
    for (int i = 0; i < 7; i++ ){
        stat.pc = 100;
        stat.mem_addr = miss_addresses[i];
        stat.time = 1;
        stat.miss = 1;
        prefetch_access(stat);
    }

    //prefetch_access();
    //prefetch_complete(stat.pc);

    return 0;
}
