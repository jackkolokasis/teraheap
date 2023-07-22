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
#include "../include/asyncIO.h"
#include "../include/segments.h"
#include "../include/sharedDefines.h"

#define HEAPWORD (8)                       // In the JVM the heap is aligned to 8 words
#define HEADER_SIZE (32)                   // Header size of the Dummy object	
#define align_size_up_(size, alignment) (((size) + ((alignment) - 1)) & ~((alignment) - 1))

#ifdef H2_DYNAMIC_FILE_ALLOCATION
	//globals for opening file
	char dev[150];
	uint64_t dev_size;
  uint64_t region_array_size;
  uint64_t max_rdd_id;
  uint64_t group_array_size;
#endif

struct _mem_pool tc_mem_pool;
int fd;

intptr_t align_size_up(intptr_t size, intptr_t alignment) {
	return align_size_up_(size, alignment);
}

void* align_ptr_up(void* ptr, size_t alignment) {
	return (void*)align_size_up((intptr_t)ptr, (intptr_t)alignment);
}

#ifndef H2_DYNAMIC_FILE_ALLOCATION
// Initialize allocator
void init(uint64_t align) {
    fd = -1;

#if ANONYMOUS
	// Anonymous mmap
    fd = open(DEV, O_RDWR);
	tc_mem_pool.mmap_start = mmap(0, V_SPACE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS|MAP_NORESERVE, -1, 0);
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
#if ANONYMOUS
	tc_mem_pool.stop_address = tc_mem_pool.mmap_start + V_SPACE;
    printf("Start address:%p\n",tc_mem_pool.start_address);
    printf("Stop address:%p\n",tc_mem_pool.stop_address);
#else
	tc_mem_pool.stop_address = tc_mem_pool.mmap_start + DEV_SIZE;
#endif
    init_regions();
	req_init();
}
#else
void init(uint64_t align,const char* l_dev, uint64_t l_dev_size) {
    fd = -1;
  //initializing globals 

	strcpy(dev, l_dev);
	strcat(dev,".XXXXXX");


	dev_size = l_dev_size;
	region_array_size = ((dev_size) / (REGION_SIZE));
  assertf(region_array_size >= MAX_PARTITIONS, "Device size should be larger, because region_array_size is calculated to be smaller than MAX_PARTITIONS!");
	max_rdd_id = ((region_array_size) / (MAX_PARTITIONS));
	group_array_size = ((region_array_size) / 2); //deprecated

#if ANONYMOUS
	// Anonymous mmap
  fd = mkstemp(dev);
  unlink(dev);
  assertf(fd >= 1, "tempfile error.");

  ftruncate(fd, dev_size);
  tc_mem_pool.mmap_start = mmap(0, V_SPACE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS|MAP_NORESERVE, -1, 0);
#else
	fd = mkstemp(dev);
  fprintf(stderr, "fd = %d\n", fd);
  unlink(dev);
  assertf(fd >= 1, "tempfile error.");

  fprintf(stderr, "truncate = %d\n", ftruncate(fd, dev_size));

  // Memory-mapped a file over a storage device
  tc_mem_pool.mmap_start = mmap(0, dev_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
#endif

	assertf(tc_mem_pool.mmap_start != MAP_FAILED, "Mapping Failed");
	// Card table in JVM needs the start address of TeraCache to be align up
	tc_mem_pool.start_address = (char *) align_ptr_up(tc_mem_pool.mmap_start, align);
	tc_mem_pool.cur_alloc_ptr = tc_mem_pool.start_address;
	tc_mem_pool.size = 0;
#if ANONYMOUS
	tc_mem_pool.stop_address = tc_mem_pool.mmap_start + V_SPACE;
    printf("Start address:%p\n",tc_mem_pool.start_address);
    printf("Stop address:%p\n",tc_mem_pool.stop_address);
#else
  tc_mem_pool.stop_address = tc_mem_pool.mmap_start + dev_size;
#endif
    init_regions();
	req_init();
}
#endif

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
#if ANONYMOUS
    return V_SPACE;
#else
#ifndef H2_DYNAMIC_FILE_ALLOCATION
	return DEV_SIZE;
#else
	return dev_size;
#endif
#endif
}

