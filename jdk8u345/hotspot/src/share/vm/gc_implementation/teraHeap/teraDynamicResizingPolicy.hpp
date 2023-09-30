#ifndef SHARE_VM_GC_IMPLEMENTATION_TERAHEAP_TERADYNAMICRESIZINGPOLICY_HPP
#define SHARE_VM_GC_IMPLEMENTATION_TERAHEAP_TERADYNAMICRESIZINGPOLICY_HPP

#include "gc_implementation/teraHeap/teraEnum.h"
#include "gc_implementation/teraHeap/teraStateMachine.hpp"
#include "memory/allocation.hpp"
#include "memory/sharedDefines.h"
#include <stdlib.h>
#include <string.h>

#define HIST_SIZE 5
#define GC_HIST_SIZE 1
#define NUM_ACTIONS 8
#define NUM_STATES 4
#define NAME_LEN 20

class TeraDynamicResizingPolicy : public CHeapObj<mtInternal> {
private:
  char state_name[NUM_STATES][NAME_LEN]; //< Define state names
  char action_name[NUM_ACTIONS][NAME_LEN]; //< Define state names
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

  actions cur_action;                 //< Current action

  states cur_state;                   // Current state

  size_t h2_cand_size_in_bytes;       //< H2 candidate objects
                                      // size in bytes

  double prev_full_gc_end;            //< Track when the previous full
                                      // gc has been ended
  double last_full_gc_start;          //< Track when the last gc

  double last_minor_gc;               //< Time happened the last minor
                                      // gc

  bool need_action;                   //< Flag that indicate if we
                                      // need to perform an action.
                                      // This flag is set by mutator
                                      // threads.
  double hist_gc_time[GC_HIST_SIZE];  // History of the gc time in
                                      // previous intervals
  double hist_iowait_time[HIST_SIZE]; // History of the iowait time in
                                      // previous intervals

  size_t shrinked_bytes;              // shrinked bytes
  
  size_t prev_page_cache_size;        // Current size of page cache
                                      // before shrink operation
  
  uint64_t window_interval;           // window_interval;
  
  bool transfer_hint_enabled;         // Check if objects have been transfered to H2 using 

  double gc_compaction_phase_ms;      // Time of the compaction phase
                                      // that includes I/O 
  TeraStateMachine *state_machine;    // FSM

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

  // Find the average of the array elements
  double calc_avg_time(double *arr, int size);

  // After each growing operation of H1 we wait to see the effect of
  // the action. If we reach a gc or the io cost is higher than gc
  // cost then we go to no action state. 
  void state_wait_after_grow(double io_time_ms, double gc_time_ms);

  // Initialize the array of state names
  void init_state_actions_names();

  // Intitilize the policy of the state machine.
  TeraStateMachine* init_state_machine_policy();
  
  // Calculation of the GC cost prediction.
  double calculate_gc_cost(double gc_time_ms);

  // Print states (for debugging and logging purposes)
  void print_state_action();

  // GrowH1 action
  void action_grow_h1(bool *need_full_gc);
  
  // ShrinkH1 action
  void action_shrink_h1();

  // Move objects from H2 to H1
  // TODO: Add the source code of Kwstas here
  void action_move_back();
  
  // Calculate the average of gc and io costs and return their values.
  // We use these values to determine the next actions.
  void calculate_gc_io_costs(double *avg_gc_time_ms, double *avg_io_time_ms,
                             uint64_t *device_active_time_ms);
  
  // Print counters for debugging purposes
  void debug_print(double avg_iowait_time, double avg_gc_time, double interval,
                   double cur_iowait_time, double cur_gc_time);
  
  // Save the history of the GC and iowait overheads. We maintain two
  // ring buffers (one for GC and one for iowait) and update these
  // buffers with the new values for GC cost and IO overhead.
  void history(double gc_time, double iowait_time_ms);
  
  // Set current time since last window
  void reset_counters();

public:
  // Constructor
  TeraDynamicResizingPolicy();

  // Destructor
  ~TeraDynamicResizingPolicy() {}

  // Init the iowait timer at the begining of the major GC.
  void gc_start(double start_time);
  
  // Count the iowait time during gc and update the gc_iowait_time_ms
  // counter for the current window
  void gc_end(double gctime, double last_full_gc);

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
  void decrease_h2_candidate_size(size_t size);

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


  // We set the time when the last minor gc happened. For cases that
  // IO is high (the whole computation happens in H2) and low number
  // of objects are created in the young generation, we have to check
  // through the VMThread if we need to perfom an action. 
  void set_last_minor_gc(double time) {
    last_minor_gc = time;
  }
  
  // Get when the last minor GC happened
  double get_last_minor_gc(void) {
    return last_minor_gc;
  }

  // VMThread enables DynaHeap to perform action. For cases that
  // IO is high (the whole computation happens in H2) and low number
  // of objects are created in the young generation.
  void action_enabled(void) {
    need_action = true; 
  }

  // Reset the action enabled by VMThread.
  void action_disabled(void) {
    need_action = false; 
  }

  // Check if the VMThread enable an action.
  bool is_action_enabled() {
    return need_action;
  }
  
  // Set the number of shrinked bytes when a SHRINK_H1 operation
  // occured.
  void set_shrinked_bytes(size_t bytes) {
    shrinked_bytes = bytes;
  }

  // Before as shrinking operation we save the size of the page cache.
  void set_current_size_page_cache() {
    prev_page_cache_size = state_machine->read_cgroup_mem_stats(true);
  }

  // Get the window interval time.
  uint64_t get_window_interval() {
    // Transform milliseconds to seconds
    return window_interval / 1000;
  }

  // Grow the capacity of H1.
  size_t grow_h1();

  // Shrink the capacity of H1 to increase the page cache size.
  size_t shrink_h1();

  // Accumulate the time of the compaction phase of the major GC when
  // it moves objects to H2. The compaction phase contains the I/O
  // wait time for transfering objects to H2.
  void compaction_phase_time(double time) {
    gc_compaction_phase_ms += (transfer_hint_enabled) ? time : 0;
  }

  // We call this function after moving objects to H2 to reste the
  // state and the counters.
  void epilog_move_h2(bool full_gc_done, bool need_resizing);

  void dram_repartition(bool *need_full_gc, bool *eager_move);
};

#endif // SHARE_VM_GC_IMPLEMENTATION_TERAHEAP_TERADYNAMICRESIZINGPOLICY_HPP
