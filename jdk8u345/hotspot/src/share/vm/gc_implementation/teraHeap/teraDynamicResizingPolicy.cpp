#include "gc_interface/collectedHeap.hpp"
#include "gc_implementation/parallelScavenge/parallelScavengeHeap.hpp"
#include "gc_implementation/teraHeap/teraDynamicResizingPolicy.hpp"
#include "memory/universe.hpp"
#include "gc_implementation/teraHeap/teraHeap.hpp"

#define BUFFER_SIZE 1024
#define CYCLES_PER_SECOND 2.4e9; // CPU frequency of 2.4 GHz
#define TRANSFER_THRESHOLD 0.4f
#define GC_FREQUENCY (10)
#define SMALL_INTERVAL ((2LL * 1000)) 
#define REGULAR_INTERVAL ((10LL * 1000)) 
#define HIGH_PRESSURE 0.75
#define IOSLACK_THRESHOLD ((1*1024*1024*1024LU))

TeraDynamicResizingPolicy::TeraDynamicResizingPolicy() {
  window_start_time = rdtsc();
  read_cpu_stats(&iowait_start, &cpu_start);
  gc_iowait_time_ms = 0;
  gc_time = 0;
  dev_time_start = get_device_active_time("nvme1n1");
  gc_dev_time = 0;
  prev_action = S_CONTINUE;

  memset(hist_gc_time, 0, GC_HIST_SIZE * sizeof(double));
  memset(hist_iowait_time, 0, HIST_SIZE * sizeof(double));

  window_interval = REGULAR_INTERVAL;
  transfer_hint_enabled = false;
  prev_full_gc_end = 0;
  last_full_gc_start = 0;
  gc_compaction_phase_ms = 0;

  init_state_names();
}
  
// Calculate ellapsed time
double TeraDynamicResizingPolicy::ellapsed_time(uint64_t start_time,
                                                uint64_t end_time) {

  double elapsed_time = (double)(end_time - start_time) / CYCLES_PER_SECOND;
  return (elapsed_time * 1000.0);
}

// Set current time since last window
void TeraDynamicResizingPolicy::reset_counters() {
  window_start_time = rdtsc();
  read_cpu_stats(&iowait_start, &cpu_start);
  gc_time = 0;
  gc_dev_time = 0;
  gc_iowait_time_ms = 0;
  dev_time_start = get_device_active_time("nvme1n1");
  transfer_hint_enabled = false;
  gc_compaction_phase_ms = 0;
  window_interval = REGULAR_INTERVAL;
}

// Check if the window limit exceed time
bool TeraDynamicResizingPolicy::is_window_limit_exeed() {
  uint64_t window_end_time;
  static int i = 0;
  window_end_time = rdtsc();
  interval = ellapsed_time(window_start_time, window_end_time);

#ifdef PER_MINOR_GC
  return true;
#else
  return (interval >= window_interval) ? true : false;
#endif
}

// Init the iowait timer at the begining of the major GC.
void TeraDynamicResizingPolicy::gc_start(double start_time) {
  read_cpu_stats(&gc_iowait_start, &gc_cpu_start);
  gc_dev_start = get_device_active_time("nvme1n1");
#ifdef GC_DISTRIBUTION
  last_full_gc_start = start_time;
#endif
}

// Count the iowait time during gc and update the gc_iowait_time_ms
// counter for the current window
void TeraDynamicResizingPolicy::gc_end(double gc_duration, double last_full_gc) {
  unsigned long long gc_iowait_end = 0, gc_cpu_end = 0, gc_blk_io_end = 0;
  uint64_t gc_dev_end = 0;
  double iowait_time = 0;
  static double last_gc_end = 0;

  read_cpu_stats(&gc_iowait_end, &gc_cpu_end);
  calc_iowait_time(gc_iowait_start, gc_iowait_end, gc_cpu_start, gc_cpu_end,
                   gc_duration, &iowait_time);
  gc_iowait_time_ms += iowait_time;

  gc_dev_end = get_device_active_time("nvme1n1");
  gc_dev_time += (gc_dev_end - gc_dev_start);

  gc_time += gc_duration;

  prev_full_gc_end = last_gc_end;
  last_gc_end = last_full_gc;

//#ifdef FLUSH_GC_HISTORY_AFTER_MOVEH2
  //if (transfer_hint_enabled) {
  //  reset_counters();
  //  memset(hist_gc_time, 0, HIST_SIZE * sizeof(double));
  //}
  //if(transfer_hint_enabled) {
  //  // substract compaction phase because it has I/O time to the device
  //  gc_compaction_phase_ms;
  //}
//#endif
}

