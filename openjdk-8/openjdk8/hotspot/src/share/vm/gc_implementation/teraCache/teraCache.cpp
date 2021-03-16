#include <iostream>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include "gc_implementation/teraCache/teraCache.hpp"
#include "gc_implementation/parallelScavenge/psVirtualspace.hpp"
#include "memory/cardTableModRefBS.hpp"
#include "memory/memRegion.hpp"
#include "memory/sharedDefines.h"
#include "runtime/globals.hpp"
#include "runtime/mutexLocker.hpp"
#include "oops/oop.inline.hpp"
#include "utilities/globalDefinitions.hpp"

char*        TeraCache::_start_addr = NULL;
char*        TeraCache::_stop_addr = NULL;

ObjectStartArray TeraCache::_start_array;
Stack<oop, mtGC> TeraCache::_tc_stack;
Stack<oop *, mtGC> TeraCache::_tc_adjust_stack;

uint64_t TeraCache::total_active_regions;
uint64_t TeraCache::total_merged_regions;
uint64_t TeraCache::total_objects;
uint64_t TeraCache::total_objects_size;
uint64_t TeraCache::fwd_ptrs_per_fgc;
uint64_t TeraCache::back_ptrs_per_fgc;
uint64_t TeraCache::trans_per_fgc;

uint64_t TeraCache::dirty_cards[16];
uint64_t TeraCache::tc_ct_trav_time[16];
uint64_t TeraCache::heap_ct_trav_time[16];

// Constructor of TeraCache
TeraCache::TeraCache() {
	
	uint64_t align = CardTableModRefBS::ct_max_alignment_constraint();
	init(align);

	_start_addr = start_addr_mem_pool();
	_stop_addr  = stop_addr_mem_pool();

	// Initilize counters for TeraCache
	// These counters are used for experiments
	total_active_regions = 0;
	total_merged_regions = 0;

	total_objects = 0;
	total_objects_size = 0;

	// Initialize arrays for the next minor collection
	for (unsigned int i = 0; i < ParallelGCThreads; i++) {
		tc_ct_trav_time[i] = 0;
		heap_ct_trav_time[i] = 0;
	}
}
		
void TeraCache::tc_shutdown() {
	r_shutdown();
}


// Check if an object `ptr` belongs to the TeraCache. If the object belongs
// then the function returns true, either it returns false.
bool TeraCache::tc_check(oop ptr) {
	return ((HeapWord *)ptr >= (HeapWord *) _start_addr)     // if greater than start address
			&& ((HeapWord *) ptr < (HeapWord *)_stop_addr);  // if smaller than stop address
}

// Check if an object `p` belongs to TeraCache. If the object bolongs to
// TeraCache then the function returns true, either it returns false.
bool TeraCache::tc_is_in(void *p) {
	char* const cp = (char *)p;

	return cp >= _start_addr && cp < _stop_addr;
}

// Return the start address of the region
char* TeraCache::tc_get_addr_region(void) {
	assertf((char *)(_start_addr) != NULL, "Region is not allocated");

	return _start_addr;
}

// Get the size of TeraCache
size_t TeraCache::tc_get_size_region(void) {
	return mem_pool_size();
}

// Allocate new object 'obj' with 'size' in words in TeraCache.
// Return the allocated 'pos' position of the object
char* TeraCache::tc_region_top(oop obj, size_t size) {
	char *pos;			// Allocation position

	MutexLocker x(tera_cache_lock);

	// Update Statistics
	total_objects_size += size;
	total_objects++;
	trans_per_fgc++;

#if DEBUG_TERACACHE
	printf("[BEFORE TC_REGION_TOP] | OOP(PTR) = %p | NEXT_POS = %p | SIZE = %lu | NAME %s\n",
			(HeapWord *)obj, cur_alloc_ptr(), size, obj->klass()->internal_name());
#endif

	pos = allocate(size);
	
	if (TeraCacheStatistics)
		tclog_or_tty->print_cr("[STATISTICS] | OBJECT = %lu | Name = %s", size, obj->klass()->internal_name());

	_start_array.allocate_block((HeapWord *)pos);

	return pos;
}

// Get the allocation top pointer of the TeraCache
char* TeraCache::tc_region_cur_ptr(void) {
	return cur_alloc_ptr();
}

// Pop the objects that are in `_tc_stack` and mark them as live object. These
// objects are located in the Java Heap and we need to ensure that they will be
// kept alive.
void TeraCache::scavenge()
{
	struct timeval start_time;
	struct timeval end_time;

	gettimeofday(&start_time, NULL);

	while (!_tc_stack.is_empty()) {
		oop obj = _tc_stack.pop();

		if (TeraCacheStatistics)
			back_ptrs_per_fgc++;

		MarkSweep::mark_and_push(&obj);
	}
	
	gettimeofday(&end_time, NULL);

	if (TeraCacheStatistics)
		tclog_or_tty->print_cr("[STATISTICS] | TC_MARK = %llu\n", 
				(unsigned long long)((end_time.tv_sec - start_time.tv_sec) * 1000) + // convert to ms
				(unsigned long long)((end_time.tv_usec - start_time.tv_usec) / 1000)); // convert to ms
}
		
void TeraCache::tc_push_object(void *p, oop o) {

#if MT_STACK
	MutexLocker x(tera_cache_lock);
#endif
	_tc_stack.push(o);
	_tc_adjust_stack.push((oop *)p);

	assertf(!_tc_stack.is_empty(), "Sanity Check");
	assertf(!_tc_adjust_stack.is_empty(), "Sanity Check");
}

