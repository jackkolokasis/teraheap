#ifndef IOWAIT_HPP
#define IOWAIT_HPP

#ifdef __cplusplus
extern "C" {
#endif

// Load the BPF object and attach the tracepoint
void ebpf_start();

// Stop tracing and unload the BPF object
void ebpf_stop();

// Add a TID to be tracked (insert into the 'mutators' map)
void ebpf_add_tid(int tid);

// Enable D-state profiling (sets enabled[0] = 1)
void ebpf_enable_tracking();

// Disable D-state profiling (sets enabled[0] = 0)
double ebpf_disable_tracking();

#ifdef __cplusplus
}
#endif

#endif // IOWAIT_HPP