// This function opens iostat and read the io wait time in percentage
void TeraDynamicResizingPolicy::read_cpu_stats(unsigned long long* cpu_iowait,
                                               unsigned long long* total_cpu) {
  FILE* stat_file = fopen("/proc/stat", "r");
  if (!stat_file) {
    fprintf(stderr, "Failed to open /proc/stat\n");
    return;
  }

  char buffer[BUFFER_SIZE];
  unsigned long long user, nice, system, idle, iowait, irq, softirq;

  while (fgets(buffer, BUFFER_SIZE, stat_file)) {
    if (strncmp(buffer, "cpu", 3) == 0) {
      sscanf(buffer, "%*s %llu %llu %llu %llu %llu %llu %llu", &user, &nice, &system, &idle, &iowait, &irq, &softirq);

      *total_cpu = user + nice + system + iowait + irq + softirq + idle;
      *cpu_iowait = iowait + idle;

      break;
    }
  }

  int res = fclose(stat_file);
  if (res != 0) {
    fprintf(stderr, "Error closing file");
  }
}

// Calculate iowait time based on the following formula
//
//                (cpu_iowait_after - cpu_iowait_before) 
//  iowait_time = -------------------------------------- * duration 
//                 (total_cpu_after - total_cpu_before)
//
void TeraDynamicResizingPolicy::calc_iowait_time(unsigned long long cpu_iowait_before,
                                                 unsigned long long cpu_iowait_after,
                                                 unsigned long long total_cpu_before,
                                                 unsigned long long total_cpu_after,
                                                 double duration, double* iowait_time) {

  unsigned long long iowait_diff = cpu_iowait_after - cpu_iowait_before;
  unsigned long long cpu_diff = total_cpu_after - total_cpu_before;

  if (cpu_diff == 0) {
    *iowait_time = 0;
    fprintf(stderr, "iowait_time = 0\n");
  }
  else
    *iowait_time = ((double) iowait_diff / cpu_diff) * duration;
}

uint64_t TeraDynamicResizingPolicy::get_device_active_time(const char* device) {
  char file_path[256];
  snprintf(file_path, sizeof(file_path), "/sys/block/%s/stat", device);
  FILE* dev_file = fopen(file_path, "r");

  if (!dev_file) {
    printf("Failed to open device statistics file.\n");
    return 0;
  }

  char line[BUFFER_SIZE];
  int res;
  
  while (fgets(line, sizeof(line), dev_file) != NULL) {
    uint64_t readIOs, readMerges, readSectors, readTicks, writeIOs, writeMerges, writeSectors, writeTicks;
    res = sscanf(line, "%lu %lu %lu %lu %lu %lu %lu %lu", &readIOs, &readMerges, &readSectors, &readTicks, &writeIOs, &writeMerges, &writeSectors, &writeTicks);

    if (res != 8) {
      fprintf (stderr, "Error reading device usage\n");
      return 0;
    }
  
    res = fclose(dev_file);
    if (res != 0) {
      fprintf(stderr, "Error closing file");
      return 0;
    }

    return readTicks + writeTicks;
  }

  return 0;
}

// Execute an action based on the state machine. This is a wrapper for
// simple_action() and optimized_action().
TeraDynamicResizingPolicy::state TeraDynamicResizingPolicy::action() {
#ifdef BASIC_VERSION
  return simple_action();
#else
  return optimized_action();
#endif
}

