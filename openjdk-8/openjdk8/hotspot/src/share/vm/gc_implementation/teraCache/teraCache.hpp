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

		static ObjectStartArray _start_array; // Keeps track of where objects 
											  // start in a 512b block

		/*-----------------------------------------------
		 * Stacks
		 *---------------------------------------------*/
		// Stack to keep back pointers (Objects that are pointed out of
		// TeraCache objects) to mark them as alive durin mark_and_push phase of
		// the Full GC.
		static Stack<oop, mtGC>   _tc_stack;

		// Stack to keep the element addresses of objects that are located in
		// TeraCache and point to objects in the heap. We adjust these pointers
		// during adjust phase of the Full GC.
		static Stack<oop *, mtGC> _tc_adjust_stack;

		/*-----------------------------------------------
		 * Statistics of TeraCache
		 *---------------------------------------------*/
		static uint64_t total_active_regions;      // Number of active regions
		static uint64_t total_merged_regions;      // Number of merged regions

		static uint64_t total_objects;             // Total number of objects located in TeraCache
		static uint64_t total_objects_size;        // Total number of objects size

		static uint64_t fwd_ptrs_per_fgc;	       // Total number of forward ptrs per FGC
		static uint64_t back_ptrs_per_fgc;	       // Total number of back ptrs per FGC
		static uint64_t trans_per_fgc;	           // Total number of objects transfered to 
												   // TeraCache per FGC
	public:
		// Constructor
		TeraCache(); 
		
		// Close TeraCache and unmap all the pages
		void tc_shutdown(); 

		// Check if this object is located in TeraCache
		bool tc_check(oop ptr);

		// Check if object p belongs to TeraCache
		bool tc_is_in(void* p);

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
};

#endif
