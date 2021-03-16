#ifndef __REGIONS_H__
#define __REGIONS_H__

#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined (__ia64__) || defined (__x86_64__)
#define INT_PTR unsigned long

#else
#define INT_PTR unsigned int
#endif

	struct _mem_pool{
		char *mmap_start;
		char* start_address;
		char* cur_alloc_ptr;
		char* stop_address;

		uint64_t size;
	};

	extern struct _mem_pool tc_mem_pool;
	extern int fd;

	// Initialize allocator with start address 'heap_end + 1'. The end of the
	// heap.
	void       init(uint64_t alignment);
	
	// Return the start address of the memory allocation pool
	char*      start_addr_mem_pool(void);
	
	// Return the start address of the memory allocation pool
	size_t     mem_pool_size(void);
	
	// Allocate a new object with `size` and return the `start allocation
	// address`.
	char *     allocate(size_t size);
	
	// Return the last address of the memory allocation pool
	char*      stop_addr_mem_pool(void);
	
	// Return the current allocation pointer
	char*      cur_alloc_ptr(void);
	
	// Return 'true' if the allocator is empty, 'false' otherwise.
	int        r_is_empty(void);
	
	// Close allocator and unmap pages
	void	   r_shutdown(void);

	// Give advise to kernel to expect page references in sequential order.  (Hence,
	// pages in the given range can be aggressively read ahead, and may be freed
	// soon after they are accessed.)
	void	   r_enable_seq(void);
	
	// Give advise to kernel to expect page references in random order (Hence, read
	// ahead may be less useful than normally.)
	void	   r_enable_rand(void);

	// Explicit write 'data' with 'size' in certain 'offset' using system call
	// without memcpy.
	void	   r_write(char *data, char *offset, size_t size);
	
#ifdef __cplusplus
}
#endif

#endif /* __REGIONS_H__ */