// According to the usage of the old generation and the io wait time
// we perform an action. This action triggers to grow H1 or shrink
// H1.
TeraDynamicResizingPolicy::state TeraDynamicResizingPolicy::optimized_action() {
  double iowait_time_ms = 0;
  uint64_t device_active_time_ms = 0, dev_time_end = 0;
  unsigned long long iowait_end = 0, cpu_end = 0;
  static int i = 0;

  // Check if we are inside the window
  if (!is_window_limit_exeed())
    return S_CONTINUE;

  // Calculate the user and iowait time during the window interval
  read_cpu_stats(&iowait_end, &cpu_end);
  calc_iowait_time(iowait_start, iowait_end, cpu_start, cpu_end,
                   interval, &iowait_time_ms);
  //mjr_faults = (get_mjr_faults() - mjr_faults_start) - gc_mjr_faults;

  iowait_time_ms -= gc_iowait_time_ms;
  dev_time_end = get_device_active_time("nvme1n1");
  device_active_time_ms = (dev_time_end - dev_time_start) - gc_dev_time;

  assert(gc_time <= interval, "GC time should be less than the window interval");
  assert(iowait_time_ms <= interval, "GC time should be less than the window interval");
  assert(device_active_time_ms <= interval, "GC time should be less than the window interval");

  if (iowait_time_ms < 0)
    iowait_time_ms = 0;

  hist_gc_time[i % HIST_SIZE] = gc_time;
  hist_iowait_time[i % HIST_SIZE] = iowait_time_ms;
  i++;

  if (i < HIST_SIZE) {
    prev_action = S_NO_ACTION;
    return S_NO_ACTION;
  }

  double avg_iowait_time = calc_avg_time(hist_iowait_time, HIST_SIZE);
  double avg_gc_time = calc_avg_time(hist_gc_time, GC_HIST_SIZE);
  
  if (TeraHeapStatistics)
    debug_print(avg_iowait_time, avg_gc_time, interval, iowait_time_ms, gc_time);

  if (avg_gc_time + avg_iowait_time < (0.1 * interval)) {
    prev_action = S_NO_ACTION;
    return S_NO_ACTION;
  }

  // We calculate the ratio of the h2 candidate objects in H1 
  double h2_candidate_ratio = (double) h2_cand_size_in_bytes / Universe::heap()->capacity();

  if (prev_action != S_SHRINK_H1 && avg_gc_time >= avg_iowait_time &&  h2_candidate_ratio >= TRANSFER_THRESHOLD) {
    prev_action = S_MOVE_H2;
    return S_MOVE_H2;
  }

  PSOldGen *old_gen = ParallelScavengeHeap::old_gen();
  
  if (avg_gc_time >= avg_iowait_time && old_gen->capacity_in_bytes() < old_gen->max_gen_size()) {
    prev_action = S_GROW_H1;
    return S_GROW_H1;
  }

  if (avg_iowait_time > avg_gc_time && device_active_time_ms > 0 ) {
    if (prev_action == S_SHRINK_H1) {
      size_t diff = read_cgroup_mem_stats(true) - prev_page_cache_size;

      if (shrinked_bytes > diff) {
        if (TeraHeapStatistics) {
          thlog_or_tty->print_cr("shrink_with_no_action diff=%lu\n", diff);
          thlog_or_tty->flush();
        }
        return S_NO_ACTION;
      }
    }
    prev_action = S_SHRINK_H1;
    return S_SHRINK_H1;
  }

  prev_action = S_NO_ACTION;
  return S_NO_ACTION;
}

