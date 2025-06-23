#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>

#define TASK_UNINTERRUPTIBLE 0x00000002  // manually define D-state flag

char LICENSE[] SEC("license") = "GPL";

// Per-thread I/O wait tracking
struct {
  __uint(type, BPF_MAP_TYPE_HASH);
  __type(key, u32);   // tid
  __type(value, u64); // start timestamp
  __uint(max_entries, 1024);
} start SEC(".maps");

struct {
  __uint(type, BPF_MAP_TYPE_HASH);
  __type(key, u32);   // tid
  __type(value, u64); // total time
  __uint(max_entries, 1024);
} iowait SEC(".maps");

// Tracked mutator threads
struct {
  __uint(type, BPF_MAP_TYPE_HASH);
  __type(key, u32);   // tid
  __type(value, u8);  // bool
  __uint(max_entries, 1024);
} mutators SEC(".maps");

// Global flag to enable/disable tracking
struct {
  __uint(type, BPF_MAP_TYPE_ARRAY);
  __type(key, u32);
  __type(value, u32);
  __uint(max_entries, 1);
} enabled SEC(".maps");

SEC("tracepoint/sched/sched_switch")
int handle_sched_switch(struct trace_event_raw_sched_switch *ctx) {
  u32 prev_tid = ctx->prev_pid;
  u32 next_tid = ctx->next_pid;
  u64 ts = bpf_ktime_get_ns();
  u32 zero = 0;

  // Check if tracking is globally enabled
  u32 *flag = bpf_map_lookup_elem(&enabled, &zero);
  if (!flag || *flag == 0)
    return 0;

  // Track I/O wait start if prev_tid is a mutator and enters D state
  u8 *tracked_prev = bpf_map_lookup_elem(&mutators, &prev_tid);
  if (tracked_prev && ctx->prev_state == TASK_UNINTERRUPTIBLE) {
    bpf_map_update_elem(&start, &prev_tid, &ts, BPF_ANY);
  }

  // Track I/O wait end if next_tid was previously in D state
  u8 *tracked_next = bpf_map_lookup_elem(&mutators, &next_tid);
  u64 *start_ts = bpf_map_lookup_elem(&start, &next_tid);
  if (tracked_next && start_ts) {
    u64 delta = ts - *start_ts;
    u64 *total = bpf_map_lookup_elem(&iowait, &next_tid);
    if (total) {
      *total += delta;
    } else {
      bpf_map_update_elem(&iowait, &next_tid, &delta, BPF_ANY);
    }
    bpf_map_delete_elem(&start, &next_tid);
  }

  return 0;
}