char* allocate(size_t size, uint64_t rdd_id, uint64_t partition_id) {
	char* alloc_ptr = tc_mem_pool.cur_alloc_ptr;
	char* prev_alloc_ptr = tc_mem_pool.cur_alloc_ptr;

	assertf(size > 0, "Object should be > 0");

  alloc_ptr = allocate_to_region(size * HEAPWORD, rdd_id, partition_id);

  if (alloc_ptr == NULL) {
    perror("[Error] - H2 Allocator is full");
    exit(EXIT_FAILURE);
  }

    tc_mem_pool.size += size;
    tc_mem_pool.cur_alloc_ptr = (char *) (((uint64_t) alloc_ptr) + size * HEAPWORD);

	if (prev_alloc_ptr > tc_mem_pool.cur_alloc_ptr)
		tc_mem_pool.cur_alloc_ptr = prev_alloc_ptr;

	assertf(prev_alloc_ptr <= tc_mem_pool.cur_alloc_ptr, 
			"Error alloc ptr: Prev = %p, Cur = %p", prev_alloc_ptr, tc_mem_pool.cur_alloc_ptr);

	// Alighn to 8 words the pointer (TODO: CHANGE TO ASSERTION)
	if ((uint64_t) tc_mem_pool.cur_alloc_ptr % HEAPWORD != 0)
		tc_mem_pool.cur_alloc_ptr = (char *)((((uint64_t)tc_mem_pool.cur_alloc_ptr) + (HEAPWORD - 1)) & -HEAPWORD);

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
#ifndef H2_DYNAMIC_FILE_ALLOCATION
	munmap(tc_mem_pool.mmap_start, DEV_SIZE);
#else
	munmap(tc_mem_pool.mmap_start, dev_size);
#endif
}

// Give advise to kernel to expect page references in sequential order.  (Hence,
// pages in the given range can be aggressively read ahead, and may be freed
// soon after they are accessed.)
void r_enable_seq() {
#ifndef H2_DYNAMIC_FILE_ALLOCATION
	madvise(tc_mem_pool.mmap_start, DEV_SIZE, MADV_SEQUENTIAL);
#else
	madvise(tc_mem_pool.mmap_start, dev_size, MADV_SEQUENTIAL);
#endif	
}

// Give advise to kernel to expect page references in random order (Hence, read
// ahead may be less useful than normally.)
void r_enable_rand() {
#ifndef H2_DYNAMIC_FILE_ALLOCATION
        madvise(tc_mem_pool.mmap_start, DEV_SIZE, MADV_NORMAL);
#else
        madvise(tc_mem_pool.mmap_start, dev_size, MADV_NORMAL);
#endif
}

// Explicit write 'data' with 'size' in certain 'offset' using system call
// without memcpy.
void r_write(char *data, char *offset, size_t size) {
#ifdef ASSERT
	ssize_t s_check = 0;
	uint64_t diff = offset - tc_mem_pool.mmap_start;

	s_check = pwrite(fd, data, size * HEAPWORD, diff);
	assertf(s_check == size * HEAPWORD, "Sanity check: s_check = %ld", s_check);
#else
	uint64_t diff = offset - tc_mem_pool.mmap_start;
	pwrite(fd, data, size * HEAPWORD, diff);
#endif
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
#ifdef ASSERT
	int check = fsync(fd);
	assertf(check == 0, "Error in fsync");
#else
	fsync(fd);
#endif
}

// This function if for the FastMap hybrid version. Give advise to kernel to
// serve all the pagefault using regular pages.
void r_enable_regular_flts(void) {
#ifndef H2_DYNAMIC_FILE_ALLOCATION
        madvise(tc_mem_pool.mmap_start, DEV_SIZE, MADV_NOHUGEPAGE);
#else
        madvise(tc_mem_pool.mmap_start, dev_size, MADV_NOHUGEPAGE);
#endif
}

// This function if for the FastMap hybrid version. Give advise to kernel to
// serve all the pagefault using huge pages.
void r_enable_huge_flts(void) {
#ifndef H2_DYNAMIC_FILE_ALLOCATION
        madvise(tc_mem_pool.mmap_start, DEV_SIZE, MADV_HUGEPAGE);
#else
        madvise(tc_mem_pool.mmap_start, dev_size, MADV_HUGEPAGE);
#endif
}
