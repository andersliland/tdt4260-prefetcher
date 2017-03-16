// CZone Delta Correlation Prefetcher
// Computer Acvhitecture NTNU 2017
// Anders Liland and Adrian Ribe


#include <iostream>
#include <list>
#include <map>
#include <stdint.h>
#include <string.h>

#include "interface.hh"

#define BLOCK_SIZE 64
#define MAX_QUEUE_SIZE 100
#define MAX_PHYS_MEM_ADDR ((uint64_t)(256*1024*1024) - 1)


using std::cout;
using std::endl;
struct node
{
    Addr mem_addr;
    struct node *next;
    struct node *prev;
}*start;

class GHBEntry{
    friend class GHBTable;
private:
    Addr _missAddress, _pc , _lastAddress, _lastPrefetch;
    int delta;
    GHBEntry *_pNext, *_pPrev; // pointer to next node, _variableName means private member variable
public:
    // constructor with no arguments
    GHBEntry(Addr mem_addr) : _pNext(NULL){}
    // constructor with a given value
    GHBEntry(Addr mem_addr, Addr pc): _missAddress(mem_addr), _pc(pc), _pNext(NULL){}
    // constructor with a given value and a link of the next node (default value constructor)
    GHBEntry(Addr mem_addr, Addr pc, GHBEntry* next) : _missAddress(mem_addr), _pc(pc), _pNext(next) {}
    // getters
    int getMissAddress(void)
    {return _missAddress;}

    GHBEntry* getNext(void)
    { return _pNext; }
};

class GHBTable{
private:
    // pointer to head of node
    GHBEntry *_pHead, *_pTail;
    int numCurrentEntries;
    void push_head(GHBEntry*);
    void pop(void);
    std::map<Addr, GHBEntry* > entries;

public:
    static const int MAX_ENTRIES = 256;
    static const int ADDR_BITWIDTH = 10;
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
    GHBEntry *get(Addr pc);

    void create_list(Addr mem_addr);

};

GHBTable::GHBTable(){
    // initialise the head and tail node
    cout << " New GHBTable created" << endl;
    numCurrentEntries = 0;
    _pHead = _pTail = NULL;
}
GHBTable::GHBTable(Addr mem_addr){
    // create a new node, acting as both the head and tail node
    _pHead = new GHBEntry(mem_addr);
    _pTail = _pHead;

}
GHBTable::~GHBTable(){
    // empty for now, used to delete linked list
}
// display elements of GHB table

void GHBTable::traverse_and_print(){

    GHBEntry *p;
    if (_pHead == NULL) {
        cout << "GHB table empty, nothing to display" << endl;
        return;
    }
    p = _pHead;
    cout << "GHB Table missAddress is :" << endl;
    while(p != NULL){
        cout << p-> _missAddress << "<->";
        //cout << p-> _pc << "<->";
        // the pointer moves along to the next one
        p = p-> _pNext;
    }
    cout << "NULL" << endl;
}
// pops the last element/oldest (tail) from the list
void GHBTable::pop(){
    if(this->_pTail != NULL){
        GHBEntry * last = this->_pTail;
        this->_pTail = last->_pPrev;
        delete last;
        this->numCurrentEntries--;
    }
}
// inserts a new element at the front/head of the list
void GHBTable::append(Addr mem_addr, Addr pc){

    if( _pHead == NULL){
        cout << "First create the GHB table" << endl;
        return;
    }

    GHBEntry * entry = new GHBEntry(mem_addr, pc);
    if (entries.size() >= this->MAX_ENTRIES){ // if full
        this->pop();
        entry->_pNext = this->_pHead;
    }else if(entries.size()){ // if not empty
        entry->_pNext = this->_pHead;
    }else{ // if empty
        cout << "GHB table is empty" << endl;
        this->_pTail = entry;
    }
    this->_pHead = entry;

}

void GHBTable::create_list(Addr mem_addr){

    struct node *entry, *temp;
    temp = new(struct node);
    temp->mem_addr = mem_addr;
    temp->next = NULL;
    if (start == NULL){
        temp->prev = NULL;
        start = temp;
    }else{
        entry = start;
        while(entry->next !=NULL)
            entry = entry->next;
        entry->next = temp;
        temp->prev = entry;
    }
}


// Pushes an GHBTable entry to head of the table (head is the newest value in the table)
void GHBTable::push_head(GHBEntry * entry){
    if (_pHead == NULL){
        cout << "First create the GHB table, push" << endl;
        return;
    }else if (entry->_pPrev == NULL){ // entry is already head
        return;
    } else if(entry-> _pNext == NULL){ // entry is tail
        this->_pTail = entry->_pPrev;
        entry->_pNext = this->_pHead;
    }else{
        entry->_pNext->_pPrev = entry->_pPrev;
        entry-> _pPrev -> _pNext = entry-> _pNext;

    }
    this->_pHead = entry;
}

GHBEntry * GHBTable::get(Addr mem_addr){
    // check if entry exsist in map
    if (this->entries.find(mem_addr) == this-> entries.end()){
        cout << "no entries in GHBEntry map" << endl;
        return NULL;
    }else{
        cout << "create a new GHBentry" << endl;
        GHBEntry * entry = entries[mem_addr];
        this->push_head(entry);
        return entry;
    }
    return NULL;
}



class IndexTableEntry{

};
class IndexTable{

};

using namespace std;

static GHBTable * table;

// --------- PREFETCH SIMULATED FUNCTIONS ------------------------
void prefetch_init(void){
    std::cout << "prefetch_init" << std::endl;

    table = new GHBTable;
}

void prefetch_access(AccessStat stat)
{
    //std::cout << "There have been a cache access" << std::endl;
    GHBEntry * entry = table->get(stat.mem_addr);
    if (entry == NULL){
        cout << "GHBEntry is empty" << endl;
        table->append(stat.mem_addr, stat.pc);
        entry = table->get(stat.mem_addr);
    }

    if (stat.miss) {
        std::cout << "cache miss at mem_addr: " << stat.mem_addr << std::endl;
        table->append(stat.mem_addr, stat.pc);
    }else {
        std::cout << "cache hit  at mem_addr: " << stat.mem_addr << std::endl;
    }


    Addr pf_addr = 5;
    if (pf_addr < MAX_PHYS_MEM_ADDR) {
        cout << "prefetch issued for addr " << pf_addr << endl;
        //issue_prefetch( pf_addr);
    }
}


void prefetch_complete(Addr addr){
    cout << "prefetch_complete" << endl;
}


int main( ) {
    prefetch_init();

    AccessStat stat;
    stat.mem_addr = 99;
    GHBTable->create_list(stat.mem_addr)


    int miss_addresses[7] = {47,49,54,56,58,63,65};
    for (int i = 0; i < 7; i++ ){
        stat.pc = 55;
        stat.mem_addr = miss_addresses[i];
        stat.time = 1;
        stat.miss = 1;
        prefetch_access(stat);
    }
    table->traverse_and_print();
    prefetch_complete(stat.pc);
    return 0;
}
