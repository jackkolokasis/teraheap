#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <random>
#include "ioWait.hpp"

const char* device_path = "/dev/nullb0";
const size_t FILE_SIZE = 10L * 1024 * 1024 * 1024; // 10 GB
const int NUM_READS = 100000;

int gettid() {
  return syscall(SYS_gettid);
}

void* read_worker(void* arg) {
  int core_id = *reinterpret_cast<int*>(arg);

  // Pin thread
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core_id, &cpuset);
  pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);

  int tid = gettid();
  ebpf_add_tid(tid);

  int fd = open(device_path, O_RDWR | O_DIRECT);
  if (fd < 0) {
    perror("open");
    return nullptr;
  }

  void* map = mmap(nullptr, FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (map == MAP_FAILED) {
    perror("mmap");
    close(fd);
    return nullptr;
  }

  std::mt19937 rng(time(NULL) ^ tid);
  std::uniform_int_distribution<size_t> offset_dist(0, FILE_SIZE - 4096);

  volatile char sink = 0;
  char* mem = static_cast<char*>(map);

  for (int i = 0; i < NUM_READS; ++i) {
    size_t offset = offset_dist(rng);
    sink ^= mem[offset]; // trigger read
  }

  munmap(map, FILE_SIZE);
  close(fd);
  return nullptr;
}

int main() {
  ebpf_start();
  ebpf_enable_tracking();

  pthread_t threads[2];
  int cores[2] = {0, 1};

  for (int i = 0; i < 2; ++i) {
    if (pthread_create(&threads[i], nullptr, read_worker, &cores[i]) != 0) {
      perror("pthread_create");
      return 1;
    }
  }

  for (int i = 0; i < 2; ++i) {
    pthread_join(threads[i], nullptr);
  }

  double iowait_time_ms = ebpf_disable_tracking();
  fprintf(stderr, "[RESULT] IO wait time = %.2lf ms\n", iowait_time_ms);
  ebpf_stop();

  return 0;
}
