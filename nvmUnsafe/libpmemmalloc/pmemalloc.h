
/**************************************************
 *
 * file: pmemmalloc.h
 *
 * @Author:   Iacovos G. Kolokasis
 * @Version:  01-09-2018
 * @email:    kolokasis@ics.forth.gr
 *
 * Persistent Memory malloc-like library
 *
 ***************************************************
 */

#ifndef PMEMALLOC_H_
#define PMEMALLOC_H_

#include <stdint.h>

/* Size of the static area returned by pmem_static_area() */
#define	PMEM_STATIC_SIZE 4096

/* Number of onactive/onfree values allowed */
#define	PMEM_NUM_ON 3

/*
 * Given a relative pointer, add in the base associated
 * with the given Persistent Memory Pool (pmp).
 */
#define	PMEM(pmp, ptr_) ((typeof(ptr_))(pmp + (uintptr_t)ptr_))

void *pmemalloc_init(const char *path, size_t size);
void *pmemalloc_static_area(void *pmp);
void *pmemalloc_reserve(void *pmp, size_t size);
void pmemalloc_onactive(void *pmp, void *ptr_, void **parentp_, void *nptr_);
void pmemalloc_onfree(void *pmp, void *ptr_, void **parentp_, void *nptr_);
void pmemalloc_activate(void *pmp, void *ptr_);
void pmemalloc_free(void *pmp, void *ptr_);
void pmemalloc_check(const char *path);

#endif // PMEMMALLOC_H_
