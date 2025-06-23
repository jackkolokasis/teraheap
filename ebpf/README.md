# eBPF Profiler Library

This project provides a shared library (`libebpf_profiler.so`) for
tracking the time JVM mutator threads spend in the uninterruptible `D`
state (usually due to I/O wait). It works by interacting with a
preloaded eBPF program and pinned BPF maps.

---

## 📦 Features

- Dynamically enables and disables eBPF-based tracking from inside the JVM.
- Tracks time mutator threads spend in `D` state.
- Operates without root once eBPF is preloaded and pinned.
- Shared library built in C++, usable from native code or via JNI.

---

## 📁 Directory Structure

```
.
├── include/              # Header file: ioWait.hpp
├── src/                  # Source file: ioWait.cpp
├── ebpf_tracing/         # eBPF program source and vmlinux.h
├── loader/               # One-time setup loader: load/unload pinned eBPF
├── build/                # Build artifacts (.o files)
├── lib/                  # Output .so file
├── Makefile              # Main build script
└── README.md             # This file
```

---

## ⚙️ Prerequisites

- **Ubuntu 22.04+**
- **Kernel 5.8+** with `CONFIG_BPF_SYSCALL=y` and `/sys/kernel/btf/vmlinux` available
- `clang`, `llvm`, `make`, `bpftool`, `libbpf-dev`

```bash
sudo apt install clang llvm make libbpf-dev libelf-dev bpftool
```

---

## 🛠️ Build Instructions

```bash
make
```

This will:
- Generate `vmlinux.h` (if not present)
- Compile `sched_switch.bpf.c` to eBPF object
- Build `lib/libebpf_profiler.so`
- Build the loader tool: `loader/load_bpf_setup`

---

## 🚀 Loading the eBPF Program (Once per Boot)

Because BPF requires elevated privileges to load programs, do the following **once**:

```bash
sudo make load
```

This will:
- Load the BPF program
- Pin the maps (`/sys/fs/bpf/enabled`, `/sys/fs/bpf/mutators`, `/sys/fs/bpf/iowait`)
- Attach to the tracepoint `sched:sched_switch`

To unload the pinned program and maps:

```bash
sudo make unload
```

---

## 🔧 Using the Shared Library

You can call the following functions from native code:

```cpp
void ebpf_enable_tracking();       // Start tracking
void ebpf_disable_tracking();      // Stop tracking and print total wait time
void ebpf_add_tid(int tid);        // Add a mutator thread ID to monitor
```

**Example:**

```cpp
ebpf_add_tid(tid);
ebpf_enable_tracking();
// ... run mutators ...
ebpf_disable_tracking();
```

This will print total time in I/O wait (D state) for all tracked threads during the interval.

---

## ❌ Known Limitations

- Requires root to initially load the BPF program.
- Cannot load BPF programs from within the JVM without capabilities.
- Only threads added with `ebpf_add_tid()` are tracked.
---
