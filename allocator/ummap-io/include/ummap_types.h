#ifndef __UMMAP_TYPES_H__
#define __UMMAP_TYPES_H__

#include "futex.h"

#include <linux/userfaultfd.h>
#include <stdint.h>
#include <stdlib.h>
#include <semaphore.h>

/**
 * Enumerate that defines the type of page-fault captured.
 */
typedef enum
{
    PAGEFAULT_READ = 0,
    PAGEFAULT_WRITE
} pf_type_t;

/**
 * Structure that defines the properties of each memory page. The
 * header is divided according to the following structure (with
 * 5-unused bits):
 * 
 *   ----------------------------------------------------------------
 *   |               FLUSH_TIME              | Reserved | R | D | V |
 *   ----------------------------------------------------------------
 *   ·              <- 56 bits ->            ·     <- 8 bits ->     ·
 */
typedef struct ummap_page {
  uint64_t                header;          // Header with the properties (e.g., flags)
  futex_t                 futex;           // Futex to guarantee data-integrity
} ummap_page_t;

typedef struct ummap_ualloc {
  char                    *addr;           // Start addres of the anonymous mmap
  long                    uffd;            // Userfaultfd file descriptor
  int                     fd;              // File descriptor of the backed storage device
  size_t                  size;            // Size of the virtual address space
  struct uffdio_api       uffdio_api;       
  struct uffdio_register  uffdio_register;
  ummap_page_t            *page_array;     // Array of pages metadata
  pthread_t               tid;             // Thread identifier
} ummap_alloc_t;

/**
 * Structure that defines the state of the main I/O thread, that manages the
 * flushing to storage of the dirty segments.
 */
typedef struct iothread {
    pthread_t  tid;                // Thread identifier
    uint32_t   is_active;          // Flag to determine if the thread is allowed
    sem_t      sem;                // Synchronization semaphore
} iothread_t;

#endif // __UMMAP_TYPES_H__
