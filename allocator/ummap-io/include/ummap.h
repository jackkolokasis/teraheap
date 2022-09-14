#ifndef __UMMAP_H__
#define __UMMAP_H__

#include <stdlib.h>
#include <linux/userfaultfd.h>

/**
 * Establishes an user-level memory-mapped I/O allocation, which maps a given
 * file from storage to memory in segments of the specified size.
 */
void ummap(size_t size, int prot, int fd, off_t offset, void **ptr);
#endif