// According to the usage of the old generation and the io wait time
// we perform an action. This action triggers to grow H1 or shrink
// H1.
// This is the simple version of the state machine. There is no optimizations.
TeraDynamicResizingPolicy::state TeraDynamicResizingPolicy::simple_action() {
  double iowait_time_ms = 0;
  uint64_t device_active_time_ms = 0, dev_time_end = 0;
  unsigned long long iowait_end = 0, cpu_end = 0;
  static int i = 0;

  // Check if we are inside the window
  if (!is_window_limit_exeed())
    return S_CONTINUE;

  // Calculate the user and iowait time during the window interval
  read_cpu_stats(&iowait_end, &cpu_end);
  calc_iowait_time(iowait_start, iowait_end, cpu_start, cpu_end,
                   interval, &iowait_time_ms);

  iowait_time_ms -= gc_iowait_time_ms;
  dev_time_end = get_device_active_time("nvme1n1");
  device_active_time_ms = (dev_time_end - dev_time_start) - gc_dev_time;

  assert(gc_time <= interval, "GC time should be less than the window interval");
  assert(iowait_time_ms <= interval, "GC time should be less than the window interval");
  assert(device_active_time_ms <= interval, "GC time should be less than the window interval");

  if (iowait_time_ms < 0 || device_active_time_ms <= 0)
    iowait_time_ms = 0;

  history(i, gc_time - gc_compaction_phase_ms, iowait_time_ms);
  i++;

  if (i < HIST_SIZE) {
    cur_action = S_NO_ACTION;
    return S_NO_ACTION;
  }

#ifdef LAZY_MOVE_H2
  if (cur_action == S_MOVE_H2) {
    cur_action = S_MOVE_H2;
    return S_MOVE_H2;
  }
#endif

  double avg_iowait_time = calc_avg_time(hist_iowait_time, HIST_SIZE);
  double avg_gc_time = calc_avg_time(hist_gc_time, GC_HIST_SIZE);

#ifdef WAIT_AFTER_GROW
  state res = should_wait_after_grow(avg_iowait_time, avg_gc_time);
  if (res != S_CONTINUE) {
    cur_action = res;
    return res;
  }
#endif

  if (TeraHeapStatistics)
    debug_print(avg_iowait_time, avg_gc_time, interval, iowait_time_ms, gc_time);

  // If the difference between GC and iowait is up to 100 ms then  do
  // not perform any operation
  if (abs(avg_gc_time - avg_iowait_time) <= 100) {
    cur_action = S_NO_ACTION;
    return S_NO_ACTION;
  }

  // We calculate the ratio of the h2 candidate objects in H1 
  double h2_candidate_ratio = (double) h2_cand_size_in_bytes / Universe::heap()->capacity();
  if (avg_gc_time >= avg_iowait_time &&  h2_candidate_ratio >= TRANSFER_THRESHOLD) {
    cur_action = S_MOVE_H2;
    return S_MOVE_H2;
  }

  PSOldGen *old_gen = ParallelScavengeHeap::old_gen();
  bool under_h1_max_limit = old_gen->capacity_in_bytes() < old_gen->max_gen_size();

  if (cur_action != S_MOVE_H2 && avg_gc_time >= avg_iowait_time && under_h1_max_limit) {
    cur_action = S_GROW_H1;
    return S_GROW_H1;
  }

  if (avg_iowait_time > avg_gc_time && device_active_time_ms > 0) {
    return state_shrink_h1();
  }

  cur_action = S_NO_ACTION;
  return S_NO_ACTION;
}

// Print counters for debugging purposes
void TeraDynamicResizingPolicy::debug_print(double avg_iowait_time, double avg_gc_time,
                                            double interval, double cur_iowait_time, double cur_gc_time) {
  thlog_or_tty->print_cr("avg_iowait_time_ms = %lf\n", avg_iowait_time);
  thlog_or_tty->print_cr("avg_gc_time_ms = %lf\n", avg_gc_time);
  thlog_or_tty->print_cr("cur_iowait_time_ms = %lf\n", cur_iowait_time);
  thlog_or_tty->print_cr("cur_gc_time_ms = %lf\n", cur_gc_time);
  thlog_or_tty->print_cr("interval = %lf\n", interval);
  thlog_or_tty->flush();
}

// Find the average of the array elements
double TeraDynamicResizingPolicy::calc_avg_time(double *arr, int size) {
  double sum = 0;

  for (int i = 0; i < size; i++) {
    sum += arr[i];
  }

  return (double) sum / size;
}

