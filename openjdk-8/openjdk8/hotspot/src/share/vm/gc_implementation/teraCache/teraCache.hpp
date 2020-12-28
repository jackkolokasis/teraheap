#ifndef SHARE_VM_GC_IMPLEMENTATION_TERACACHE_TERACACHE_HPP
#define SHARE_VM_GC_IMPLEMENTATION_TERACACHE_TERACACHE_HPP
#include <vector>
#include <map>
#include "gc_implementation/parallelScavenge/objectStartArray.hpp"
#include "gc_interface/collectedHeap.inline.hpp"
#include "oops/oop.hpp"
#include "utilities/globalDefinitions.hpp"
#include <regions.h>

class TeraCache {
	private:
		static char*    _start_addr;          // TeraCache start address of mmap region
		static char*    _stop_addr;           // TeraCache ends address of mmap region
		static region_t _region;              // Region
		static char*    _start_pos_region;    // Start address of region
		static char*    _next_pos_region;     // Next allocated region in region

		// TODO Test this I am not sure until now
		static ObjectStartArray _start_array; // Keeps track of where objects 
											  // start in a 512b block

		//===============================================
		// TODO Remove
		static HeapWord* _parent_node;		 // Parent Node for debugging;

		struct back_ptr 
		{
			HeapWord* _new_dst = NULL;
			std::vector<HeapWord*> _v_src;
		};
		// Pointers from teraCache to heap
		std::map<HeapWord*, back_ptr> tc_to_heap_ptrs;  
		//===============================================
		//
		static Stack<oop, mtGC>       _tc_stack;
		static Stack<oop *, mtGC>       _tc_adjust_stack;

		// Statistics of TeraCache
		int total_active_regions;      // Number of active regions
		int total_objects;             // Number of objects located in teraCache
		int total_objects_size;        // Total number of objects size
		int total_merged_regions;      // Number of merged regions
		int total_forward_ptrs;	       // Total number of forward ptrs

	public:
		// Constructor
		TeraCache(); 

		// Check if this object is located in TeraCache
		bool tc_check(oop ptr);

		// Check if object p belongs to TeraCache
		bool tc_is_in(void* p);

		// Create new region
		void tc_new_region(void);

		bool tc_empty(void);

		// Get region allocation address
		char* tc_get_addr_region(void);

		// Get the size of TeraCache
		size_t tc_get_size_region(void);

		// Add new object in the region
		char* tc_region_top(oop obj, size_t size);

		// Get the last access pointer of the region
		char* tc_region_cur_ptr(void);

		// Use in full GC
		void scavenge();




		// TeraCache adjust pointers during 3rd phase of fullgc
		// TODO remove
		void tc_adjust_pointers();

		// TeraCache add back pointer to the map of back pointers
		void add_tc_back_ptr(HeapWord *dest);
		
		// TeraCahce trace objects
		void tc_trace_obj(oop obj);
		
		// Teracache update heap pointer
		void tc_update_heap_ptr(HeapWord* dest, HeapWord *new_dest);
		
		// Teracache adjust back pointers
		void tc_adjust_back_ptr(bool full_gc);

		HeapWord* tc_heap_ptr(HeapWord* dest);

		// Clear back pointers at the end of each Full GC
		void tc_clear_map(void);

		// Print the elements of map
		void tc_print_map(void);
		
		// Debuging
		// Check for backward pointers
		void tc_check_back_pointers(bool assert_on = false);

		// Increase forward ptrs from JVM heap to TeraCache
		void tc_increase_forward_ptrs();

		ObjectStartArray* start_array() { return &_start_array; }

		void tc_push_object(void *p, oop o);

		void tc_adjust();
};

#endif
