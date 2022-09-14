#include "../include/futex.h"
#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>

#define FUTEX_FN(futex, op, val) \
  syscall(SYS_futex, futex, op, val, NULL, NULL, 0)

void futex_lock(futex_t *futex) {
  // Check if the futex is not available, forcing the process to wait
  while (!__sync_val_compare_and_swap(&futex->word, 1, 0)) {
    __sync_add_and_fetch(&futex->wcnt, 1);

    FUTEX_FN(&futex->word, FUTEX_WAIT, 0);

    __sync_sub_and_fetch(&futex->wcnt, 1);
  }
}

void futex_unlock(futex_t *futex) {
  // Important: The counter is required to optimize the performance
  //            of the unlock function (i.e., "FUTEX_WAKE" requires
  //            switching to the kernel from user-space).

  // If the futex was taken, try to wake-up one of the waiting
  // processes
  if (!__sync_val_compare_and_swap(&futex->word, 0, 1) && futex->wcnt > 0)
    FUTEX_FN(&futex->word, FUTEX_WAKE, 1);
}

