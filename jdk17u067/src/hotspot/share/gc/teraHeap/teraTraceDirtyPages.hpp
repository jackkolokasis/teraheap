#ifndef SHARE_GC_TERAHEAP_TERATRACEDIRTYPAGES_HPP
#define SHARE_GC_TERAHEAP_TERATRACEDIRTYPAGES_HPP

#define TRACE_DIRTY_PAGES_MAGIC 'T'
#define TRACE_DIRTY_PAGES _IOW(TRACE_DIRTY_PAGES_MAGIC, 1, struct ioctl_data)

#include "gc/shared/collectedHeap.inline.hpp"

struct ioctl_data {
  unsigned long start_address;    //< Start virtual address of mmaped space
  unsigned long end_address;      //< End virtual address of mmaped space
  unsigned long *page_array;      //< Page array to be filled by the kernel
  size_t page_array_size;         //< Size of the page array
  unsigned long *num_dirty_pages; //< Number of dirty pages
};

class TeraTraceDirtyPages: public CHeapObj<mtInternal> {
private:
  struct ioctl_data data;
  int trace_fd;

  size_t get_page_cache_size(void);

public:
  TeraTraceDirtyPages(unsigned long start_address, unsigned long end_address);
  ~TeraTraceDirtyPages();

  void find_dirty_pages();
};

#endif // SHARE_GC_TERAHEAP_TERATRACEDIRTYPAGES_HPP
