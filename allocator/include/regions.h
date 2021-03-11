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

	// Initialize allocator with start address 'heap_end + 1'. The end of the
	// heap.
	void         init(uint64_t alignment);
	
	// Return the start address of the memory allocation pool
	char*        start_addr_mem_pool(void);
	
	// Return the start address of the memory allocation pool
	size_t       mem_pool_size(void);
	
	// Allocate a new object with `size` and return the `start allocation
	// address`.
	char *       allocate(size_t size);
	
	// Return the last address of the memory allocation pool
	char*        stop_addr_mem_pool(void);
	
	// Return the current allocation pointer
	char*        cur_alloc_ptr(void);
	
	// Return 'true' if the allocator is empty, 'false' otherwise.
	int        r_is_empty(void);
	
	// Close allocator and unmap pages
	void        r_shutdown(void);
	
#ifdef __cplusplus
}
#endif

#endif /* __REGIONS_H__ */
