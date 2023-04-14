#ifndef __TERA_ALLOCATOR_H__
#define __TERA_ALLOCATOR_H__

#ifdef __cplusplus
extern "C" {
#endif

#if defined (__ia64__) || defined (__x86_64__)
#define INT_PTR unsigned long

#include <inttypes.h>
#include <stdlib.h>

#else
#define INT_PTR unsigned int
#endif

void init_arena(uint64_t entries);

char* tera_malloc(uint64_t size);

void free_arena();

#ifdef __cplusplus
}
#endif

#endif // __TERA_ALLOCATOR_H__
