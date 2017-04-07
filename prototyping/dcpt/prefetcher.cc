/*
 * Awesome DCPT-based prefetcher
 */

#include "interface.hh"
#include <list>

class DCPTEntry
{
	public:
		static const int NUMBER_OF_DELTAS = 16;
		static const int DELTA_BITFIELD_WIDTH = 12;

		DCPTEntry(Addr pc);
		Addr getPC() const { return pc; }

		void miss(Addr & addr, Addr ** prefetch, int & size);
	private:
		Addr pc, lastAddress, lastPrefetch;
		int deltaArray[DCPTEntry::NUMBER_OF_DELTAS];
		int deltaNext;

		void collectPrefetchCandidates(int start, int stop, Addr ** prefetch, int & size) const;

};

class DCPTTable
{
	public:
		DCPTTable(int entries);
		~DCPTTable();

		DCPTEntry * getEntry(Addr pc);
	private:
		std::list<DCPTEntry *> table;
		int entries;
};

static DCPTTable * table;

#define DELTAPTR_INC(x)	((x + NUMBER_OF_DELTAS + 1) % NUMBER_OF_DELTAS)
#define DELTAPTR_DEC(x)	((x + NUMBER_OF_DELTAS - 1) % NUMBER_OF_DELTAS)

using namespace std;

DCPTEntry::DCPTEntry(Addr pc) : pc(pc), lastAddress(0), lastPrefetch(0), deltaNext(0)
{
	memset(deltaArray, 0, NUMBER_OF_DELTAS * sizeof(int));
}

void DCPTEntry::miss(Addr & addr, Addr ** prefetch, int & size)
{
	int delta = addr - lastAddress;
	delta /= BLOCK_SIZE;
	delta &= ~(-(1 << DELTA_BITFIELD_WIDTH));

	if(delta == 0)
		return;

	deltaArray[deltaNext] = delta;

	int start = DELTAPTR_DEC(deltaNext);
	deltaNext = DELTAPTR_INC(deltaNext);

	int a = deltaArray[start], b = delta;

	*prefetch = 0;
	size = 0;

	for(int i = start; i != DELTAPTR_DEC(deltaNext); i = DELTAPTR_DEC(i))
	{
		if(deltaArray[DELTAPTR_DEC(i)] == a && deltaArray[i] == b)
		{
			collectPrefetchCandidates(DELTAPTR_INC(i), start, prefetch, size);
			lastPrefetch = (*prefetch)[size - 1];
			break;
		}
	}
}

void DCPTEntry::collectPrefetchCandidates(int start, int stop, Addr ** prefetch, int & size) const
{
	list<Addr> candidates;
	list<Addr>::iterator it;
	int prevAddress = lastAddress;
	int a = 0;

	for(int i = start; i != stop; i = DELTAPTR_INC(i))
	{
		candidates.push_back(prevAddress + deltaArray[i] * BLOCK_SIZE);
		prevAddress += deltaArray[i] * BLOCK_SIZE;
	}

	for(it = candidates.begin(); it != candidates.end(); ++it)
	{
		Addr addr = *it;
		if(addr == lastPrefetch)
			it = candidates.erase(it, candidates.end());
	}

	*prefetch = new Addr[candidates.size()];
	for(it = candidates.begin(); it != candidates.end(); ++it)
		(*prefetch)[a++] = *it;
	size = candidates.size();
}

DCPTTable::DCPTTable(int entries) : entries(entries)
{

}

DCPTEntry * DCPTTable::getEntry(Addr pc)
{
	list<DCPTEntry *>::iterator i = table.begin();
	for(; i != table.end(); ++i)
	{
		DCPTEntry * entry = *i;
		if(pc == entry->getPC())
		{
			i = table.erase(i);
			table.push_front(entry);
			return entry;
		}
	}

	DCPTEntry * newEntry = new DCPTEntry(pc);
	table.push_front(newEntry);

	if(table.size() > entries)
	{
		DCPTEntry * last = table.back();
		delete last;
		table.pop_back();
	}

	return newEntry;
}

void prefetch_init(void)
{
	/* Called before any calls to prefetch_access. */
	/* This is the place to initialize data structures. */

	table = new DCPTTable(82);
	//DPRINTF(HWPrefetch, "Initialized DCPT-based prefetcher\n");
}

void prefetch_access(AccessStat stat)
{
	int size = 0;
	Addr * prefetchList = 0;

	if(stat.miss)
	{
		DCPTEntry * entry = table->getEntry(stat.pc);
		entry->miss(stat.mem_addr, &prefetchList, size);
	}

	if(prefetchList != 0)
	{
		if(current_queue_size() >= MAX_QUEUE_SIZE - size)
			delete[] prefetchList;
		else {
			for(int i = 0; i < size; ++i)
			{
				if(!in_cache(prefetchList[i]) && !in_mshr_queue(prefetchList[i]) && prefetchList[i] < MAX_PHYS_MEM_ADDR)
					issue_prefetch(prefetchList[i]);
			}
			delete[] prefetchList;
		}
	} else {
		if(stat.miss)
		{
			if(!in_cache(stat.mem_addr + BLOCK_SIZE))
				issue_prefetch(stat.mem_addr + BLOCK_SIZE);
			if(!in_cache(stat.mem_addr + BLOCK_SIZE * 2))
				issue_prefetch(stat.mem_addr + BLOCK_SIZE * 2);
		} else {
			if(!in_cache(stat.mem_addr + BLOCK_SIZE))
				issue_prefetch(stat.mem_addr + BLOCK_SIZE);
			if(!in_cache(stat.mem_addr + BLOCK_SIZE * 2))
				issue_prefetch(stat.mem_addr + BLOCK_SIZE * 2);
		}
	}
}

void prefetch_complete(Addr addr)
{

}


// ######## REMOVE BEFORE SIMULATOR ######
int main( ) {
    AccessStat stat;
    prefetch_init();

    int pc[12] = {1,2,3,4,5,6,7};
    int miss_addresses[12] = {1147,1245,1149,1250,1255, 1260,1270,1154,1156,1158,1163,1165};

    for (int i = 0; i < 12; i++ ){
        stat.pc = pc[i];
        stat.mem_addr = miss_addresses[i];
        stat.time = 1;
        stat.miss = 1;
        prefetch_access(stat);
    }
    prefetch_complete(stat.mem_addr);
    return 0;
}
