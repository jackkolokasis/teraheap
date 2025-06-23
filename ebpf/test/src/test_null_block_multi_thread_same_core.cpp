#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <cstring>
#include <iostream>
#include <sched.h>
#include <sys/syscall.h>
#include "ioWait.hpp"

const char* device_path = "/dev/nullb0";
const size_t BLOCK_SIZE = 4096;
const int NUM_WRITES = 20;
const int CORE_ID = 0; // Shared core

int gettid() {
  return syscall(SYS_gettid);
}

void* io_worker(void* arg) {
  // Pin both threads to the same core
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(CORE_ID, &cpuset);
  if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0) {
    perror("pthread_setaffinity_np");
  }

  int tid = gettid();
  ebpf_add_tid(tid);

  int fd = open(device_path, O_WRONLY | O_SYNC);
  if (fd < 0) {
    perror("open");
    return nullptr;
  }

  char buf[BLOCK_SIZE] = {0};
  for (int i = 0; i < NUM_WRITES; ++i) {
    ssize_t ret = write(fd, buf, BLOCK_SIZE);
    if (ret != BLOCK_SIZE) {
      perror("write");
      break;
    }
  }

  close(fd);
  return nullptr;
}

int main() {
  ebpf_start();
  ebpf_enable_tracking();

  pthread_t threads[2];

  for (int i = 0; i < 2; ++i) {
    if (pthread_create(&threads[i], nullptr, io_worker, nullptr) != 0) {
      perror("pthread_create");
      return 1;
    }
  }

  for (int i = 0; i < 2; ++i) {
    pthread_join(threads[i], nullptr);
  }

  double iowait_time_ms = ebpf_disable_tracking();
  fprintf(stderr, "IO wait time ms = %lf\n", iowait_time_ms);
  ebpf_stop();

  return 0;
}
