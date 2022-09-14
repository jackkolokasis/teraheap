#ifndef _UMMAP_FUTEX_H
#define _UMMAP_FUTEX_H

#include <stdint.h>

#define FUTEX_INITIALIZER { .word = 1, .wcnt = 0 }

/**
 * Structure that contains the properties required for each futex.
 */
typedef struct
{
    int32_t word;   // Futex word used for synchronization
    int32_t wcnt;   // Counter to account for waiting process
} futex_t;

/**
 * Acquires the futex if available, otherwise forcing the process to wait.
 */
void futex_lock(futex_t *futex);

/**
 * Releases the futex if it was acquired, waking up any other process waiting.
 */
void futex_unlock(futex_t *futex);

#endif

