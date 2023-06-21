#include "memory/allocation.hpp"

#include <stdlib.h>
#include <string.h>

#ifndef SHARE_VM_GC_IMPLEMENTATION_TERAHEAP_TERADYNAMICRESIZINGPOLICY_HPP
#define SHARE_VM_GC_IMPLEMENTATION_TERAHEAP_TERADYNAMICRESIZINGPOLICY_HPP

class TeraDynamicResizingPolicy : public CHeapObj<mtInternal> {
public:
  enum state {
    S_NO_ACTION,                      //< Do not perform any action
    S_SHRINK_H1,                      //< Shrink H1 because the I/O is high
    S_GROW_H1,                        //< Grow H1 because the GC is high
    S_MOVE_BACK,                      //< Move obects from H2 to H1 
    S_CONTINUE,                       //< Continue not finished interval
    S_MOVE_H2,                        //< Transfer objects to H2
  };

private:
  uint64_t window_start_time;         //< Window start time
  unsigned long long iowait_start;    //< Start counting iowait time at
                                      // the start of the window
  unsigned long long cpu_start;       //< Start counting iowait time at
                                      // the start of the window

  unsigned long long gc_iowait_start; //< IO wait time created during gc
  unsigned long long gc_cpu_start;    //< IO wait time created during gc
  double gc_iowait_time_ms;           //< Total IO wait time generated
  uint64_t gc_dev_time;               //< Total time that the device
                                      // was active during GC
  uint64_t gc_dev_start;              //< Start counting the H2 device
                                      // utilization at the start of GC
  uint64_t dev_time_start;            //< Start to count the active
                                      // device time
  double gc_time;                     //< Total gc time for the
                                      // interval of the window
  double interval;                    //< Interval of the window
  state prev_action;                  //< Previous action

  size_t h2_cand_size_in_bytes;       //< H2 candidate objects
                                      // size in bytes

  double last_full_gc_ms;             //< Track when the last gc
                                      // has been performed

  // Check if the window limit exceed time
  bool is_window_limit_exeed();

  // Calculate ellapsed time
  double ellapsed_time(uint64_t start_time, uint64_t end_time);

  // This function opens iostat and read the io wait time at the
  // current time.
  void read_cpu_stats(unsigned long long* cpu_iowait,
                      unsigned long long* total_cpu);
  
  // Calculate iowait time based on the following formula
  //
  //                (cpu_iowait_after - cpu_iowait_before) 
  //  iowait_time = -------------------------------------- * duration 
  //                 (total_cpu_after - total_cpu_before)
  //
  void calc_iowait_time(unsigned long long cpu_iowait_before,
                               unsigned long long cpu_iowait_after,
                               unsigned long long total_cpu_before,
                               unsigned long long total_cpu_after,
                               double duration, double* iowait_time);

  // Count timer. We avoid to use os::elapsed_time() because internally
  // uses the clock_get_time which adds extra overhead. This function
  // is executed in the common path.
  uint64_t rdtsc() {
    unsigned int lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
  }

public:
  // Constructor
  TeraDynamicResizingPolicy() {
    window_start_time = rdtsc();
    read_cpu_stats(&iowait_start, &cpu_start);
    gc_iowait_time_ms = 0;
    gc_time = 0;
    dev_time_start = get_device_active_time("nvme1n1");
    gc_dev_time = 0;
    prev_action = S_CONTINUE;
    last_full_gc_ms = 0;
  }

  // Destructor
  ~TeraDynamicResizingPolicy() {
  }
  
  // Set current time since last window
  void reset_counters();

  // Init the iowait timer at the begining of the major GC.
  void gc_start();
  
  // Count the iowait time during gc and update the gc_iowait_time_ms
  // counter for the current window
  void gc_end(double gctime, double last_full_gc);

  // According to the usage of the old generation and the io wait time
  // we perform an action. This action triggers to grow H1 or shrink
  // H1.
  state action();

  // Get the total time in milliseconds that the device is active.
  // This fucntion utilizes the /sys/block/<dev>/stat file to read the
  // read and write ticks.
  // Returns: read ticks + write ticks
  uint64_t get_device_active_time(const char* device);

  // Increase the size of H2 candidate objects that are in H1 and
  // should be moved to H2. We measure only H2 candidates objects that
  // are primitive arrays and leaf objects.
  void increase_h2_candidate_size(size_t size) {
    h2_cand_size_in_bytes += size * HeapWordSize;
  }
  
  // Decrease the size of H2 candidate objects that are in H1 and
  // should be moved to H2. We measure only H2 candidates objects that
  // are primitive arrays and leaf objects.
  void decrease_h2_candidate_size(size_t size) {
    h2_cand_size_in_bytes -= size * HeapWordSize;
    if (h2_cand_size_in_bytes < 0)
      h2_cand_size_in_bytes = 0;
  }

  // At the start each major GC we should reset the counter of the
  // size of h2 candidate objects in H1.
  void reset_h2_candidate_size() {
    h2_cand_size_in_bytes = 0;
  }
  
  // At the start each major GC we should reset the counter of the
  // size of h2 candidate objects in H1.
  size_t get_h2_candidate_size() {
    return h2_cand_size_in_bytes;
  }

  // Print counters for debugging purposes
  void debug_print(double iowait_time_ms, uint64_t device_active_time_ms,
                   double interval);
};

#endif // SHARE_VM_GC_IMPLEMENTATION_TERAHEAP_TERADYNAMICRESIZINGPOLICY_HPP
