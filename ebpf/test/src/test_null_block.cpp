#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <sys/syscall.h>  // for SYS_gettid
#include <linux/unistd.h>
#include "ioWait.hpp"

const char* device_path = "/dev/nullb0";
const size_t BLOCK_SIZE = 4096;
const int NUM_WRITES = 20;  // 20 * 500ms = ~10 seconds expected delay

int gettid() {
  return syscall(SYS_gettid);
}

int main() {
  char buf[BLOCK_SIZE] = {0};

  ebpf_start();

  int tid = gettid();
  ebpf_add_tid(tid);
  ebpf_enable_tracking();

  int fd = open(device_path, O_WRONLY | O_SYNC);
  if (fd < 0) {
    perror("open");
    return 1;
  }

  std::cerr << "[TEST] Performing " << NUM_WRITES << " I/O writes to " << device_path << std::endl;

  for (int i = 0; i < NUM_WRITES; ++i) {
    ssize_t ret = write(fd, buf, BLOCK_SIZE);
    if (ret != BLOCK_SIZE) {
      perror("write");
      break;
    }
  }

  close(fd);

  double iowait_time_ms = ebpf_disable_tracking();
  fprintf(stderr, "IO wait time ms = %lf\n", iowait_time_ms);
  ebpf_stop();

  return 0;
}