// Grow the capacity of H1.
size_t TeraDynamicResizingPolicy::grow_h1() {
  PSOldGen *old_gen = ParallelScavengeHeap::old_gen();
  size_t new_size = 0;
  size_t cur_size = old_gen->capacity_in_bytes();
  size_t ideal_page_cache  = prev_page_cache_size + shrinked_bytes;
  size_t cur_page_cache = read_cgroup_mem_stats(true);

  bool ioslack = (cur_page_cache < ideal_page_cache); 
  shrinked_bytes = 0;
  prev_page_cache_size = 0;

  if (ioslack) {
    if (TeraHeapStatistics) {
      thlog_or_tty->print_cr("Grow_by = %lu\n", (ideal_page_cache - cur_page_cache));
      thlog_or_tty->flush();
    }

    // Reset the metrics for page cache because now we use the ioslack
    return cur_size + (ideal_page_cache - cur_page_cache); 
  }

  new_size = cur_size + (GROW_STEP * read_cgroup_mem_stats(true));

  if (TeraHeapStatistics) {
    thlog_or_tty->print_cr("[GROW_H1] Before = %lu | After = %lu | PageCache = %lu\n",
                           cur_size, new_size, read_cgroup_mem_stats(true));
    thlog_or_tty->flush();
  }

  return new_size;
}

// Shrink the capacity of H1 to increase the page cache size.
size_t TeraDynamicResizingPolicy::shrink_h1() {
  PSOldGen *old_gen = ParallelScavengeHeap::old_gen();
  size_t cur_size = old_gen->capacity_in_bytes();
  size_t used_size = old_gen->used_in_bytes();
  size_t free_space = cur_size - used_size;
  size_t new_size = used_size + (SHRINK_STEP * free_space); ;

  set_shrinked_bytes(cur_size - new_size);
  set_current_size_page_cache();

  if (TeraHeapStatistics) {
    thlog_or_tty->print_cr("[SHRINK_H1] Before = %lu | After = %lu | PageCache = %lu\n",
                           cur_size, new_size, read_cgroup_mem_stats(true));
    thlog_or_tty->flush();
  }

  return new_size;
}
  
// Decrease the size of H2 candidate objects that are in H1 and
// should be moved to H2. We measure only H2 candidates objects that
// are primitive arrays and leaf objects.
void TeraDynamicResizingPolicy::decrease_h2_candidate_size(size_t size) {
  h2_cand_size_in_bytes -= size * HeapWordSize;
  if (h2_cand_size_in_bytes < 0)
    h2_cand_size_in_bytes = 0;

#ifdef LAZY_MOVE_H2
  transfer_hint_enabled = true;
#else
  transfer_hint_enabled = Universe::teraHeap()->is_direct_promote() ? false : true;
#endif
}

