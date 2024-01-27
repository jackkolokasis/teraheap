#include "gc/shared/gc_globals.hpp"
#include "gc/teraHeap/teraStatistics.hpp"
#include "runtime/arguments.hpp"

TeraStatistics::TeraStatistics() {
  total_objects = 0;
  total_objects_size = 0;
  num_fwd_tables = 0;

  forward_ref = NEW_C_HEAP_ARRAY(size_t, ParallelGCThreads, mtGC);
  memset(forward_ref, 0, ParallelGCThreads * sizeof(size_t));

  backward_ref = 0;
  moved_objects_per_gc = 0;

  memset(obj_distr_size, 0, sizeof(obj_distr_size));

  primitive_arrays_size = 0;
  primitive_obj_size = 0;
  non_primitive_obj_size = 0;

  num_primitive_arrays = 0;
  num_primitive_obj = 0;
  num_non_primitive_obj = 0;
}

TeraStatistics::~TeraStatistics() {
  FREE_C_HEAP_ARRAY(size_t, forward_ref);
}

// Increase by one the counter that shows the total number of
// objects that are moved to H2. Increase by 'size' the counter that shows 
// the total size of the objects that are moved to H2. Increase by one
// the number of objects that are moved in the current gc cycle to H2.
void TeraStatistics::add_object(size_t size) {
  total_objects++;
  total_objects_size += size;
  moved_objects_per_gc++;
}

// Per GC thread we count the number of forwarding references from
// objects in H1 to objects in H2 during the marking phase.
void TeraStatistics::add_forward_ref(unsigned int references,
                                       unsigned int worker_id) {
  assert(worker_id < ParallelGCThreads, "Out-of-bound access");
  forward_ref[worker_id] += references;
}
  
// Increase by one the number of backward references per full GC;
void TeraStatistics::add_back_ref() {
  backward_ref++;
}
  
// Update the distribution of objects size. We divide the objects
// into three categories:
//  Bytes
//  KBytes
//  MBytes
void TeraStatistics::update_object_distribution(size_t size) {
  size_t obj_size = (size * HeapWordSize) / 1024UL;
  int count = 0;

  while (obj_size > 0) {
    count++;
    obj_size/=1024UL;
  }

  assert(count <=2, "Array out of range");
  obj_distr_size[count]++;
}
  
// Update the number of forwarding tables 
void TeraStatistics::add_fwd_tables() {
  num_fwd_tables++;
}

// Print the statistics of TeraHeap at the end of each FGC
// Will print:
//	- the total forward references from the H1 to the H2
//	- the total backward references from H2 to the H1
//	- the total objects that has been moved to H2
//	- the current total size of objects in H2
//	- the current total objects that are moved in H2
void TeraStatistics::print_major_gc_stats() {
  size_t total_fwd_ref = 0; 

  for (unsigned int i = 0; i < ParallelGCThreads; i++)
    total_fwd_ref += forward_ref[i];

	thlog_or_tty->print_cr("[STATISTICS] | TOTAL_FORWARD_TABLES = %lu\n", num_fwd_tables);
	thlog_or_tty->print_cr("[STATISTICS] | TOTAL_FORWARD_PTRS = %lu\n", total_fwd_ref);
	thlog_or_tty->print_cr("[STATISTICS] | TOTAL_BACK_PTRS = %lu\n", backward_ref);
	thlog_or_tty->print_cr("[STATISTICS] | TOTAL_TRANS_OBJ = %lu\n", moved_objects_per_gc);

	thlog_or_tty->print_cr("[STATISTICS] | TOTAL_OBJECTS  = %lu\n", total_objects);
	thlog_or_tty->print_cr("[STATISTICS] | TOTAL_OBJECTS_SIZE = %lu\n", total_objects_size);
	thlog_or_tty->print_cr("[STATISTICS] | DISTRIBUTION | B = %lu | KB = %lu | MB = %lu\n",
			obj_distr_size[0], obj_distr_size[1], obj_distr_size[2]);

	thlog_or_tty->print_cr("[STATISTICS] | NUM_PRIMITIVE_ARRAYS  = %lu\n", num_primitive_arrays);
	thlog_or_tty->print_cr("[STATISTICS] | PRIMITIVE_ARRAYS_SIZE  = %lu\n", primitive_arrays_size);
  thlog_or_tty->print_cr("[STATISTICS] | NUM_PRIMITIVE_OBJ  = %lu\n", num_primitive_obj);
	thlog_or_tty->print_cr("[STATISTICS] | PRIMITIVE_OBJ_SIZE  = %lu\n", primitive_obj_size);
  thlog_or_tty->print_cr("[STATISTICS] | NUM_NON_PRIMITIVE_OBJ  = %lu\n", num_non_primitive_obj);
	thlog_or_tty->print_cr("[STATISTICS] | NON_PRIMITIVE_OBJ_SIZE  = %lu\n", non_primitive_obj_size);

  thlog_or_tty->flush();

  // Init the statistics counters of TeraHeap to zero for the next GC
  memset(forward_ref, 0, ParallelGCThreads * sizeof(uint64_t));
  backward_ref = 0;
  moved_objects_per_gc = 0;
  num_fwd_tables = 0;
  primitive_arrays_size = 0;
  primitive_obj_size = 0;
  non_primitive_obj_size = 0;
  num_primitive_arrays = 0;
  num_primitive_obj = 0;
  num_non_primitive_obj = 0;
}


// Update the statistics for primitive arrays (e.g., char[], int[]).
// Keep the number of 'instances' and their 'total_size' per major GC.
void TeraStatistics::add_primitive_arrays_stats(size_t instances, size_t total_size) {
  num_primitive_arrays += instances;
  primitive_arrays_size += total_size;
}

// Update the statistics for objects with only primitive type fields.
// Keep the number of 'instances' and their 'total_size' per major GC.
void TeraStatistics::add_primitive_obj_stats(size_t instances, size_t total_size) {
  num_primitive_obj += instances;
  primitive_obj_size += total_size;
}
  
// Update the statistics for objects with non primitive type fields.
// Keep the number of 'instances' and their 'total_size' per major GC.
void TeraStatistics::add_non_primitive_obj_stats(size_t instances, size_t total_size) {
  num_non_primitive_obj += instances;
  non_primitive_obj_size += total_size;
}
