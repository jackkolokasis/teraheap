#ifndef SHARE_GC_TERAHEAP_TERASTATISTICS_HPP
#define SHARE_GC_TERAHEAP_TERASTATISTICS_HPP

#include "memory/allocation.hpp"
#include "oops/oop.hpp"

class TeraStatistics: public CHeapObj<mtInternal> {
private:
  long  total_objects_moved;               
  long  total_objects_size; 
  long  backward_ref;

  // time to scan h2 card table
  double h2_card_table_scan_time_ms;
  // time for evacuation to be completed
  double evac_time_ms; 

  bool is_mixed_gc;   
  
public:

  TeraStatistics();

  // Increase by one the counter that shows the total number of
  // objects that are moved to H2. Increase by 'size' the counter that shows
  // the total size of the objects that are moved to H2. Increase by one
  // the number of objects that are moved in the current gc cycle to H2.
  void add_object(long size);


  // Increase by one the number of backward references per full GC;
  void add_back_ref();


  // Print the statistics of TeraHeap at the end of each FGC
  // Will print:
  //	- the total forward references from the H1 to the H2
  //	- the total backward references from H2 to the H1
  //	- the total objects that has been moved to H2
  //	- the current total size of objects in H2
  //	- the current total objects that are moved in H2
  void print_gc_stats();

  void record_h2_scan_time(double time){
    h2_card_table_scan_time_ms = time;
  }

  void record_evacuation_time(double time ){
    evac_time_ms = time;
  }

  void set_is_in_mix(bool is_mix_gc){    
    is_mixed_gc = is_mix_gc;
  }

};

#endif // SHARE_GC_TERAHEAP_TERASTATISTICS_HPP
