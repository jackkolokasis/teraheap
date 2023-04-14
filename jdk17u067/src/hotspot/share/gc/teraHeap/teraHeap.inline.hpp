#include "gc/teraHeap/teraHeap.hpp"
#include <tera_allocator.h>

// Request an address from the allocator. Sizes should be up to 24bytes.
// Eeach entry in the forwarding table is 24bytes
inline char* TeraHeap::tera_dram_malloc(size_t size) {
  return tera_malloc((uint64_t) size);
}
