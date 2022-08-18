#include "ummap.h"
#define _GNU_SOURCE
  #include <inttypes.h>
#include <sys/types.h>
#include <stdio.h>
#include <linux/userfaultfd.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <poll.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <poll.h>

#include "../include/common.h"
#include "../include/ummap.h"

pthread_t thr;      /* ID of thread that handles page faults */
ummap_alloc_t *ualloc;

#define PAGE_SIZE (sysconf(_SC_PAGE_SIZE))

static void *
fault_handler_thread(void *arg)
{
  static struct uffd_msg msg;   /* Data read from userfaultfd */
  long uffd;                    /* userfaultfd file descriptor */
  struct uffdio_copy uffdio_copy;
  ssize_t nread;
  void *buffer = calloc(PAGE_SIZE, sizeof(char));

  uffd = (long) arg;

  /* Loop, handling incoming events on the userfaultfd
              file descriptor. */
  for (;;) {
    /* See what poll() tells us about the userfaultfd. */
    struct pollfd pollfd;
    int nready;
    pollfd.fd = uffd;
    pollfd.events = POLLIN;
    nready = poll(&pollfd, 1, -1);
    if (nready == -1)
      errExit("poll");

    DBGPRINT();
    DBGPRINT("poll() returns: nready = %d | POLLIN = %d | POLLERR = %d",
             nready, (pollfd.revents & POLLIN) != 0, (pollfd.revents & POLLERR) != 0);

    /* Read an event from the userfaultfd. */
    nread = read(uffd, &msg, sizeof(msg));
    if (nread == 0) {
      perror("EOF on userfaultfd!\n");
      exit(EXIT_FAILURE);
    }

    if (nread == -1)
      errExit("read");

    /* We expect only one kind of event; verify that assumption. */

    if (msg.event != UFFD_EVENT_PAGEFAULT) {
      perror("Unexpected event on userfaultfd");
      exit(EXIT_FAILURE);
    }

    /* Display info about the page-fault event. */
    DBGPRINT("UFFD_EVENT_PAGEFAULT event:");
    DBGPRINT("flags = %llu; ", msg.arg.pagefault.flags);
    DBGPRINT("address = %llu", msg.arg.pagefault.address);

    /* Copy the page pointed to by 'page' into the faulting
                  region. Vary the contents that are copied in, so that it
                  is more obvious that each fault is handled separately. */
    unsigned long align_virt_addr = (unsigned long) msg.arg.pagefault.address & ~(PAGE_SIZE - 1);
    unsigned long file_offset = align_virt_addr - (unsigned long) ualloc->addr;

    pread(ualloc->fd, buffer, PAGE_SIZE, file_offset);

    uffdio_copy.src = (unsigned long) buffer;

    /* We need to handle page faults in units of pages(!).
                  So, round faulting address down to page boundary. */
    uffdio_copy.dst = (unsigned long) msg.arg.pagefault.address & ~(PAGE_SIZE - 1);
    uffdio_copy.len = PAGE_SIZE;
    uffdio_copy.mode = 0;
    uffdio_copy.copy = 0;
    if (ioctl(uffd, UFFDIO_COPY, &uffdio_copy) == -1)
      errExit("ioctl-UFFDIO_COPY");

    DBGPRINT("uffdio_copy.copy returned %lld", uffdio_copy.copy);
  }
}

void ummap(size_t size, int prot, int fd, off_t offset, void **ptr) {
  long uffd;          /* userfaultfd file descriptor */
  char *addr;         /* Start of region handled by userfaultfd */
  struct uffdio_api uffdio_api;
  struct uffdio_register uffdio_register;
  int s;

  /* Create and enable userfaultfd object. */
  uffd = syscall(__NR_userfaultfd, O_CLOEXEC | O_NONBLOCK);
  if (uffd == -1)
    errExit("userfaultfd");

  uffdio_api.api = UFFD_API;
  uffdio_api.features = 0;
  if (ioctl(uffd, UFFDIO_API, &uffdio_api) == -1)
    errExit("ioctl-UFFDIO_API");

  /* Create a private anonymous mapping. The memory will be
              demand-zero paged--that is, not yet allocated. When we
              actually touch the memory, it will be allocated via
              the userfaultfd. */
  addr = mmap(NULL, size, PROT_READ | PROT_WRITE,
              MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (addr == MAP_FAILED)
    errExit("mmap");

  DBGPRINT("Address returned by mmap() = %p", addr);
  ualloc = (ummap_alloc_t *)calloc(1, sizeof(ummap_alloc_t));


  /* Register the memory range of the mapping we just created for
              handling by the userfaultfd object. In mode, we request to track
              missing pages (i.e., pages that have not yet been faulted in). */
  uffdio_register.range.start = (unsigned long) addr;
  uffdio_register.range.len = size;
  uffdio_register.mode = UFFDIO_REGISTER_MODE_MISSING;
  if (ioctl(uffd, UFFDIO_REGISTER, &uffdio_register) == -1)
    errExit("ioctl-UFFDIO_REGISTER");
  
  ualloc->addr = addr;
  ualloc->uffd = uffd;
  ualloc->fd = dup(fd);
  ualloc->uffdio_api = uffdio_api;
  ualloc->uffdio_register = uffdio_register;

  /* Create a thread that will process the userfaultfd events. */
  s = pthread_create(&thr, NULL, fault_handler_thread, (void *) uffd);
  if (s != 0) {
    errno = s;
    errExit("pthread_create");
  }

  *ptr = addr;
}
