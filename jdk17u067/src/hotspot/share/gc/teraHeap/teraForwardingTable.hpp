#ifndef SHARE_GC_TERAHEAP_TERAFORWARDINGTABLE_HPP
#define SHARE_GC_TERAHEAP_TERAFORWARDINGTABLE_HPP

#include "utilities/terahashtable.hpp"
#include "utilities/terahashtable.inline.hpp"

// Hashtable to record oops used for JvmtiTagMap
class TeraForwardingTableEntry : public TeraHashtableEntry<HeapWord *, mtNMT> {
public:
  TeraForwardingTableEntry* next() const {
    return (TeraForwardingTableEntry*)TeraHashtableEntry<HeapWord *, mtNMT>::next();
  }

  TeraForwardingTableEntry** next_addr() {
    return (TeraForwardingTableEntry**)TeraHashtableEntry<HeapWord *, mtNMT>::next_addr();
  }
}; 

class TeraForwardingTable : public TeraHashtable<HeapWord *, mtNMT> {
  enum Constants {
    _table_size = 1000
  };

private:
  TeraForwardingTableEntry* bucket(int i) {
    return (TeraForwardingTableEntry*) TeraHashtable<HeapWord *, mtNMT>::bucket(i);
  }
  
  TeraForwardingTableEntry** bucket_addr(int i) {
    return (TeraForwardingTableEntry**) TeraHashtable<HeapWord *, mtNMT>::bucket_addr(i);
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
