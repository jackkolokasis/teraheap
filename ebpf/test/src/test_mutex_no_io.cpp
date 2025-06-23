#include <pthread.h>
#include <unistd.h>
#include <sched.h>
#include <sys/syscall.h>
#include <cstring>
#include <iostream>
#include <mutex>
#include "ioWait.hpp"

const int NUM_ITERATIONS = 50;  // More iterations
const int SLEEP_IN_MUTEX_MS = 300;

std::mutex shared_mutex;

int gettid() {
  return syscall(SYS_gettid);
}

void heavy_computation() {
  volatile double x = 1.0;
  for (int i = 0; i < 1e7; ++i) {
    x *= 1.0000001;
  }
}

void* worker(void* arg) {
  int core_id = *reinterpret_cast<int*>(arg);

  // Pin thread to specific core
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core_id, &cpuset);
  if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0) {
    perror("pthread_setaffinity_np");
  }

  int tid = gettid();
  ebpf_add_tid(tid);

  for (int i = 0; i < NUM_ITERATIONS; ++i) {
    {
      std::unique_lock<std::mutex> lock(shared_mutex);
      usleep(SLEEP_IN_MUTEX_MS * 1000);  // Spend 300ms inside mutex
    }

    // Heavy computation outside mutex
    heavy_computation();
  }

  return nullptr;
}

int main() {
  ebpf_start();
  ebpf_enable_tracking();

  pthread_t threads[2];
  int cores[2] = {0, 1};

  for (int i = 0; i < 2; ++i) {
    if (pthread_create(&threads[i], nullptr, worker, &cores[i]) != 0) {
      perror("pthread_create");
      return 1;
    }
  }

  for (int i = 0; i < 2; ++i) {
    pthread_join(threads[i], nullptr);
  }

  double iowait_time_ms = ebpf_disable_tracking();
  fprintf(stderr, "[EBPF] IO-wait time (ms): %.2f\n", iowait_time_ms);

  ebpf_stop();
  return 0;
}
