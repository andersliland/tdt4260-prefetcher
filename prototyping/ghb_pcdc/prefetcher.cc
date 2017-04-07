#ifndef _GHB_PCDC_H_
#define _GHB_PCDC_H_

#include "interface.hh"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <set>
#include <vector>


struct PrefetchDecision
{
    explicit PrefetchDecision()
        : prefetchAddresses() { /*empty */}

    explicit PrefetchDecision(const std::vector<Addr> &addrs)
        : prefetchAddresses(addrs) { /* empty */ }

    std::vector<Addr> prefetchAddresses;
};

class Prefetcher
{
public:
    virtual ~Prefetcher() {};
    virtual unsigned int prefetch_attempts() const = 0;
    virtual unsigned int prefetch_hits() const = 0;
    virtual void increase_aggressiveness() = 0;
    virtual void decrease_aggressiveness() = 0;
    virtual PrefetchDecision react_to_access(AccessStat stat) = 0;
    virtual void prefetch_complete(Addr addr) {};
};

struct TableEntry
{
    explicit TableEntry() : address(0x0), previousMiss(0)
        {
            DPRINTF(HWPrefetch, "Creating new TableEntry()\n");
        }
    explicit TableEntry(Addr addr, TableEntry *prevMiss)
        : address(addr), previousMiss(prevMiss)
        {
            DPRINTF(HWPrefetch,
                    "Creating new TableEntry(addr, prev)\n");
        }
    ~TableEntry()
        {
            DPRINTF(HWPrefetch, "Destroyhing a TableEntry\n");
        }
    Addr address;
    //Pointer member, does this mean we need copy ctor, assignment op
    //and destructor? In this case, it's a pointer we do not own so
    //we should not delete it when the TableEntry is deleted, and when
    //copying a TableEntry it makes sense to copy the pointer instead
    //of its value - right..?
    TableEntry *previousMiss;
// private:
//     //Disallow copying..?
//     TableEntry& operator=(const TableEntry&);
//     TableEntry(const TableEntry&);

};

template <unsigned int TableSize>
class GlobalHistoryBuffer
{
public:
    explicit GlobalHistoryBuffer()
        : head_(0), evictingOldEntry_(false)
        {
            DPRINTF(HWPrefetch, "Creating new GHB\n");
            memset (buffer_, 0, sizeof(buffer_));
        }
    TableEntry* insert(const AccessStat &stat, TableEntry *previousMiss);
private:
    TableEntry* findFirstEntryReferencing(const TableEntry * const e);
    std::size_t head_;
    bool evictingOldEntry_;
    TableEntry buffer_[TableSize];
    //Disallow copying
    GlobalHistoryBuffer& operator=(const GlobalHistoryBuffer&);
    GlobalHistoryBuffer(const GlobalHistoryBuffer&);
};

template<unsigned int TableSize>
TableEntry* GlobalHistoryBuffer<TableSize>::findFirstEntryReferencing(const TableEntry *const e)
{
    TableEntry *first = 0;
    unsigned int num = 0;
    for (int i = head_; i >= 0; i--)
    {
        if (buffer_[i].previousMiss == e)
        {
            if (num == 0)
            {
                first = &buffer_[i];
            }
            num++;
        }
    }
    for (int i = TableSize - 1; i > head_; i--)
    {
        if (buffer_[i].previousMiss == e)
        {
            if (num == 0)
            {
                first =  &buffer_[i];
            }
            num++;
        }
    }
    assert(num <= 1 &&
           "Should not have more access to same previous miss.");
    return first;
}


template <unsigned int TableSize>
TableEntry* GlobalHistoryBuffer<TableSize>::insert(
    const AccessStat &stat,
    TableEntry *previousMiss)
{
    if (evictingOldEntry_)
    {
        assert(buffer_[head_].previousMiss == 0 &&
               "Evicted entry is first in the queue, and "
               "should not point to any elements in front");
        TableEntry *e = findFirstEntryReferencing(&buffer_[head_]);
        if (e != 0)
        {
            DPRINTF(HWPrefetch,
                    "Kicking out referenced element, unlinking it\n");
            e->previousMiss = 0;
        }
        else
        {
            DPRINTF(HWPrefetch, "Element was not referenced\n");
        }
        //If we are kicking out the previous miss, do not link to it...
        if (&buffer_[head_] == previousMiss)
        {
            previousMiss = 0;
        }
    }
//    buffer_[head_] = TableEntry(stat.mem_addr, previousMiss);
//    TableEntry *newEntry = &buffer_[head_];
    TableEntry *newEntry =
        new (&buffer_[head_]) TableEntry(stat.mem_addr, previousMiss);
    head_++;
    if (head_ >= TableSize)
    {
        head_ = 0;
        evictingOldEntry_ = true;
    }
    return newEntry;
}

template <unsigned int TableSize>
class IndexTable
{
public:
    explicit IndexTable()
        : head_(0)
        {
            DPRINTF(HWPrefetch, "Creating new Index Table\n");
            memset(buffer_, 0, sizeof(buffer_));
        }

    //Overload [] instead? :x
    TableEntry* previousAccessTo(Addr pc) const;
    void setPreviousAccessTo(Addr pc, TableEntry *e);
private:
    TableEntry buffer_[TableSize];
    std::size_t head_;
    //Disallow copying
    IndexTable& operator=(const IndexTable&);
    IndexTable(const IndexTable&);
};

