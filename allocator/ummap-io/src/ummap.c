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
#include "../include/ummap_types.h"
#include "../include/ummap.h"

ummap_alloc_t *ualloc;  // Usespace mmap instance
#if FALSE
iothread_t *g_iothread;  // Eviction thread
#endif
int page_cache_size = 0;

#define PAGE_SIZE        (4096)
#define PAGE_SHIFT       (  12)
#define PAGE_CACHE_LIMIT (524288)

#define IS_PAGE_VALID(page)    ((page)->header & __UINT64_C(1))
#define IS_PAGE_DIRTY(page)    ((page)->header & __UINT64_C(2))
#define IS_PAGE_READ(page)     ((page)->header & __UINT64_C(4))
#define RESET_VALID_FLAG(page) ((page)->header &= ~__UINT64_C(1))
#define SET_HEADER(page, V, D, R, FT) \
    ((page)->header = (uint64_t)(V | (D << 1) | (R << 2)) | (FT << 8))
#define GET_FLUSH_TIME(page)  ((page)->header >> 8)

static void write_page(off_t page_index) {
  void *page_addr = ualloc->addr + (page_index * PAGE_SIZE);
  pwrite(ualloc->fd, page_addr, PAGE_SIZE, page_index * PAGE_SIZE);
}

static void sync_page(ummap_page_t *page, off_t page_index) {
  futex_lock(&page->futex);
  write_page(page_index);
  SET_HEADER(page, FALSE, FALSE, TRUE, 0);
  futex_unlock(&page->futex);
}

static void ensure_page_fit() {
  if (page_cache_size + 1 <= PAGE_CACHE_LIMIT) {
    page_cache_size++;
    return;
  }

  int index;
  int total_pages = ualloc->size / PAGE_SIZE;
  int clean_index = 0;
  int dirty_index = 0;
  ummap_page_t *evict_page_clean = NULL;
  ummap_page_t *evict_page_dirty = NULL;

  DBGPRINT("Evict dirty");
  for (index = 0; index < total_pages; index++) {
    ummap_page_t *page = &(ualloc->page_array[index]); 
    if (IS_PAGE_VALID(page) && !IS_PAGE_DIRTY(page)) {
      evict_page_clean = page;
      clean_index = index;
      break;
    }

    if (evict_page_dirty == NULL && IS_PAGE_VALID(page) && IS_PAGE_DIRTY(page)) {
      evict_page_dirty = page;
      dirty_index = index;
    }
  }

  if (evict_page_clean == NULL && evict_page_dirty != NULL) {
    // Synchronize the segment with storage, if dirty
    sync_page(evict_page_dirty, dirty_index);
    fdatasync(ualloc->fd);
    // Mark the segment as non-valid
    madvise(ualloc->addr + (dirty_index + PAGE_SIZE),  PAGE_SIZE, MADV_DONTNEED);
    DBGPRINT("Evict dirty");
  }
  else {
    // Mark the segment as non-valid
    RESET_VALID_FLAG(evict_page_clean);
    madvise(ualloc->addr + (clean_index + PAGE_SIZE),  PAGE_SIZE, MADV_DONTNEED);
    DBGPRINT("Evict clean");
  }
}

static int read_page(unsigned long addr, void **buffer) {
  unsigned long page_align_addr = (unsigned long) addr & ~(PAGE_SIZE - 1);
  unsigned long file_offset = page_align_addr - (unsigned long) ualloc->addr;
  int read_bytes;

  read_bytes = pread(ualloc->fd, *buffer, PAGE_SIZE, file_offset);

  if (read_bytes == -1) {
      perror("pread fail");
      exit(EXIT_FAILURE);
  }

  return read_bytes;
}

static void * fault_handler_thread(void *arg)
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
    int           nready;
    uint64_t      page_index;
    ummap_page_t *page;

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

    page_index = ((unsigned long)msg.arg.pagefault.address - (unsigned long)ualloc->addr) >> PAGE_SHIFT;
    page = &ualloc->page_array[page_index];

    if (!IS_PAGE_VALID(page)) {
      // Ensure that we can fit another page
      ensure_page_fit();

      if (IS_PAGE_READ(page)) {
        int num_bytes = read_page((unsigned long) msg.arg.pagefault.address, buffer);
        DBGPRINT("Device read: %d bytes", num_bytes);
      }
    }
    // Update page metadata
    // Acquire the lock for the page
    futex_lock(&page->futex);
    DBGPRINT("Header Before: %lu", page->header);
    // Update the header to set the timestamp and the dirty flag, if needed
    SET_HEADER(page, TRUE, msg.arg.pagefault.flags, !!IS_PAGE_READ(page), (msg.arg.pagefault.flags * time(NULL)));
    DBGPRINT("Header Before: %lu", page->header);
    // Release the lock for the page
    futex_unlock(&page->futex);

    // Copy the page pointed to by 'page' into the faulting region.
    // Vary the contents that are copied in, so that it is more
    // obvious that each fault is handled separately.
    uffdio_copy.src = (unsigned long) buffer;

    // We need to handle page faults in units of pages(!). So, round
    // faulting address down to page boundary.
    uffdio_copy.dst  = (unsigned long) msg.arg.pagefault.address & ~(PAGE_SIZE - 1);
    uffdio_copy.len  = PAGE_SIZE;
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
  uint64_t i, num_pages = (size / PAGE_SIZE);

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
  
  ualloc->addr            = addr;
  ualloc->uffd            = uffd;
  ualloc->fd              = dup(fd);
  ualloc->size           = size;
  ualloc->uffdio_api      = uffdio_api;
  ualloc->uffdio_register = uffdio_register;
  ualloc->page_array      = calloc(num_pages, sizeof(ummap_page_t));

  for (i = 0; i < num_pages; i++) {
    ummap_page_t *page = &ualloc->page_array[i];
    SET_HEADER(page, FALSE, FALSE, FALSE, 0);
    page->futex = (futex_t)FUTEX_INITIALIZER;
  }

  // Create a thread that will process the userfaultfd events. 
  s = pthread_create(&ualloc->tid, NULL, fault_handler_thread, (void *) uffd);
  if (s != 0) {
    errno = s;
    errExit("pthread_create");
  }

#if FALSE
  // Setup and lunch the evicion io thread.
  g_iothread = calloc(1, sizeof(iothread_t));
  g_iothread->is_active = TRUE;
  sem_init(&g_iothread->sem, 0, 0);
  s = pthread_create(&g_iothread->tid, NULL, iothread_handler, NULL);
  if (s != 0) {
    errno = s;
    errExit("pthread_create");
  }
#endif

  *ptr = addr;
}
