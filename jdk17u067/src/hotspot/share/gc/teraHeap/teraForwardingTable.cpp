#include "precompiled.hpp"
#include "gc/teraHeap/teraForwardingTable.hpp"
#include "gc/shared/collectedHeap.hpp"
//#include "gc/teraHeap/teraForwardingTable.hpp"
//#include "memory/universe.hpp"
//#include "utilities/hashtable.inline.hpp"
//#include "gc/shared/oopStorage.hpp"
#include "gc/teraHeap/teraHeap.hpp"
//#include "jvmtifiles/jvmtiEnv.hpp"
#include "logging/log.hpp"
//#include "memory/allocation.hpp"
//#include "memory/resourceArea.hpp"
//#include "oops/oop.inline.hpp"
//#include "oops/weakHandle.inline.hpp"
//#include "prims/jvmtiEventController.inline.hpp"
//#include "prims/jvmtiExport.hpp"
#include "prims/jvmtiTagMapTable.hpp"
#include "utilities/terahashtable.hpp"
#include "utilities/terahashtable.inline.hpp"
#include "utilities/macros.hpp"

unsigned int TeraForwardingTable::compute_hash(HeapWord *addr) {
  assert(addr != NULL, "obj is null");
  oop obj = cast_to_oop(addr);
  return Universe::heap()->hash_oop(obj);
}

// Entries are C_Heap allocated
TeraForwardingTableEntry* TeraForwardingTable::new_entry(unsigned int hash, HeapWord *h2_addr) {
  TeraForwardingTableEntry* entry = (TeraForwardingTableEntry*)TeraHashtable<HeapWord *, mtNMT>::new_entry(hash, h2_addr);
  return entry;
}

// Add a new entry in the table
TeraForwardingTableEntry* TeraForwardingTable::add(HeapWord *src_addr, HeapWord *dst_addr) {
  unsigned int hash = compute_hash(src_addr);
  int index = hash_to_index(hash);

  // One was added while acquiring the lock
  assert(find(index, hash, dst_addr) == NULL, "shouldn't already be present");
  TeraForwardingTableEntry* entry = new_entry(hash, dst_addr);
  TeraHashtable<HeapWord *, mtNMT>::add_entry(index, entry);
  log_trace(teraheap, fdtable)("TeraForwardingTable: Key = %d | Value = %p", index, dst_addr);
  
  // Resize if the table is getting too big.
  //resize_if_needed();

  return entry;
}

TeraForwardingTableEntry* TeraForwardingTable::find(int index, unsigned int hash, HeapWord *src_addr) {
  assert(src_addr != NULL, "Cannot search for a NULL address");

  for (TeraForwardingTableEntry* e = bucket(index); e != NULL; e = e->next()) {
    if (e->hash() == hash) {
      log_trace(teraheap, fdtable)("Entry Found | Src Addres = %p | Index = %d | Hash = %u", src_addr, index, hash);
      return e;
    }
  }
  log_trace(teraheap, fdtable)("Entry not found | Src Addres = %p | Index = %d | Hash = %u", src_addr, index, hash);
  return NULL;
}

TeraForwardingTableEntry* TeraForwardingTable::find(HeapWord *src_addr) {
  unsigned int hash = compute_hash(src_addr);
  int index = hash_to_index(hash);
  return find(index, hash, src_addr);
}

const int _resize_load_trigger = 5;       // load factor that will trigger the resize
static bool _resizable = true;

void TeraForwardingTable::resize_if_needed() {
  if (_resizable && number_of_entries() > (_resize_load_trigger * table_size())) {
    int desired_size = calculate_resize(true);
    if (desired_size == table_size()) {
      _resizable = false; // hit max
    } else {
      if (!resize(desired_size)) {
        // Something went wrong, turn resizing off
        _resizable = false;
      }
      log_trace(teraheap, fdtable)("TeraForwardingTable table resized to %d", table_size());
    }
  }
}

void TeraForwardingTable::free_entry(TeraForwardingTableEntry* entry) {
  TeraBasicHashtable<mtNMT>::free_entry(entry);
}

void TeraForwardingTable::clear() {
  // Clear this table
  log_trace(teraheap, fdtable)("JvmtiTagMapTable cleared");
  for (int i = 0; i < table_size(); ++i) {
    for (TeraForwardingTableEntry* m = bucket(i); m != NULL;) {
      TeraForwardingTableEntry* entry = m;
      // read next before freeing.
      m = m->next();
      free_entry(entry);
    }
    TeraForwardingTableEntry** p = bucket_addr(i);
    *p = NULL; // clear out buckets.
  }
  assert(number_of_entries() == 0, "should have removed all entries");
}

TeraForwardingTable::TeraForwardingTable()
  : TeraHashtable<HeapWord *, mtNMT>(_table_size, sizeof(TeraForwardingTableEntry)) {}

TeraForwardingTable::~TeraForwardingTable() {
  //clear();
  // base class ~TeraBasicHashtable deallocates the buckets.
}
