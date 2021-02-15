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
		static Stack<oop, mtGC>         _tc_stack;
		static Stack<oop *, mtGC>       _tc_adjust_stack;

		/*-----------------------------------------------
		 * Statistics of TeraCache
		 *---------------------------------------------*/
		uint64_t total_active_regions;      // Number of active regions
		uint64_t total_merged_regions;      // Number of merged regions

		uint64_t total_objects;             // Total number of objects located in TeraCache
		uint64_t total_objects_size;        // Total number of objects size

		uint64_t fwd_ptrs_per_fgc;	       // Total number of forward ptrs per FGC
		uint64_t back_ptrs_per_fgc;	       // Total number of back ptrs per FGC
		uint64_t trans_per_fgc;	           // Total number of objects transfered to 
									   // TeraCache per FGC

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
		
		// Increase forward ptrs from JVM heap to TeraCache
		void tc_increase_forward_ptrs();

		ObjectStartArray* start_array() { return &_start_array; }

		void tc_push_object(void *p, oop o);

		void tc_adjust();

		// Deallocate the stacks
		void tc_clear_stacks();

		// Init the statistics counters of TeraCache to zero when a Full GC
		// starts
		void tc_init_counters();
		
		// Print the statistics of TeraCache at the end of each FGC
		// Will print:
		//	- the total forward pointers from the JVM heap to the
		// TeraCache
		//	- the total back pointers from TeraCache to the JVM heap
		//	- the total objects that has been transfered to the TeraCache
		//	- the current total size of objects in TeraCache
		//	- the current total objects that are located in TeraCache
		void tc_print_statistics();


		// TeraCache add back pointer to the map of back pointers
		void add_tc_back_ptr(HeapWord *dest);
		
		// TeraCahce trace objects
		void tc_trace_obj(oop obj);
		
		// Teracache update heap pointer
		void tc_update_heap_ptr(HeapWord* dest, HeapWord *new_dest);
		
		HeapWord* tc_heap_ptr(HeapWord* dest);

		// Clear back pointers at the end of each Full GC
		void tc_clear_map(void);

		// Print the elements of map
		void tc_print_map(void);
		
		// Debuging
		// Check for backward pointers
		void tc_check_back_pointers(bool assert_on = false);

};

#endif
