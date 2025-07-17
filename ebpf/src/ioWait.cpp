#include "../include/ioWait.hpp"
#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include <linux/bpf.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

static int mutators_fd = -1;
static int enabled_fd = -1;
static int iowait_fd = -1;

void ebpf_start() {
  mutators_fd = bpf_obj_get("/sys/fs/bpf/mutators");
  if (mutators_fd < 0) {
    fprintf(stderr, "Failed to open pinned map 'mutators'\n");
  }

  enabled_fd = bpf_obj_get("/sys/fs/bpf/enabled");
  if (enabled_fd < 0) {
    fprintf(stderr, "Failed to open pinned map 'enabled'\n");
  }
  
  iowait_fd = bpf_obj_get("/sys/fs/bpf/iowait");
  if (iowait_fd < 0) {
    fprintf(stderr, "Failed to open pinned map 'iowait'\n");
  }

  if (mutators_fd >= 0 && enabled_fd >= 0 && iowait_fd >= 0) {
    fprintf(stderr, "eBPF pinned maps opened successfully\n");
  }
}

void ebpf_add_tid(int tid) {
  if (mutators_fd < 0)
    return;

  uint8_t one = 1;
  if (bpf_map_update_elem(mutators_fd, &tid, &one, BPF_ANY) != 0) {
    fprintf(stderr, "Failed to add TID %d to 'mutators' map\n", tid);
  }
}

void ebpf_enable_tracking() {
  if (enabled_fd < 0) 
    return;
  
  uint32_t key = 0;
  uint32_t value = 1;
  if (bpf_map_update_elem(enabled_fd, &key, &value, BPF_ANY) != 0) {
    fprintf(stderr, "Failed to enable tracking\n");
  } 
}

double ebpf_disable_tracking() {
  if (enabled_fd < 0 || iowait_fd < 0)
    return -1.0;

  // Disable tracking
  uint32_t key = 0;
  uint32_t value = 0;
  if (bpf_map_update_elem(enabled_fd, &key, &value, BPF_ANY) != 0) {
    fprintf(stderr, "Failed to disable tracking\n");
    return -1.0;
  }

  // Iterate over all entries in iowait map
  uint32_t lookup_key = 0;
  uint32_t next_key;
  uint64_t wait_value;
  uint64_t total_wait_ns = 0;

  while (bpf_map_get_next_key(iowait_fd, &lookup_key, &next_key) == 0) {
    if (bpf_map_lookup_elem(iowait_fd, &next_key, &wait_value) == 0) {
      total_wait_ns += wait_value;
      bpf_map_delete_elem(iowait_fd, &next_key);
    }
    lookup_key = next_key;
  }

  // Report total wait time (convert to milliseconds)
  double total_wait_ms = total_wait_ns / 1e6;
  return total_wait_ms;
}

// Helper to clear all entries in a BPF hash map
void clear_bpf_hash_map(int fd) {
  uint32_t key = 0, next_key = 0;
  while (bpf_map_get_next_key(fd, &key, &next_key) == 0) {
    bpf_map_delete_elem(fd, &next_key);
    key = next_key;
  }
}

void ebpf_stop() {
  fprintf(stderr, "Clear and close EBPF\n");
  // Clear and close mutators map
  if (mutators_fd >= 0) {
    clear_bpf_hash_map(mutators_fd);
    close(mutators_fd);
    mutators_fd = -1;
  }

  // Reset and close enabled map (array)
  if (enabled_fd >= 0) {
    uint32_t key = 0;
    uint32_t value = 0;
    bpf_map_update_elem(enabled_fd, &key, &value, BPF_ANY);
    close(enabled_fd);
    enabled_fd = -1;
  }

  // Clear and close iowait map
  if (iowait_fd >= 0) {
    clear_bpf_hash_map(iowait_fd);
    close(iowait_fd);
    iowait_fd = -1;
  }
}
