#include "gc_interface/collectedHeap.hpp"
#include "gc_implementation/parallelScavenge/parallelScavengeHeap.hpp"
#include "gc_implementation/teraHeap/teraDynamicResizingPolicy.hpp"
#include "memory/universe.hpp"

#define BUFFER_SIZE 1024
#define CYCLES_PER_SECOND 2.4e9; // CPU frequency of 2.4 GHz
//#define WINDOW_INTERVAL ((30LL * 1000))
#define TRANSFER_THRESHOLD 0.4f
#define GC_FREQUENCY (25)
  
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
  window_interval = (prev_action == S_MOVE_H2) ? 5000 : 30000;
  //window_interval = 30000;
  //gc_mjr_faults = 0;
  //mjr_faults = 0;
  //mjr_faults_start = get_mjr_faults();
}

// Check if the window limit exceed time
bool TeraDynamicResizingPolicy::is_window_limit_exeed() {
  uint64_t window_end_time;
  window_end_time = rdtsc();
  interval = ellapsed_time(window_start_time, window_end_time);

  return (interval >= window_interval) ? true : false;
}

// Init the iowait timer at the begining of the major GC.
void TeraDynamicResizingPolicy::gc_start() {
  read_cpu_stats(&gc_iowait_start, &gc_cpu_start);
  gc_dev_start = get_device_active_time("nvme1n1");
  //gc_mjr_faults_start = get_mjr_faults();
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

  //gc_mjr_faults += get_mjr_faults() - gc_mjr_faults_start;
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

// According to the usage of the old generation and the io wait time
// we perform an action. This action triggers to grow H1 or shrink
// H1.
TeraDynamicResizingPolicy::state TeraDynamicResizingPolicy::action() {
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

  double avg_iowait_time = calc_avg_time(hist_iowait_time);
  double avg_gc_time = calc_avg_time(hist_gc_time);
  
  if (TeraHeapStatistics)
    debug_print(avg_iowait_time, avg_gc_time, interval, iowait_time_ms, gc_time);

  if (avg_gc_time + avg_iowait_time < (0.1 * interval)) {
    //if ((os::elapsedTime() -  last_full_gc_ms >= GC_FREQUENCY)) {
    //  prev_action = S_SHRINK_H1;
    //  return S_SHRINK_H1;
    //}

    prev_action = S_NO_ACTION;
    return S_NO_ACTION;
  }

  // We calculate the ratio of the h2 candidate objects in H1 
  double h2_candidate_ratio = (double) h2_cand_size_in_bytes / Universe::heap()->capacity();

  if (avg_gc_time >= avg_iowait_time &&  h2_candidate_ratio >= TRANSFER_THRESHOLD) {
    prev_action = S_MOVE_H2;
    return S_MOVE_H2;
  }
  
  if (avg_gc_time >= avg_iowait_time && h2_candidate_ratio < TRANSFER_THRESHOLD
    && ParallelScavengeHeap::old_gen()->capacity_in_bytes() < ParallelScavengeHeap::old_gen()->max_gen_size()) {
    prev_action = S_GROW_H1;
    return S_GROW_H1;
  }

  if (avg_iowait_time > avg_gc_time && device_active_time_ms > 0 ) {
    if (prev_action == S_SHRINK_H1) {
      size_t diff = get_buffer_cache_space() - prev_page_cache_size;

      if (shrinked_bytes > diff) {
        if (TeraHeapStatistics) {
          thlog_or_tty->print_cr("shrink_with_no_action diff=%lu\n", diff);
          thlog_or_tty->flush();
        }
        return S_NO_ACTION;
      }
    }
    last_shrink_action = os::elapsedTime();
    prev_action = S_SHRINK_H1;
    return S_SHRINK_H1;
  }

  prev_action = S_NO_ACTION;
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

// Calculate free bytes for old generation for promotion
size_t TeraDynamicResizingPolicy::calculate_old_gen_free_bytes(size_t max_free_bytes, size_t used_bytes) {
  //double ratio = (double) max_free_bytes / (used_bytes + max_free_bytes);
  //unsigned long desired_page_cache_bytes = 2 * mjr_faults * 4096LU;
  unsigned long desired_page_cache_bytes = 0.5 * max_free_bytes;

  //if (prev_action == S_SHRINK_H1)
  //  desired_page_cache_bytes *= 2;

  if (TeraHeapStatistics) {
    //thlog_or_tty->print_cr("mjr_faults = %lu\n", mjr_faults);
    thlog_or_tty->print_cr("desired_page_cache_bytes = %lu\n", desired_page_cache_bytes);
    thlog_or_tty->flush();
  }

  return (desired_page_cache_bytes < max_free_bytes) ? 
    (max_free_bytes - desired_page_cache_bytes) : ((size_t) 0.4 * max_free_bytes);
}

//unsigned long TeraDynamicResizingPolicy::get_mjr_faults() {
//  char proc_stat_file[50];
//  snprintf(proc_stat_file, sizeof(proc_stat_file), "/proc/%d/stat", getpid());
//
//  FILE* stat_file = fopen(proc_stat_file, "r");
//  if (!stat_file) {
//    fprintf(stderr, "Error: Unable to open the stat file for process %d\n", getpid());
//  }
//
//  char line[1024];
//  fgets(line, sizeof(line), stat_file);
//
//  fclose(stat_file);
//
//  unsigned long mjr_faults = 0;
//  if (line[0] != '\0') {
//    char* token = strtok(line, " ");
//    int field = 1;
//    while (token != NULL) {
//      // Field 12 majflt  %lu The number of major faults the
//      // process has made which have required loading a memory page
//      // from disk.
//      if (field == 12) {
//        mjr_faults = atol(token);
//        break;
//      }
//      token = strtok(NULL, " ");
//      field++;
//    }
//  }
//
//  return mjr_faults;
//}
  
// Get the total size of the buffer and spage cache
size_t TeraDynamicResizingPolicy::get_buffer_cache_space() {
  FILE *mem_info = fopen("/proc/meminfo", "r");

  if (!mem_info) {
    printf("Error: Cannot open /proc/meminfo file.\n");
    return 0;
  }

  char line[BUFFER_SIZE];
  size_t cache_size_kb = 0;
  size_t buffer_size_kb = 0;

  while (fgets(line, sizeof(line), mem_info) != NULL) {
    if (strncmp(line, "Buffers:", 8) == 0) {
      // Extract the value after "Buffers:" and convert it to an size_t
      sscanf(line + 8, "%lu", &buffer_size_kb);
    }

    if (strncmp(line, "Cached:", 7) == 0) {
      // Extract the value after "Cached:" and convert it to an integer
      sscanf(line + 7, "%lu", &cache_size_kb);
      break; // No need to continue searching after finding the cache size
    }
  }

  if (fclose(mem_info)) {
    fprintf(stderr, "Error closing file");
  }

  return (cache_size_kb + buffer_size_kb) * 1024;

  //size_t desired_free_space = 0.5 * ((cache_size_kb + buffer_size_kb) * 1024);

  //if (TeraHeapStatistics) {
  //  thlog_or_tty->print_cr("Grow_by = %lu\n", desired_free_space);
  //  thlog_or_tty->flush();
  //}

  //return desired_free_space;
}
  
// Find the average of the array elements
double TeraDynamicResizingPolicy::calc_avg_time(double *arr) {
  double sum = 0;

  for (int i = 0; i < HIST_SIZE; i++) {
    sum += arr[i];
  }

  return sum / HIST_SIZE;
}

bool TeraDynamicResizingPolicy::should_grow_h1_after_shrink() {
  if (prev_action == S_SHRINK_H1 && (os::elapsedTime() - last_shrink_action) < GC_FREQUENCY) {
    return true;
  }
  return false;
}
