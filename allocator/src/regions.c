#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <aio.h>

#include "../include/regions.h"
#include "../include/sharedDefines.h"
#include "../include/asyncIO.h"
#include "../include/segments.h"

#define HEAPWORD (8)                       // In the JVM the heap is aligned to 8 words
#define HEADER_SIZE (32)                   // Header size of the Dummy object	
#define align_size_up_(size, alignment) (((size) + ((alignment) - 1)) & ~((alignment) - 1))

struct _mem_pool tc_mem_pool;
int fd;

intptr_t align_size_up(intptr_t size, intptr_t alignment) {
	return align_size_up_(size, alignment);
}

void* align_ptr_up(void* ptr, size_t alignment) {
	return (void*)align_size_up((intptr_t)ptr, (intptr_t)alignment);
}

// Initialize allocator
void init(uint64_t align) {
    fd = -1;

#if ANONYMOUS
	// Anonymous mmap
	tc_mem_pool.mmap_start = mmap(0, DEV_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, fd, 0);
#else
    fd = open(DEV, O_RDWR);
	// Memory-mapped a file over a storage device
	tc_mem_pool.mmap_start = mmap(0, DEV_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
#endif

	assertf(tc_mem_pool.mmap_start != MAP_FAILED, "Mapping Failed");

	// Card table in JVM needs the start address of TeraCache to be align up
	tc_mem_pool.start_address = (char *) align_ptr_up(tc_mem_pool.mmap_start, align);

	tc_mem_pool.cur_alloc_ptr = tc_mem_pool.start_address;
	tc_mem_pool.size = 0;
	tc_mem_pool.stop_address = tc_mem_pool.mmap_start + DEV_SIZE;
    init_regions();
	req_init();

#if ALIGN_ON
	tc_mem_pool.region_free_space = REGION_SIZE - HEADER_SIZE;
#endif
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

#if SPARK_HINT
char* allocate(size_t size, uint64_t rdd_id) {
	char* alloc_ptr = tc_mem_pool.cur_alloc_ptr;

	//assertf(alloc_ptr >= tc_mem_pool.start_address &&
	//		alloc_ptr < tc_mem_pool.stop_address, "TeraCache out-of-space");
	assertf(size > 0, "Object should be > 0");
    #if REGIONS
    alloc_ptr = allocate_to_region(size * HEAPWORD, rdd_id);
    if (alloc_ptr == NULL)
        printf("Total cached data:%zu\n",tc_mem_pool.size);
    assertf(alloc_ptr != NULL, "alloc_ptr is NULL");
    #endif
    tc_mem_pool.size += size;
    tc_mem_pool.cur_alloc_ptr = (char *) (((uint64_t) alloc_ptr) + size * HEAPWORD);

	// Alighn to 8 words the pointer (TODO: CHANGE TO ASSERTION)
	if ((uint64_t) tc_mem_pool.cur_alloc_ptr % HEAPWORD != 0)
		tc_mem_pool.cur_alloc_ptr = (char *)((((uint64_t)tc_mem_pool.cur_alloc_ptr) + (HEAPWORD - 1)) & -HEAPWORD);

	//assertf(tc_mem_pool.cur_alloc_ptr < tc_mem_pool.stop_address,
	//		"TeraCache if full");

	return alloc_ptr;
}
#else
// Allocate a new object with `size` and return the `start allocation
// address`.
// Args: `size` is in words, each word is 8 byte. Size should be > 0
char* allocate(size_t size) {
	char* alloc_ptr = tc_mem_pool.cur_alloc_ptr;
	size_t obj_size = size * HEAPWORD;

	assertf(alloc_ptr >= tc_mem_pool.start_address &&
			alloc_ptr < tc_mem_pool.stop_address, "TeraCache out-of-space");
	assertf(obj_size > 0, "Object should be > 0");

#if REGIONS && !ALIGN_ON
    alloc_ptr = allocate_to_region(obj_size);
    if (alloc_ptr == NULL)
        printf("Total cached data:%zu\n",tc_mem_pool.size);
    assertf(alloc_ptr != NULL, "alloc_ptr is NULL");
#endif

#if ALIGN_ON && !REGIONS
	assertf(obj_size <= (REGION_SIZE - HEADER_SIZE), "Oject size: %lu exceeds region size: %lu", 
			obj_size, REGION_SIZE - HEADER_SIZE);
#endif

	tc_mem_pool.cur_alloc_ptr = (char *) (((uint64_t) alloc_ptr) + obj_size);
	tc_mem_pool.size += size;

#if ALIGN_ON && !REGIONS
	// Decrease the available free space counter for the region
	tc_mem_pool.region_free_space -= obj_size;
#endif
	
	// Alighn to 8 words the pointer
	if ((uint64_t) tc_mem_pool.cur_alloc_ptr % HEAPWORD != 0)
		tc_mem_pool.cur_alloc_ptr = (char *)((((uint64_t)tc_mem_pool.cur_alloc_ptr) + (HEAPWORD - 1)) & -HEAPWORD);

	assertf(tc_mem_pool.cur_alloc_ptr < tc_mem_pool.stop_address,
			"TeraCache if full");

	return alloc_ptr;
}
#endif

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

// Give advise to kernel to expect page references in sequential order.  (Hence,
// pages in the given range can be aggressively read ahead, and may be freed
// soon after they are accessed.)
void r_enable_seq() {
	madvise(tc_mem_pool.mmap_start, DEV_SIZE, MADV_SEQUENTIAL);
}

// Give advise to kernel to expect page references in random order (Hence, read
// ahead may be less useful than normally.)
void r_enable_rand() {
	madvise(tc_mem_pool.mmap_start, DEV_SIZE, MADV_NORMAL);
}

// Explicit write 'data' with 'size' in certain 'offset' using system call
// without memcpy.
void r_write(char *data, char *offset, size_t size) {
	ssize_t s_check;
	uint64_t diff = offset - tc_mem_pool.mmap_start;

	s_check = pwrite(fd, data, size * HEAPWORD, diff);
	assertf(s_check == size * HEAPWORD, "Sanity check: s_check = %ld", s_check);
}
	
// Explicit asynchronous write 'data' with 'size' in certain 'offset' using
// system call without memcpy.
// Do not use r_awrite with r_write
void r_awrite(char *data, char *offset, size_t size) {
	
	uint64_t diff = offset - tc_mem_pool.mmap_start;

	req_add(fd, data, size * HEAPWORD, diff);
}
	
// Check if all the asynchronous requestes have been completed
// Return 1 on succesfull, and 0 otherwise
int	r_areq_completed() {
	return is_all_req_completed();
}
	
// We need to ensure that all the writes will be flushed from buffer
// cur_alloc_ptrcur_alloc_ptrhe and they will be written to the device.
void r_fsync() {
	int check = fsync(fd);
	assertf(check == 0, "Error in fsync");
}

#if ALIGN_ON
// Check if the current object with 'size' can fit in the available region
// Return 1 on succesfull, and 0 otherwise
int	r_is_obj_fits_in_region(size_t size) {
	size_t obj_size = size * HEAPWORD;

	assertf(obj_size > 0, "Object should be > 0");
	assertf(obj_size <= REGION_SIZE - HEADER_SIZE, "Oject size: %lu exceeds region size: %lu",
			obj_size, REGION_SIZE - HEADER_SIZE);

	return (obj_size  <= tc_mem_pool.region_free_space);
}

// Get the top address of the current region and set the current_ptr to the
// next region
char* r_region_top_addr(void) {
	// Set the current ptr to the next region and initialize next region's free space
	// We reserve the last HEADE_SIZE bytes in regions for the dummy object
	tc_mem_pool.cur_alloc_ptr += tc_mem_pool.region_free_space + HEADER_SIZE;

	assertf(tc_mem_pool.cur_alloc_ptr >= tc_mem_pool.start_address &&
			tc_mem_pool.cur_alloc_ptr < tc_mem_pool.stop_address, "TeraCache out-of-space");

	tc_mem_pool.region_free_space = REGION_SIZE - HEADER_SIZE;

	return tc_mem_pool.cur_alloc_ptr;
}

// This function if for the FastMap hybrid version. Give advise to kernel to
// serve all the pagefault using regular pages.
void r_enable_regular_flts(void) {
	madvise(tc_mem_pool.mmap_start, DEV_SIZE, MADV_NOHUGEPAGE);
}

// This function if for the FastMap hybrid version. Give advise to kernel to
// serve all the pagefault using huge pages.
void r_enable_huge_flts(void) {
	madvise(tc_mem_pool.mmap_start, DEV_SIZE, MADV_HUGEPAGE);
}

#endif