// Save the history of the GC and iowait overheads. We maintain two
// ring buffers (one for GC and one for iowait) and update these
// buffers with the new values for GC cost and IO overhead.
void TeraDynamicResizingPolicy::history(int index, double gc_time_ms, double iowait_time_ms) {
#ifdef GC_DISTRIBUTION
  double gc_ratio = calculate_gc_cost(gc_time_ms);
  hist_gc_time[index % GC_HIST_SIZE] = gc_ratio * interval;
  hist_iowait_time[index % HIST_SIZE] = iowait_time_ms;

#else
  hist_gc_time[index % GC_HIST_SIZE] = gc_time_ms;
  hist_iowait_time[index % HIST_SIZE] = iowait_time_ms;
#endif
}
// Calculation of the GC cost prediction.
double TeraDynamicResizingPolicy::calculate_gc_cost(double gc_time_ms) {
  static double last_gc_time_ms  = 0;
  static double last_gc_free_bytes_ratio = 1;
  static double gc_percentage_ratio = 0;

  // Interval between the previous and the current gc
  double gc_interval_ms = (last_full_gc_start - prev_full_gc_end) * 1000;

  // Non major GC cycles happens.
  if (gc_time_ms == 0 && gc_interval_ms == 0)
    return 0;

#ifdef LAZY_MOVE_H2
  bool is_no_action = (cur_action == S_NO_ACTION || cur_action == S_IOSLACK
                       || cur_action == S_CONTINUE || cur_action == S_MOVE_H2
                       || cur_action == S_WAIT_AFTER_GROW);
#else
  bool is_no_action = (cur_action == S_NO_ACTION || cur_action == S_IOSLACK || cur_action == S_CONTINUE);
#endif

  if (gc_time_ms == 0 && is_no_action) {
    return gc_percentage_ratio;
  }

  // Free bytes
  PSOldGen *old_gen = ParallelScavengeHeap::old_gen();
  double cur_live_bytes_ratio = (double) old_gen->used_in_bytes() / old_gen->capacity_in_bytes();
  double cur_free_bytes_ratio = (1 - cur_live_bytes_ratio);
  double cost = 0;

#ifdef LAZY_MOVE_H2
  if (gc_time_ms == 0 && (cur_action == S_SHRINK_H1 || cur_action == S_GROW_H1)) {
    cost = last_gc_time_ms * (1 - cur_free_bytes_ratio);
    gc_percentage_ratio = (cost * last_gc_free_bytes_ratio) / (cur_free_bytes_ratio * gc_interval_ms);

    if (TeraHeapStatistics) {
      thlog_or_tty->print_cr("gc_percentage = %lf\n", gc_percentage_ratio);
    }

    return gc_percentage_ratio;
  }

#else
  if (gc_time_ms == 0) {
    cost = last_gc_time_ms * (1 - cur_free_bytes_ratio);
    gc_percentage_ratio = (cost * last_gc_free_bytes_ratio) / (cur_free_bytes_ratio * gc_interval_ms);

    if (TeraHeapStatistics) {
      thlog_or_tty->print_cr("gc_percentage = %lf\n", gc_percentage_ratio);
    }

    return gc_percentage_ratio;
  }
#endif

  // Calculate the cost of current GC
  last_gc_time_ms = gc_time_ms;
  last_gc_free_bytes_ratio = cur_free_bytes_ratio;
  cost = last_gc_time_ms * (1 - cur_free_bytes_ratio);
  gc_percentage_ratio = (cost * last_gc_free_bytes_ratio) / (cur_free_bytes_ratio * gc_interval_ms);

  if (TeraHeapStatistics) {
    thlog_or_tty->print_cr("gc_percentage = %lf\n", gc_percentage_ratio);
  }

  return gc_percentage_ratio;
}

// Calculation of the GC cost prediction.
//double TeraDynamicResizingPolicy::calculate_gc_cost(double gc_time_ms) {
//  static double last_gc_time_ms  = 0;
//  static double last_gc_free_bytes_ratio = 1;
//
//  // Free bytes
//  PSOldGen *old_gen = ParallelScavengeHeap::old_gen();
//  double cur_live_bytes_ratio = (double) old_gen->used_in_bytes() / old_gen->capacity_in_bytes();
//  double cur_free_bytes_ratio = (1 - cur_live_bytes_ratio);
//
//  // Interval between the previous and the current gc
//  double gc_interval_ms = (last_full_gc_start - prev_full_gc_end) * 1000;
//  double gc_percentage_ratio = 0;
//  double cost = 0;
//
//  if (gc_time_ms == 0) {
//    // Calculate the cost of current GC
//    cost = last_gc_time_ms * (1 - cur_free_bytes_ratio);
//    gc_percentage_ratio = (cost * last_gc_free_bytes_ratio) / (cur_free_bytes_ratio * gc_interval_ms);
//
//    if (TeraHeapStatistics) {
//      thlog_or_tty->print_cr("gc_percentage = %lf\n", gc_percentage_ratio);
//    }
//
//    return gc_percentage_ratio;
//  }
//
//  last_gc_time_ms = gc_time_ms;
//  last_gc_free_bytes_ratio = cur_free_bytes_ratio;
//  cost = last_gc_time_ms * (1 - cur_free_bytes_ratio);
//  gc_percentage_ratio = (cost * last_gc_free_bytes_ratio) / (cur_free_bytes_ratio * gc_interval_ms);
//
//  if (TeraHeapStatistics) {
//    thlog_or_tty->print_cr("gc_percentage = %lf\n", gc_percentage_ratio);
//  }
//
//  return gc_percentage_ratio;
//}
  