// Adjust backwards pointers during Full GC.  
void TeraCache::tc_adjust() {
	struct timeval start_time;
	struct timeval end_time;

	gettimeofday(&start_time, NULL);

	while (!_tc_adjust_stack.is_empty()) {
		oop* obj = _tc_adjust_stack.pop();
		MarkSweep::adjust_pointer(obj);
	}
	
	gettimeofday(&end_time, NULL);

	if (TeraCacheStatistics)
		tclog_or_tty->print_cr("[STATISTICS] | TC_ADJUST %llu\n",
				(unsigned long long)((end_time.tv_sec - start_time.tv_sec) * 1000) + // convert to ms
				(unsigned long long)((end_time.tv_usec - start_time.tv_usec) / 1000)); // convert to ms
}

// Increase the number of forward ptrs from JVM heap to TeraCache
void TeraCache::tc_increase_forward_ptrs() {
	fwd_ptrs_per_fgc++;
}

// Check if the TeraCache is empty. If yes, return 'true', 'false' otherwise
bool TeraCache::tc_empty() {
	return r_is_empty();
}
		
void TeraCache::tc_clear_stacks() {
	_tc_adjust_stack.clear(true);
	_tc_stack.clear(true);
}

// Init the statistics counters of TeraCache to zero when a Full GC
// starts
void TeraCache::tc_init_counters() {
	fwd_ptrs_per_fgc  = 0;	
	back_ptrs_per_fgc = 0;
	trans_per_fgc     = 0;
}

// Print the statistics of TeraCache at the end of each FGC
// Will print:
//	- the total forward pointers from the JVM heap to the TeraCache
//	- the total back pointers from TeraCache to the JVM heap
//	- the total objects that has been transfered to the TeraCache
//	- the current total size of objects in TeraCache until
//	- the current total objects that are located in TeraCache
void TeraCache::tc_print_statistics() {
	tclog_or_tty->print_cr("[STATISTICS] | TOTAL_FORWARD_PTRS = %lu\n", fwd_ptrs_per_fgc);
	tclog_or_tty->print_cr("[STATISTICS] | TOTAL_BACK_PTRS = %lu\n", back_ptrs_per_fgc);
	tclog_or_tty->print_cr("[STATISTICS] | TOTAL_TRANS_OBJ = %lu\n", trans_per_fgc);

	tclog_or_tty->print_cr("[STATISTICS] | TOTAL_OBJECTS  = %lu\n", total_objects);
	tclog_or_tty->print_cr("[STATISTICS] | TOTAL_OBJECTS_SIZE = %lu\n", total_objects_size);
}
		
// Keep for each thread the time that need to traverse the TeraCache
// card table.
// Each thread writes the time in a table based on each ID and then we
// take the maximum time from all the threads as the total time.
void TeraCache::tc_ct_traversal_time(unsigned int tid, uint64_t total_time) {
	if (tc_ct_trav_time[tid]  < total_time)
		tc_ct_trav_time[tid] = total_time;
}

// Keep for each thread the time that need to traverse the Heap
// card table
// Each thread writes the time in a table based on each ID and then we
// take the maximum time from all the threads as the total time.
void TeraCache::heap_ct_traversal_time(unsigned int tid, uint64_t total_time) {
	if (heap_ct_trav_time[tid]  < total_time)
		heap_ct_trav_time[tid] = total_time;
}

// Print the statistics of TeraCache at the end of each minorGC
// Will print:
//	- the time to traverse the TeraCache dirty card tables
//	- the time to traverse the Heap dirty card tables
//	- TODO number of dirty cards in TeraCache
//	- TODO number of dirty cards in Heap
void TeraCache::tc_print_mgc_statistics() {
	uint64_t max_tc_ct_trav_time = 0;		//< Maximum traversal time of
											// TeraCache card tables from all
											// threads
	uint64_t max_heap_ct_trav_time = 0;     //< Maximum traversal time of Heap
											// card tables from all the threads

	for (unsigned int i = 0; i < ParallelGCThreads; i++) {
		if (max_tc_ct_trav_time < tc_ct_trav_time[i])
			max_tc_ct_trav_time = tc_ct_trav_time[i];
		
		if (max_heap_ct_trav_time < heap_ct_trav_time[i])
			max_heap_ct_trav_time = heap_ct_trav_time[i];
	}

	tclog_or_tty->print_cr("[STATISTICS] | TC_CT_TIME = %lu\n", max_tc_ct_trav_time);
	tclog_or_tty->print_cr("[STATISTICS] | HEAP_CT_TIME = %lu\n", max_heap_ct_trav_time);

	// Initialize arrays for the next minor collection
	for (unsigned int i = 0; i < ParallelGCThreads; i++) {
		tc_ct_trav_time[i] = 0;
		heap_ct_trav_time[i] = 0;
	}
}

// Give advise to kernel to expect page references in sequential order
void TeraCache::tc_enable_seq() {
	r_enable_seq();
}

// Give advise to kernel to expect page references in random order
void TeraCache::tc_enable_rand() {
	r_enable_rand();
}
		
// Explicit (using systemcall) write 'data' with 'size' to the specific
// 'offset' in the file.
void TeraCache::tc_write(char *data, char *offset, size_t size) {
	r_write(data, offset, size);
}
