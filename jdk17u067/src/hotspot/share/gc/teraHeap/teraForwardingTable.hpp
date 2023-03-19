#ifndef SHARE_GC_TERAHEAP_TERAFORWARDINGTABLE_HPP
#define SHARE_GC_TERAHEAP_TERAFORWARDINGTABLE_HPP

#include "utilities/hashtable.hpp"
#include "utilities/hashtable.inline.hpp"

// Hashtable to record oops used for JvmtiTagMap
class TeraForwardingTableEntry : public HashtableEntry<HeapWord *, mtGC> {
public:
  TeraForwardingTableEntry* next() const {
    return (TeraForwardingTableEntry*)HashtableEntry<HeapWord *, mtGC>::next();
  }

  TeraForwardingTableEntry** next_addr() {
    return (TeraForwardingTableEntry**)HashtableEntry<HeapWord *, mtGC>::next_addr();
  }
}; 

class TeraForwardingTable : public Hashtable<HeapWord *, mtGC> {
  enum Constants {
    _table_size = 100
  };

private:
  TeraForwardingTableEntry* bucket(int i) {
    return (TeraForwardingTableEntry*) Hashtable<HeapWord *, mtGC>::bucket(i);
  }
  
  TeraForwardingTableEntry** bucket_addr(int i) {
    return (TeraForwardingTableEntry**) Hashtable<HeapWord *, mtGC>::bucket_addr(i);
  }
  
  // Done
  TeraForwardingTableEntry* new_entry(unsigned int hash, HeapWord* w);

  // Done
  void free_entry(TeraForwardingTableEntry* entry);

  // Done
  unsigned int compute_hash(HeapWord *addr);

  // Done
  TeraForwardingTableEntry* find(int index, unsigned int hash, HeapWord *addr);

  // Done
  void resize_if_needed();

public:
  // Done
  TeraForwardingTable();

  // Done
  ~TeraForwardingTable();

  // Done
  TeraForwardingTableEntry* add(HeapWord *src_addr, HeapWord *dst_addr);
  // Done
  TeraForwardingTableEntry* find(HeapWord *src_addr);

  bool is_empty() const { return number_of_entries() == 0; }

  // Done
  void clear();
};

#endif // SHARE_GC_TERAHEAP_TERAFORWARDINGTABLE_HPP
