#ifndef SHARE_VM_GC_IMPLEMENTATION_TERACACHE_TERACACHE_HPP
#define SHARE_VM_GC_IMPLEMENTATION_TERACACHE_TERACACHE_HPP
#include "gc_interface/collectedHeap.inline.hpp"
#include "oops/oop.hpp"
#include <regions.h>

class TeraCache {
	private:
		static char*    _start_addr;              // TeraCache start address
		static char*    _stop_addr;               // TeraCache ends address
		static region_t _region;                  // Region
		static char*    _start_pos_region;        // Start address of region
		static char*    _next_pos_region;         // Next allocated region in region

		// Statistics of TeraCache
		int total_active_regions;          // Number of active regions
		int total_objects;                 // Number of objects located in teraCache
		int total_objects_size;             // Total number of objects size
		int total_merged_regions;             // Number of merged regions

		// Stack that locates root objects - RDDs
		// We use this hash table to start traversing the heap from
		// these roots and then move to the other roots
		//protected:
		//  static Stack<oop, mtGC>  _tera_root_stack;

	public:
		// Constructor
		TeraCache(); 

		// Check if this object is located in TeraCache
		bool tc_check(oop ptr);

		// Create new region
		void tc_new_region(void);

		// Get region allocation address
		char* tc_get_addr_region(void);

		// Add new object in the region
		char* tc_region_top(oop obj, size_t size);

		// Get the last access pointer of the region
		char* tc_region_cur_ptr(void);

		// Add root pointer to the stack
		//void add_root_stack(oop obj);

		//// Get root pointer from the stack
		//oop  get_root_stack(void);

};

#endif
