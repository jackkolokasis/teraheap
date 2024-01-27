#ifndef SHARE_GC_TERAHEAP_TERASTATISTICS_HPP
#define SHARE_GC_TERAHEAP_TERASTATISTICS_HPP

#include "memory/allocation.hpp"
#include "oops/oop.hpp"

class TeraStatistics: public CHeapObj<mtInternal> {
private:
  size_t  total_objects;               //< Total number of objects located in TeraHeap
  size_t  total_objects_size;          //< Total number of objects size

  size_t* forward_ref;                 //< Total number of forward ptrs per FGC
  size_t  backward_ref;                //< Total number of back ptrs per FGC
  size_t  moved_objects_per_gc;        //< Total number of objects transfered to
                                       //< TeraHeap per FGC

  size_t num_fwd_tables;               //< Number of forwarding tables

  size_t obj_distr_size[3];            //< Object size distribution between B, KB, MB

  size_t primitive_arrays_size;        //< Total size of promitive arrays
  size_t primitive_obj_size;           //< Total size of objects with ONLY primitive type fields 
  size_t non_primitive_obj_size;       //< Total size of objects with non primitive type fields
  
  size_t num_primitive_arrays;         //< Total number of promitive arrays instances
  size_t num_primitive_obj;            //< Total number of objects with ONLY primitive type fields 
  size_t num_non_primitive_obj;        //< Total size of objects with non primitive type fields

public:

  TeraStatistics();
  ~TeraStatistics();

  // Increase by one the counter that shows the total number of
  // objects that are moved to H2. Increase by 'size' the counter that shows
  // the total size of the objects that are moved to H2. Increase by one
  // the number of objects that are moved in the current gc cycle to H2.
  void add_object(size_t size);

  // Per GC thread we count the number of forwarding references from
  // objects in H1 to objects in H2 during the marking phase.
  void add_forward_ref(unsigned int references, unsigned int worker_id);

  // Increase by one the number of backward references per full GC;
  void add_back_ref();

  // Update the distribution of objects size. We divide the objects
  // into three categories: (1) Bytes, (2) KBytes, and (3) MBytes
  void update_object_distribution(size_t size);
  
  // Update the number of forwarding tables 
  void add_fwd_tables();
  
  // Print the statistics of TeraHeap at the end of each FGC
  // Will print:
  //	- the total forward references from the H1 to the H2
  //	- the total backward references from H2 to the H1
  //	- the total objects that has been moved to H2
  //	- the current total size of objects in H2
  //	- the current total objects that are moved in H2
  void print_major_gc_stats();

  // Update the statistics for primitive arrays (e.g., char[], int[]).
  // Keep the number of 'instances' and their 'total_size' per major GC.
  void add_primitive_arrays_stats(size_t instances, size_t total_size);
  
  // Update the statistics for objects with only primitive type fields.
  // Keep the number of 'instances' and their 'total_size' per major GC.
  void add_primitive_obj_stats(size_t instances, size_t total_size);
  
  // Update the statistics for objects with non primitive type fields.
  // Keep the number of 'instances' and their 'total_size' per major GC.
  void add_non_primitive_obj_stats(size_t instances, size_t total_size);
};

#endif // SHARE_GC_TERAHEAP_TERASTATISTICS_HPP
