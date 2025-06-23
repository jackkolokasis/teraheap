#include <fcntl.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <chrono>
#include <cstring>
#include <iostream>
#include <vector>
#include <atomic>
#include <thread>
#include <sched.h>
#include "ioWait.hpp"

#define USE_EBPF 0

const char* device_path = "/dev/nullb0";
const size_t BLOCK_SIZE = 4096;
const int NUM_THREADS = 2;
const int NUM_WRITES_PER_THREAD = 50;

std::atomic<int> ready_count(0);

int gettid() {
  return syscall(SYS_gettid);
}

void pin_to_core(int core_id) {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core_id, &cpuset);
  pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
}

void* io_worker(void* arg) {
  int id = *(int*)arg;
  int core = id;  // Use core 0 and 1 (modify as needed)
  pin_to_core(core);

  char buf[BLOCK_SIZE] = {0};

#if USE_EBPF
  int tid = gettid();
  ebpf_add_tid(tid);
#endif

  int fd = open(device_path, O_WRONLY | O_SYNC);
  if (fd < 0) {
    perror("open");
    pthread_exit(nullptr);
  }

  ready_count.fetch_add(1);

  while (ready_count.load() < NUM_THREADS);  // synchronize start

  for (int i = 0; i < NUM_WRITES_PER_THREAD; ++i) {
    ssize_t ret = write(fd, buf, BLOCK_SIZE);
    if (ret != BLOCK_SIZE) {
      perror("write");
      break;
    }

    // Optional: simulate some CPU work
    for (volatile int spin = 0; spin < 100000; ++spin);
  }

  close(fd);
  pthread_exit(nullptr);
}

int main() {
#if USE_EBPF
  ebpf_start();
  ebpf_enable_tracking();
#endif

  auto start = std::chrono::high_resolution_clock::now();

  std::vector<pthread_t> threads(NUM_THREADS);
  std::vector<int> ids(NUM_THREADS);

  for (int i = 0; i < NUM_THREADS; ++i) {
    ids[i] = i;
    pthread_create(&threads[i], nullptr, io_worker, &ids[i]);
  }

  for (auto& th : threads)
    pthread_join(th, nullptr);

  auto end = std::chrono::high_resolution_clock::now();
  double duration_ms = std::chrono::duration<double, std::milli>(end - start).count();

#if USE_EBPF
  double iowait_ms = ebpf_disable_tracking();
  ebpf_stop();
  std::cerr << "✅ eBPF I/O Wait Time (total): " << iowait_ms << " ms" << std::endl;
#endif

  std::cerr << "⏱️ Total Execution Time: " << duration_ms << " ms" << std::endl;
  return 0;
}
