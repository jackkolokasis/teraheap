#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "../include/regions.h"
#include "../include/sharedDefines.h"

#define HEAPWORD (8)                       // In the JVM the heap is aligned to 8 words

struct _mem_pool tc_mem_pool;

#define align_size_up_(size, alignment) (((size) + ((alignment) - 1)) & ~((alignment) - 1))

intptr_t align_size_up(intptr_t size, intptr_t alignment) {
	return align_size_up_(size, alignment);
}

void* align_ptr_up(void* ptr, size_t alignment) {
	return (void*)align_size_up((intptr_t)ptr, (intptr_t)alignment);
}

// Initialize allocator
void init(uint64_t align) {
    int fd = -1;

    fd = open(DEV, O_RDWR);

	// Memory-mapped a raw device
	tc_mem_pool.mmap_start = mmap(0, DEV_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	assertf(tc_mem_pool.mmap_start != MAP_FAILED, "Mapping Failed");

	// Card table in JVM needs the start address of TeraCache to be align up
	tc_mem_pool.start_address = (char *) align_ptr_up(tc_mem_pool.mmap_start, align);

	tc_mem_pool.cur_alloc_ptr = tc_mem_pool.start_address;
	tc_mem_pool.size = 0;
	tc_mem_pool.stop_address = tc_mem_pool.mmap_start + DEV_SIZE;
}


// Return the start address of the memory allocation pool
char* start_addr_mem_pool() {
	assertf(tc_mem_pool.start_address != NULL, "Start address is NULL");
	return tc_mem_pool.start_address;
}

// Return the last address of the memory allocation pool
char* stop_addr_mem_pool() {
	assertf(tc_mem_pool.stop_address != NULL, "Stop address is NULL");
	return tc_mem_pool.stop_address;
}

// Return the `size` of the memory allocation pool
size_t mem_pool_size() {
	assertf(tc_mem_pool.start_address != NULL, "Start address is NULL");
	return DEV_SIZE;
}

// Allocate a new object with `size` and return the `start allocation
// address`.
// Args: `size` is in words, each word is 8 byte. Size should be > 0
char* allocate(size_t size) {
	char* alloc_ptr = tc_mem_pool.cur_alloc_ptr;

	assertf(alloc_ptr >= tc_mem_pool.start_address &&
			alloc_ptr < tc_mem_pool.stop_address, "TeraCache out-of-space");
	assertf(size > 0, "Object should be > 0");


	tc_mem_pool.cur_alloc_ptr = (char *) (((uint64_t) alloc_ptr) + size * HEAPWORD);
	tc_mem_pool.size += size;

	// Alighn to 8 words the pointer
	if ((uint64_t) tc_mem_pool.cur_alloc_ptr % HEAPWORD != 0)
		tc_mem_pool.cur_alloc_ptr = (char *)((((uint64_t)tc_mem_pool.cur_alloc_ptr) + (HEAPWORD - 1)) & -HEAPWORD);

	assertf(tc_mem_pool.cur_alloc_ptr < tc_mem_pool.stop_address,
			"TeraCache if full");

	return alloc_ptr;
}

// Return the current allocation pointer
char* cur_alloc_ptr() {
	assertf(tc_mem_pool.cur_alloc_ptr >= tc_mem_pool.start_address
			&& tc_mem_pool.cur_alloc_ptr < tc_mem_pool.stop_address,
			"Allocation pointer out-of-bound")

	return tc_mem_pool.cur_alloc_ptr;
}

// Return 'true' if the allocator is empty, 'false' otherwise.
// Invariant: Initialize allocator
int r_is_empty() {
	assertf(tc_mem_pool.start_address != NULL, "Allocator should be initialized");

	return tc_mem_pool.size == 0;
}

// Close allocator and unmap pages
void r_shutdown(void) {
	printf("CALL HERE");
	munmap(tc_mem_pool.mmap_start, DEV_SIZE);
}