template<unsigned int TableSize>
TableEntry* IndexTable<TableSize>::previousAccessTo(Addr pc) const
{
    std::size_t index = pc % TableSize;
    if (buffer_[index].address == pc)
    {
        return buffer_[index].previousMiss;
    }
    return 0;
}

template<unsigned int TableSize>
void IndexTable<TableSize>::setPreviousAccessTo(Addr pc, TableEntry *e)
{
    //Is this correct..?
    //Idea: If e is the address of a miss which an index table entry
    //has recorded as the address of its previous miss, then that miss
    //must have been overwritten. Thus, it should be cleared.
    for (int i = 0; i < TableSize; ++i)
    {
        if (buffer_[i].previousMiss == e)
        {
            buffer_[i].previousMiss = 0;
        }
    }
    std::size_t index = pc % TableSize;
    new (&buffer_[index]) TableEntry(pc, e);
//    buffer_[index] = TableEntry(pc, e);
}


template <unsigned int TableSize>
class GHB_PCDC : public Prefetcher
{
public:
    explicit GHB_PCDC()
        : attempts_(0),
          hits_(0),
          numBlocksToPrefetch_(2),
          historyBuffer_(),
          indexTable_()
        {
            DPRINTF(HWPrefetch, "Creating GHB_PCDC\n");
        }
    unsigned int prefetch_attempts() const { return attempts_; }
    unsigned int prefetch_hits() const { return hits_; }
    void increase_aggressiveness() {
        if (numBlocksToPrefetch_ > 1)
            numBlocksToPrefetch_--;

        DPRINTF(HWPrefetch, "Decreased aggressiveness to %u",
                numBlocksToPrefetch_);
    }
    void decrease_aggressiveness() {
        numBlocksToPrefetch_++;
        DPRINTF(HWPrefetch, "Increased aggressiveness to %u",
                numBlocksToPrefetch_);
    }
    PrefetchDecision react_to_access(AccessStat stat);
private:
    void insert(const AccessStat &stat);

    std::vector<int> compute_delta_table(
        const AccessStat &stat) const;

    int pastPreviousOccurrenceOfLastPair(const std::vector<int> &deltas) const;

    unsigned int attempts_;
    unsigned int hits_;
    unsigned int numBlocksToPrefetch_;
    GlobalHistoryBuffer<TableSize> historyBuffer_;
    IndexTable<TableSize> indexTable_;
};

template<unsigned int TableSize>
void GHB_PCDC<TableSize>::insert(const AccessStat &stat)
{
    TableEntry *lastEntry = indexTable_.previousAccessTo(stat.pc);
    TableEntry *newEntry = historyBuffer_.insert(stat, lastEntry);
    indexTable_.setPreviousAccessTo(stat.pc, newEntry);
}


template<unsigned int TableSize>
std::vector<int> GHB_PCDC<TableSize>::compute_delta_table(
    const AccessStat &stat) const
{
    std::vector<int> deltaTable(0);
    std::vector<TableEntry*> seen;
    for (TableEntry *e = indexTable_.previousAccessTo(stat.pc);
         e != 0 && e->previousMiss != 0; e = e->previousMiss)
    {
        typedef std::vector<TableEntry*>::const_iterator It;
        It prevOccurrence = find(seen.begin(), seen.end(), e);
        if (prevOccurrence != seen.end())
        {
            DPRINTF(HWPrefetch, "Error, detected cycle in GHB\n");
            DPRINTF(HWPrefetch, "Cycle: %#x", *prevOccurrence);
            ++prevOccurrence;
            for (; prevOccurrence != seen.end(); ++prevOccurrence)
            {
                DPRINTF(HWPrefetch, ", %#x", *prevOccurrence);
            }
            DPRINTF(HWPrefetch, ", %#x", e);
            abort();
        }
        seen.push_back(e);
        deltaTable.push_back(e->address - e->previousMiss->address);
        DPRINTF(HWPrefetch, "Added another delta\n");
    }
    return deltaTable;
}

template<unsigned int TableSize>
int GHB_PCDC<TableSize>::pastPreviousOccurrenceOfLastPair(
    const std::vector<int> &deltas) const
{
    if (deltas.size() < 4) return -1;

    int d1 = deltas.at(deltas.size() - 2),
        d2 = deltas.at(deltas.size() - 1);
    for (int i = deltas.size() - 4; i >= 0; i--)
    {
        if (deltas.at(i) == d1 && deltas.at(i+1) == d2)
            return i + 2;
    }
    return -1;
}

template<unsigned int TableSize>
PrefetchDecision GHB_PCDC<TableSize>::react_to_access(AccessStat stat)
{
    //ONLY DO THIS ON A MISS! But debug cycle first...
    if (!stat.miss)
    {
        return PrefetchDecision();
    }
    insert(stat);
    std::vector<int> deltas = compute_delta_table(stat);
    DPRINTF(HWPrefetch, "Size of delta table: %u\n", deltas.size());
    int index = pastPreviousOccurrenceOfLastPair(deltas);
    if (index == -1)
    {
        return PrefetchDecision(); //Do not prefetch
    }
    else
    {
        std::vector<Addr> addrs;
        std::size_t endFetchIndex =
            static_cast<std::size_t>(index + numBlocksToPrefetch_);
        Addr prevAddr = stat.mem_addr;
        for (int i = index,
                 e = std::min(endFetchIndex, deltas.size());
             i < e; ++i)
        {
            addrs.push_back(prevAddr + deltas.at(i));
            prevAddr += deltas.at(i);
        }
        return PrefetchDecision(addrs);
    }
}

#endif /* _GHB_PCDC_H_ */
