#include <iostream>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include "gc_implementation/teraCache/teraCache.hpp"
#include "memory/sharedDefines.h"
#include "runtime/mutexLocker.hpp"          // std::mutex
#include "oops/oop.inline.hpp"

char*        TeraCache::_start_addr = NULL; // Address shows where TeraCache start
char*        TeraCache::_stop_addr = NULL;  // Address shows where TeraCache ends
region_t     TeraCache::_region = NULL;
char*        TeraCache::_start_pos_region = NULL;
char*        TeraCache::_next_pos_region = NULL;   // Next allocated region in region

TeraCache::TeraCache()
{
	init();
	_start_addr = (char *)start_addr_mem_pool();
	_stop_addr  =  (char*)((char *)_start_addr + mem_pool_size());

	total_active_regions = 0;
	total_objects = 0;
	total_objects_size = 0;
	total_merged_regions = 0;
}

bool TeraCache::tc_check(oop ptr)
{

#if DEBUG_TERACACHE
	printf("[TC_CHECK] | OOP(PTR) = %p | START_ADDR = %p | STOP_ADDR = %p | start = %p \n", 
			ptr, _start_addr, _stop_addr, (char *) ptr);
#endif

	if (((HeapWord *)ptr >= (HeapWord *) _start_addr) && ((HeapWord *) ptr < (HeapWord *)_stop_addr))
	{
		return true;
	}
	else 
	{
		return false;
	}
}

void TeraCache::tc_new_region(void)
{
	// Update Statistics
	total_active_regions++;

	// Create a new region
	_region = new_region(NULL);

	// Initialize the size of the region
	//_start_pos_region = (char *)rc_rstralloc0(_region, 5242880*sizeof(char));
	_start_pos_region = (char *)rc_rstralloc0(_region, 41943040*sizeof(char));

	// Check if the allocation happens succesfully
	assertf((char *)(_start_pos_region) !=  (char *) NULL, "Allocation Failed");

	// Initializw pointer
	_next_pos_region = _start_pos_region;

#if STATISTICS
	std::cerr << "[STATISTICS] | NUM_ACTIVE_REGIONS = " << total_active_regions << std::endl;
#endif
}

char* TeraCache::tc_get_addr_region(void)
{
	assertf((char *)(_start_pos_region) != NULL, "Region is not allocated");
	return (char *) _start_pos_region;
}

char* TeraCache::tc_region_top(oop obj, size_t size)
{
#if DEBUG_TERACACHE
	printf("[TC_REGION_TOP] | OOP(PTR) = %p | NEXT_POS = %p | SIZE = %p\n", 
			obj, _next_pos_region, size);
#endif

#if STATISTICS
	std::cerr << "[STATISTICS] | OBJECT_SIZE  = " << size << std::endl;
#endif

	// Update Statistics
	MutexLocker x(tera_cache_lock);
	total_objects_size += size;
	total_objects ++;
	char *tmp = _next_pos_region;

	printf("[BEFORE TC_REGION_TOP] | OOP(PTR) = %p | NEXT_POS = %p | SIZE = %d\n", (HeapWord *)obj, tmp, size);
	// make heapwordsize
	_next_pos_region = (char *) (((uint64_t) _next_pos_region) + size*sizeof(char*));


	if ((uint64_t) _next_pos_region % 8 != 0)
	{
		_next_pos_region = (char *)((((uint64_t)_next_pos_region) + (8 - 1)) & -8);
	}

	assertf((char *)(_next_pos_region) < (char *) _stop_addr, "Region is out-of-space");

#if STATISTICS
	std::cerr << "[STATISTICS] | NUM_OF_OBJECTS  = " << total_objects << std::endl;
	std::cerr << "[STATISTICS] | TOTAL_OBJECTS_SIZE = " << total_objects_size << std::endl;
#endif

	return tmp;
}

// Get the last access pointer of the region
char* TeraCache::tc_region_cur_ptr(void)
{
	std::cerr << "Get the region of the last pointer" << std::endl;

	assertf((char *)(_next_pos_region) != (char *) NULL, "Invalid pointer");
	assertf((char *)(_next_pos_region) < (char *) _stop_addr, "Region is full");
	return (char *) _next_pos_region;
}


void TeraCache::tc_adjust_pointers()
{
	
	if (_start_pos_region == _next_pos_region)
	{
		return;
	}

	HeapWord* q  = (HeapWord *) _start_pos_region;
	HeapWord* t  = (HeapWord *) _next_pos_region;

	// Point all the oops to the new location
	while (q < t) {
		if (oop(q)->klass() == NULL)
		{
			return;
		}

		std::cerr << "Ajust pointers in the teraCache" << std::endl;
		size_t size = oop(q)->adjust_pointers();
		q += size;
	}
}


// Check for backward pointers
void TeraCache::tc_check_back_pointers()
{
	if (_start_pos_region == _next_pos_region)
	{
		return;
	}

	HeapWord* q  = (HeapWord *) _start_pos_region;
	HeapWord* t  = (HeapWord *) _next_pos_region;

	while (q < t)
	{
		// Get the size of the object
		size_t size = oop(q)->size();

		// Follow the contents of the object
		oop(q)->follow_contents_tera_cache();

		// Move to the next object
		q+=size;
	}
}