// Read the memory statistics for the cgroup
// TODO: Make the path dynamic (after the submission of the paper)
size_t TeraDynamicResizingPolicy::read_cgroup_mem_stats(bool read_page_cache) {
  // Define the path to the memory.stat file
  const char* file_path = "/sys/fs/cgroup/memory/memlim/memory.stat";

  // Open the file for reading
  FILE* file = fopen(file_path, "r");

  if (file == NULL) {
    fprintf(stderr, "Failed to open memory.stat\n");
    return 0;
  }

  char line[BUFFER_SIZE];
  size_t res = 0;

  // Read each line in the file
  while (fgets(line, sizeof(line), file)) {
    if (read_page_cache && strncmp(line, "cache", 5) == 0) {
      // Extract the cache value
      res = atoll(line + 6);
      break;
    }

    if (strncmp(line, "rss", 3) == 0) {
      // Extract the rss value
      res = atoll(line + 4);
      break;
    }
  }

  // Close the file
  fclose(file);
  return res;
}

// Before we perform a shrink operation we check if there is an IO
// slack. If there is IO slack then we abort the extra shrinking
// operation.
TeraDynamicResizingPolicy::state TeraDynamicResizingPolicy::state_shrink_h1() {
  size_t cur_rss = 0;
  size_t cur_cache = 0;

  cur_rss = read_cgroup_mem_stats(false);
  cur_cache = read_cgroup_mem_stats(true);
  bool ioslack = ((cur_rss + cur_cache) < (59055800320 * 0.97));

  if (ioslack) {
    cur_action = S_IOSLACK;
    return S_IOSLACK;
  }

  cur_action = S_SHRINK_H1;
  return S_SHRINK_H1;
}

#ifdef WAIT_AFTER_GROW
// After each growing operation of H1 we wait to see the effect of
// the action. If we reach a gc or the io cost is higher than gc
// cost then we go to no action state. 
TeraDynamicResizingPolicy::state TeraDynamicResizingPolicy::should_wait_after_grow(double io_time_ms,                                                                                                                
                                                       double gc_time_ms) {

  if (cur_action != S_WAIT_AFTER_GROW)
    return S_CONTINUE;

  PSOldGen *old_gen = ParallelScavengeHeap::old_gen();
  size_t cur_size = old_gen->capacity_in_bytes();
  size_t used_size = old_gen->used_in_bytes();

  // Occupancy of the old generation is higher than 85%
  bool high_occupancy = (((double)(used_size) / cur_size) > 0.70);

  if (high_occupancy) {
    return S_NO_ACTION; // remember to change that for optimization in S_GROW_H1
  }

  if (abs(io_time_ms - gc_time_ms) <= 100) {
    return S_NO_ACTION;
  }

  if (gc_time_ms > io_time_ms) {
    return S_NO_ACTION; // remmber to change that for optimization in S_SHRINK_H1;
  }

  return S_NO_ACTION;
}                 
#endif

// Print states (for debugging and logging purposes)
void TeraDynamicResizingPolicy::print_state(enum state cur_state) {
  if (!TeraHeapStatistics)
    return;

  thlog_or_tty->stamp(true);
  thlog_or_tty->print_cr("STATE = %s\n", state_name[cur_state]);
  thlog_or_tty->flush();
}

// Initialize the array of state names
void TeraDynamicResizingPolicy::init_state_names() {
  strncpy(state_name[0], "S_NO_ACTION",       12);
  strncpy(state_name[1], "S_SHRINK_H1",       12);
  strncpy(state_name[2], "S_GROW_H1",         10);
  strncpy(state_name[3], "S_MOVE_BACK",       12);
  strncpy(state_name[4], "S_CONTINUE",        11);
  strncpy(state_name[5], "S_MOVE_H2",         10);
  strncpy(state_name[6], "S_IOSLACK",         10);
  strncpy(state_name[7], "S_WAIT_AFTER_GROW", 18);
}
  
// Check if the old gen is at the highest size
bool TeraDynamicResizingPolicy::is_old_gen_max_capacity() {
  PSOldGen *old_gen = ParallelScavengeHeap::old_gen();
  bool under_h1_max_limit = old_gen->capacity_in_bytes() < old_gen->max_gen_size();
  return under_h1_max_limit;
}
