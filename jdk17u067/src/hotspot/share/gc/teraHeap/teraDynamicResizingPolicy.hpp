#include "memory/allocation.hpp"

#include <stdlib.h>
#include <string.h>

#ifndef SHARE_GC_TERAHEAP_TERADYNAMICRESIZINGPOLICY_HPP
#define SHARE_GC_TERAHEAP_TERADYNAMICRESIZINGPOLICY_HPP

class TeraDynamicResizingPolicy : public CHeapObj<mtInternal> {
public:
  enum state {
    S_NO_ACTION,                          //< Do not perform any action
    S_SHRINK_H1,                          //< Shrink H1 because the I/O is high
    S_GROW_H1,                            //< Grow H1 because the GC is high
    S_MOVE_BACK,                          //< Move obects from H2 to H1 
    S_CONTINUE,                           //< Continue not finished interval
    S_MOVE_H2,                            //< Transfer objects to H2
  };

private:

  uint64_t window_start_time;             //< Window start time
  static unsigned long long iowait_start; //< Start counting iowait time at
                                          // the start of the window
  static unsigned long long cpu_start;    //< Start counting iowait time at
                                          // the start of the window

  static unsigned long long gc_iowait_start; //< IO wait time created during gc
  static unsigned long long gc_cpu_start;    //< IO wait time created during gc
  static double gc_iowait_time_ms;        //< Total IO wait time generated
                                          // in GC cycles for the current
                                          // window.
  static uint64_t gc_dev_time;            //< Total time that the device
                                          // was active during GC
  static uint64_t gc_dev_start;           //< Start counting the H2 device
                                          // utilization at the start of GC
  static uint64_t dev_time_start;         //< Start to count the active
                                          // device time
  static double gc_time;                  //< Total gc time for the
                                          // interval of the window
  double interval;                        //< Interval of the window
  state prev_action;                      //< Previous action

  // Check if the window limit exceed time
  bool is_window_limit_exeed();

  // Calculate ellapsed time
  double ellapsed_time(uint64_t start_time, uint64_t end_time);

  // This function opens iostat and read the io wait time at the
  // current time.
  static void read_cpu_stats(unsigned long long* cpu_iowait,
                             unsigned long long* total_cpu);
  
  // Calculate iowait time based on the following formula
  //
  //                (cpu_iowait_after - cpu_iowait_before) 
  //  iowait_time = -------------------------------------- * duration 
  //                 (total_cpu_after - total_cpu_before)
  //
  static void calc_iowait_time(unsigned long long cpu_iowait_before,
                               unsigned long long cpu_iowait_after,
                               unsigned long long total_cpu_before,
                               unsigned long long total_cpu_after,
                               double duration, double* iowait_time);

  // Count timer. We avoid to use os::elapsed_time() because internally
  // uses the clock_get_time which adds extra overhead. This function
  // is executed in the common path.
  static uint64_t rdtsc() {
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
  }

  // Destructor
  ~TeraDynamicResizingPolicy() {
  }
  
  // Set current time since last window
  void reset_counters();

  // Init the iowait timer at the begining of the major GC.
  static void gc_start();
  
  // Count the iowait time during gc and update the gc_iowait_time_ms
  // counter for the current window
  static void gc_end(double gctime);

  // According to the usage of the old generation and the io wait time
  // we perform an action. This action triggers to grow H1 or shrink
  // H1.
  state action();

  // Get the total time in milliseconds that the device is active.
  // This fucntion utilizes the /sys/block/<dev>/stat file to read the
  // read and write ticks.
  // Returns: read ticks + write ticks
  static uint64_t get_device_active_time(const char* device);

};

#endif // SHARE_GC_TERAHEAP_TERADYNAMICRESIZINGPOLICY_HPP
