#ifndef _UMMAP_H
#define _UMMAP_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Establishes an user-level memory-mapped I/O allocation, which maps a given
 * file from storage to memory in segments of the specified size.
 */
int ummap(size_t size, size_t seg_size, int prot, int fd, off_t offset,
          unsigned int flush_interval, int read_file, int ptype, void **ptr);

/**
 * Performs a selective synchronization of the dirty segments from the given
 * mapping to storage and thereafter evicts the mapping, if requested.
 */
int umsync(void *addr, int evict);

/**
 * Allows to re-configure an existing user-level memory-mapped I/O allocation.
 * The method guarantees a synchronization with storage before remapping, if
 * requested.
 */
int umremap(void *old_addr, int fd, off_t offset, int sync, void **new_addr);

/**
 * Releases an user-level memory-mapped I/O allocation. The method guarantees
 * a synchronization with storage before releasing, if requested.
 */
int umunmap(void *addr, int sync);

/**
 * Retrieves the current I/O stats, based on number of read / write operations.
 */
int umstats(unsigned int *num_reads, unsigned int *num_writes);

#ifdef __cplusplus
}
#endif

#endif

