#ifndef _UMMAP_UTIL_H
#define _UMMAP_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct timespec timespec_t;

/**
 * Calculates the integer part of the base 2 logarithm of a given number.
 */
uint32_t log2s(uint64_t n);

/**
 * Defines a given time specification according to the current RTC time.
 */
int ts_set(timespec_t *ts, time_t tv_sec, long tv_nsec);

/**
 * Retrieves the maximum memory available in the system.
 */
int get_totalram(size_t *totalram);

/**
 * Retrieves the estimated memory consumption (i.e., resident).
 */
int get_usedram(size_t *usedram);

/**
 * Retrieves the environment variable with the given name if it exists.
 */
int get_env(const char *name, const char *format, void *target);

/**
 * Opens a shared memory segment and allows to extend it, if requested.
 */
int open_shm(const char *str, size_t size, int8_t incr, void **addr,
             size_t *count);

/**
 * Opens a shared named semaphore with an initial value.
 */
int open_sem(const char *name, uint32_t value, sem_t **sem);

#ifdef __cplusplus
}
#endif

#endif

