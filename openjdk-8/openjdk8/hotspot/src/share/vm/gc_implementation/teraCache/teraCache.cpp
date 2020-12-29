#include <iostream>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include "gc_implementation/teraCache/teraCache.hpp"
#include "gc_implementation/parallelScavenge/psVirtualspace.hpp"
#include "memory/cardTableModRefBS.hpp"
#include "memory/memRegion.hpp"
#include "memory/sharedDefines.h"
#include "runtime/mutexLocker.hpp"          // std::mutex
#include "oops/oop.inline.hpp"
#include "utilities/globalDefinitions.hpp"

char*        TeraCache::_start_addr = NULL;
char*        TeraCache::_stop_addr = NULL;
region_t     TeraCache::_region = NULL;
char*        TeraCache::_start_pos_region = NULL;
char*        TeraCache::_next_pos_region = NULL; 
HeapWord*    TeraCache::_parent_node = NULL;
ObjectStartArray TeraCache::_start_array;
Stack<oop, mtGC> TeraCache::_tc_stack;
Stack<oop *, mtGC> TeraCache::_tc_adjust_stack;

// Constructor of TeraCache
TeraCache::TeraCache()
{
	init();
	// Start address of memory map pool
	_start_addr = (char *)start_addr_mem_pool();
	// Stop address of memory map pool
	_stop_addr  =  (char*)((char *)_start_addr + mem_pool_size());

	// Initilize counters for TeraCache
	// These counters are used for experiments
	total_active_regions = 0;
	total_objects = 0;
	total_objects_size = 0;
	total_merged_regions = 0;
	total_forward_ptrs = 0;
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

// Create a new region in the TeraCache. The size of regions is dynamically. 
// TODO(JK:) Support multiple regions
// TODO(JK:) Fix the size. For now I use a hard number for the size
void TeraCache::tc_new_region()
{
	// Update Statistics
	total_active_regions++;

	// Create a new region
	_region = new_region(NULL);

	// Initialize the size of the region
	_start_pos_region = (char *)rc_rstralloc0(_region, 41943040*sizeof(char));
	_start_pos_region = (char *)align_ptr_up(_start_pos_region, CardTableModRefBS::ct_max_alignment_constraint());

	// Check if the allocation happens succesfully
	assertf((char *)(_start_pos_region) !=  (char *) NULL, "Allocation Failed");

	// Initialize pointer
	_next_pos_region = _start_pos_region;

	if (TeraCacheStatistics)
	{
		tclog_or_tty->print_cr("[STATISTICS] | NUM_ACTIVE_REGIONS = %d", total_active_regions);
	}
}

// Return the start address of the region
char* TeraCache::tc_get_addr_region(void)
{
	assertf((char *)(_start_pos_region) != NULL, "Region is not allocated");
	return _start_pos_region;
}

// Get the size of the region
size_t TeraCache::tc_get_size_region(void)
{
	return 41943040;
}

// Get the allocation top pointer of the region
char* TeraCache::tc_region_top(oop obj, size_t size)
{
	if (TeraCacheStatistics)
	{
		tclog_or_tty->print_cr("[STATISTICS] | OBJECT_SIZE  =  %d", size);
	}

	// Update Statistics
	MutexLocker x(tera_cache_lock);
	total_objects_size += size;
	total_objects ++;
	char *tmp = _next_pos_region;

	printf("[BEFORE TC_REGION_TOP] | OOP(PTR) = %p | NEXT_POS = %p | SIZE = %lu | NAME %s\n",
			(HeapWord *)obj, tmp, size, obj->klass()->internal_name());
	// make heapwordsize
	_next_pos_region = (char *) (((uint64_t) _next_pos_region) + size*sizeof(char*));

	if ((uint64_t) _next_pos_region % 8 != 0)
	{
		_next_pos_region = (char *)((((uint64_t)_next_pos_region) + (8 - 1)) & -8);
	}

	_start_array.allocate_block((HeapWord *)tmp);

	assertf((char *)(_next_pos_region) < (char *) _stop_addr, "Region is out-of-space");

	if (TeraCacheStatistics)
	{
		tclog_or_tty->print_cr("[STATISTICS] | NUM_OF_OBJECTS  = %d", total_objects);
		tclog_or_tty->print_cr("[STATISTICS] | TOTAL_OBJECTS_SIZE = %d", total_objects_size);
	}

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

void TeraCache::scavenge()
{
	while (!_tc_stack.is_empty()) {
		oop obj = _tc_stack.pop();

		printf("TC_STACK: obj = %p\n", obj); 

		MarkSweep::mark_and_push(&obj);
	}
}
		
void TeraCache::tc_push_object(void *p, oop o) {
	_tc_stack.push(o);
	_tc_adjust_stack.push((oop *)p);
}

void TeraCache::tc_adjust() {
	while (!_tc_adjust_stack.is_empty()) {
		oop * obj = _tc_adjust_stack.pop();

		MarkSweep::adjust_pointer(obj);
	}

}

// TODO Remove all these
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
void TeraCache::tc_check_back_pointers(bool assert_on)
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

		if (assert_on)
		{
			// Follow the contents of the object
			// True
			oop(q)->follow_contents_tera_cache(true);
		}
		else
		{
			// Follow the contents of the object
			// False
			std::cerr << "HERE " << __func__ << std::endl;
			oop(q)->follow_contents_tera_cache(false);
		}

		// Move to the next object
		q+=size;
	}
}


