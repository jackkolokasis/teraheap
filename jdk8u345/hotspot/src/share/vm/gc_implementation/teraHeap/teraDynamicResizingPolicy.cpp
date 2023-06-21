#include "gc_interface/collectedHeap.hpp"
#include "gc_implementation/parallelScavenge/parallelScavengeHeap.hpp"
#include "gc_implementation/teraHeap/teraDynamicResizingPolicy.hpp"
#include "memory/universe.hpp"

#define BUFFER_SIZE 1024
#define CYCLES_PER_SECOND 2.4e9; // CPU frequency of 2.4 GHz
#define WINDOW_INTERVAL ((30LL * 1000))
#define TRANSFER_THRESHOLD 0.4f
#define GC_FREQUENCY (120)
  
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
}

// Check if the window limit exceed time
bool TeraDynamicResizingPolicy::is_window_limit_exeed() {
  uint64_t window_end_time;
  window_end_time = rdtsc();
  interval = ellapsed_time(window_start_time, window_end_time);

  return (interval >= WINDOW_INTERVAL) ? true : false;
}

// Init the iowait timer at the begining of the major GC.
void TeraDynamicResizingPolicy::gc_start() {
  read_cpu_stats(&gc_iowait_start, &gc_cpu_start);
  gc_dev_start = get_device_active_time("nvme1n1");
}

// Count the iowait time during gc and update the gc_iowait_time_ms
// counter for the current window
void TeraDynamicResizingPolicy::gc_end(double gc_duration, double last_full_gc) {
  unsigned long long gc_iowait_end = 0, gc_cpu_end = 0, gc_blk_io_end = 0;
  uint64_t gc_dev_end = 0;
  double iowait_time = 0;

  read_cpu_stats(&gc_iowait_end, &gc_cpu_end);
  calc_iowait_time(gc_iowait_start, gc_iowait_end, gc_cpu_start, gc_cpu_end,
                   gc_duration, &iowait_time);
  gc_iowait_time_ms += iowait_time;

  gc_dev_end = get_device_active_time("nvme1n1");
  gc_dev_time += (gc_dev_end - gc_dev_start);

  gc_time += gc_duration;
  last_full_gc_ms = last_full_gc;
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

      *total_cpu = user + nice + system + iowait + irq + softirq;
      *cpu_iowait = iowait;

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

// According to the usage of the old generation and the io wait time
// we perform an action. This action triggers to grow H1 or shrink
// H1.
TeraDynamicResizingPolicy::state TeraDynamicResizingPolicy::action() {
  double iowait_time_ms = 0;
  uint64_t device_active_time_ms = 0, dev_time_end = 0;
  unsigned long long iowait_end = 0, cpu_end = 0;

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

  // This is for debugging
  // debug_print(iowait_time_ms, device_active_time_ms, interval);

  if (gc_time + iowait_time_ms < (0.1 * interval)) {
    if (os::elapsedTime() -  last_full_gc_ms >= GC_FREQUENCY) {
      prev_action = S_SHRINK_H1;
      return S_SHRINK_H1;
    }

    prev_action = S_NO_ACTION;
    return S_NO_ACTION;
  }

  // We calculate the ratio of the h2 candidate objects in H1 
  double h2_candidate_ratio = (double) h2_cand_size_in_bytes / Universe::heap()->capacity();

  if (gc_time >= iowait_time_ms &&  h2_candidate_ratio >= TRANSFER_THRESHOLD) {
    prev_action = S_MOVE_H2;
    return S_MOVE_H2;
  }
  
  if (gc_time >= iowait_time_ms && h2_candidate_ratio < TRANSFER_THRESHOLD
      && ParallelScavengeHeap::old_gen()->capacity_in_bytes() < ParallelScavengeHeap::old_gen()->max_gen_size()) {
    prev_action = S_GROW_H1;
    return S_GROW_H1;
  }

  if (iowait_time_ms > gc_time && device_active_time_ms > 0) {
    prev_action = S_SHRINK_H1;
    return S_SHRINK_H1;
  }

  if (os::elapsedTime() - last_full_gc_ms >= GC_FREQUENCY) {
    prev_action = S_SHRINK_H1;
    return S_SHRINK_H1;
  }

  return S_NO_ACTION;
}

// Print counters for debugging purposes
void TeraDynamicResizingPolicy::debug_print(double iowait_time_ms,
                                            uint64_t device_active_time_ms,
                                            double interval) {
  fprintf(stderr, "-----------------------------\n");
  fprintf(stderr, "iowait_time_ms = %lf\n", iowait_time_ms);
  fprintf(stderr, "gc_iowait_time_ms = %lf\n", gc_iowait_time_ms);
  fprintf(stderr, "dev_time_ms = %lu\n", device_active_time_ms);
  fprintf(stderr, "gc_dev_time_ms = %lu\n", gc_dev_time);
  fprintf(stderr, "interval = %lf\n", interval);
  fprintf(stderr, "gc_time_ms = %lf\n", gc_time);
  fprintf(stderr, "h1_capacity(GB) = %lf\n", (double) Universe::heap()->capacity()/1024/1024/1024.0);
  fprintf(stderr, "h1_max_capacity(GB) = %lf\n", (double) Universe::heap()->max_capacity()/1024/1024/1024.0);
  fprintf(stderr, "h1_used(GB) = %lf\n", (double) Universe::heap()->used()/1024/1024/1024.0);
  fprintf(stderr, "h1_used_at_last_gc(GB) = %lf\n", (double) Universe::get_heap_used_at_last_gc()/1024/1024/1024.0);
  fprintf(stderr, "h1_free_at_last_gc(GB) = %lf\n", (double) Universe::get_heap_free_at_last_gc()/1024/1024/1024.0);
  fprintf(stderr, "old_gen_used_in_bytes = %lu\n", ParallelScavengeHeap::old_gen()->used_in_bytes());
  fprintf(stderr, "old_gen_capacity_in_bytes = %lu\n", ParallelScavengeHeap::old_gen()->capacity_in_bytes());
  fprintf(stderr, "old_gen_min_gen_size = %lu\n", ParallelScavengeHeap::old_gen()->min_gen_size());
  fprintf(stderr, "old_gen_max_gen_size = %lu\n", ParallelScavengeHeap::old_gen()->max_gen_size());
  fprintf(stderr, "-----------------------------\n");
}
