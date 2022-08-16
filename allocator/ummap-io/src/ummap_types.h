#ifndef _UMMAP_TYPES_H
#define _UMMAP_TYPES_H

#include "futex.h"
#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

#define _ __attribute__ ((aligned))

/**
 * Enumerate that defines the type of page-fault captured.
 */
typedef enum
{
    PAGEFAULT_READ = 0,
    PAGEFAULT_WRITE
} pf_type_t;

/**
 * Enumerate that defines the type of evict policy implementation.
 */
typedef enum
{
    UMMAP_PTYPE_FIFO = 0,
    UMMAP_PTYPE_LIFO,
    UMMAP_PTYPE_PLRU,
    UMMAP_PTYPE_PRND,
    UMMAP_PTYPE_WIRO_F,
    UMMAP_PTYPE_WIRO_L // Important: See "umpolicy_create" to handle errors
} ummap_ptype_t;

/**
 * Enumerate that defines the configuration of memory allocation.
 */
typedef enum
{
    UMMAP_MEMCONFIG_STATIC = 0,
    UMMAP_MEMCONFIG_DYNAMIC
} mconfig_t;

/**
 * Structure that defines the properties of each memory segment. The header is
 * divided according to the following structure (with 5-unused bits):
 * 
 *   ----------------------------------------------------------------
 *   |               FLUSH_TIME              | Reserved | R | D | V |
 *   ----------------------------------------------------------------
 *   ·              <- 56 bits ->            ·     <- 8 bits ->     ·
 */
typedef struct ummap_seg
{
    uint64_t header;               // Header with the properties (e.g., flags)
    futex_t  futex;                // Futex to guarantee data-integrity
    LIST_PTR_STRUCT(ummap_seg);    // Declare the internal list pointers
}_ ummap_seg_t;

/**
 * List and function pointers that define the functionality of the evict policy.
 */
DECLARE_LIST(seg, ummap_seg_t);
typedef void         (*notify_fn_t)(list_seg_t*, ummap_seg_t*, pf_type_t);
typedef void         (*modify_fn_t)(list_seg_t*, ummap_seg_t*, pf_type_t);
typedef ummap_seg_t* (*evict_fn_t)(list_seg_t*);

/**
 * Structure that defines the properties of the evict policy.
 */
typedef struct
{
    list_seg_t  list;              // List of the evict policy
    notify_fn_t notify;            // Method to notify segment faults
    modify_fn_t modify;            // Method to modify an existing fault
    evict_fn_t  evict;             // Method to evict segments
}_ ummap_policy_t;

/**
 * Structure that defines an user-level memory-mapped I/O allocation, which is
 * employed to map files in storage to memory.
 */
typedef struct ummap_alloc
{
    char           *addr;          // Address in memory of the mapping
    size_t         size;           // Size of the mapping
    ummap_seg_t    *alloc_seg;     // Memory segment structure
    size_t         seg_size;       // Segment size
    uint32_t       seg_shift;      // Shift to calculate the memory segment
    int32_t        fd;             // File descriptor
    off_t          offset;         // File offset
    int32_t        prot;           // Protection flags (i.e., permissions)
    uint32_t       flush_interval; // Flush interval to storage
    ummap_policy_t *policy;        // Evict policy for this allocation
    LIST_PTR_STRUCT(ummap_alloc);  // Declare the internal list pointers
}_ ummap_alloc_t;

/**
 * Structure that defines the state of the main I/O thread, that manages the
 * flushing to storage of the dirty segments.
 */
typedef struct
{
    pthread_t  tid;                // Thread identifier
    uint32_t   is_active;          // Flag to determine if the thread is allowed
    uint32_t   min_flush_interval; // Minimum wait interval between each flush
    sem_t      sem;                // Synchronization semaphore
}_ iothread_t;

/**
 * Structure that contains the global status for the library.
 */
typedef struct
{
    mconfig_t mconfig;             // Memory allocation configuration
    int32_t   bsync_enabled;       // Flag to enable bulk synchronization
    size_t    memlimit;            // Main memory limit for all the allocations
    uint32_t  r_index;             // Index of the rank (intra-node)
    uint32_t  *num_ranks;          // Current number of ranks (intra-node)
    int32_t   *ranks;              // Rank structure with the PIDs (intra-node)
    size_t    ranks_count;         // Size of rank structure, in num. elems.
    size_t    *memsize;            // Process' memory consumption (intra-node)
    size_t    **memsizes;          // Memory consumption structure (intra-node)
    size_t    memsizes_count;      // Size of memsizes structure, in num. elems.
    sem_t     *sem;                // Synchronization semaphore
    uint32_t  num_reads;           // Number of I/O read operations
    uint32_t  num_writes;          // Number of I/O write operations
}_ ummap_status_t;

#ifdef __cplusplus
}
#endif

#endif