// Increase forward ptrs from JVM heap to TeraCache
void TeraCache::tc_increase_forward_ptrs()
{
	total_forward_ptrs++;
	if (TeraCacheStatistics)
	{
		tclog_or_tty->print_cr("[STATISTICS] | TOTAL_FORWARD_PTRS = %d", total_forward_ptrs);
	}
}


void TeraCache::add_tc_back_ptr(HeapWord *dest)
{
	assertf(dest != NULL, "Object is empty");
	assertf(_parent_node != NULL, "Parent node is empty");

	// Check here for duplications
	// TODO check for duplications

	// Update vector with references from TeraCache to Heap
	tc_to_heap_ptrs[dest]._v_src.push_back(_parent_node);

	for (std::map<HeapWord*, back_ptr>::const_iterator it = tc_to_heap_ptrs.begin();
				it != tc_to_heap_ptrs.end(); it++)
	{
		std::cerr << "DEBUG_CHECK " << it->first << std::endl;
	}

	return;
}


void TeraCache::tc_trace_obj(oop obj)
{
	_parent_node = (HeapWord*) obj;
}


void TeraCache::tc_update_heap_ptr(HeapWord* dest, HeapWord* new_dest)
{
	// Check if the map is empty
	std::cerr << "DEBUG_CHECK | DEST = " << dest << " NEW_DEST = " << new_dest << std::endl;
	if (tc_to_heap_ptrs.empty())
	{
		return;
	}

	// Check if the new_ptr has already been assigned
	assertf(tc_to_heap_ptrs[dest]._new_dst == NULL, "Rewrite new destination ptr");

	// Update the location of the dest ptr
	tc_to_heap_ptrs[dest]._new_dst = new_dest;

	return;
}


HeapWord* TeraCache::tc_heap_ptr(HeapWord* dest)
{
	// Check if the new_ptr has already been assigned
	assertf(tc_to_heap_ptrs[dest]._new_dst != NULL, "Return null destination");

	// Update the location of the dest ptr
	return tc_to_heap_ptrs[dest]._new_dst;

}


void TeraCache::tc_adjust_back_ptr(bool full_gc)
{
	if (full_gc)
	{
		// Traverse the map containing backward pointers
		for (std::map<HeapWord*, back_ptr>::const_iterator it = tc_to_heap_ptrs.begin();
				it != tc_to_heap_ptrs.end(); it++)
		{
			assertf(it->second._new_dst != NULL, "Invalid new address");

			std::vector <HeapWord*> tmp = it->second._v_src;

			for (size_t i = 0; i < tmp.size(); i++)
			{
				oop(tmp[i])->klass()->oop_adjust_pointers_tera_cache(oop(tmp[i]), true);
			}
		}
	}
	else
	{
		for (std::map<HeapWord*, back_ptr>::const_iterator it = tc_to_heap_ptrs.begin();
				it != tc_to_heap_ptrs.end(); it++)
		{

			assertf(it->second._new_dst != NULL, "Invalid new address");

			std::vector <HeapWord*> tmp = it->second._v_src;

			for (size_t i = 0; i < tmp.size(); i++)
			{
				oop(tmp[i])->klass()->oop_adjust_pointers_tera_cache(oop(tmp[i]), false);
			}
		}
	}
}

// Clear back pointers at the end of each FGC
void TeraCache::tc_clear_map(void)
{
	tc_to_heap_ptrs.clear();
}

void TeraCache::tc_print_map(void)
{
	for (std::map<HeapWord*, back_ptr>::const_iterator it = tc_to_heap_ptrs.begin();
			it != tc_to_heap_ptrs.end(); it++)
	{
		std::cout << "Key = " << it->first << std::endl;
		std::cout << "\t New Addr = " << it->second._new_dst << std::endl;

		std::vector <HeapWord*> tmp = it->second._v_src;

		std::cout << "\t Src <";
		for (size_t i = 0; i < tmp.size(); i++)
		{
			std::cout << tmp[i] <<", " << std::endl;
		}
	}
}

bool TeraCache::tc_empty() {
	if (_start_pos_region == _next_pos_region) {
		return true;
	}
	else {
		return false;
	}
}
