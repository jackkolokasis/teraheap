
#include "common.h"
#include "futex.h"

#define FUTEX_FN(futex, op, val) \
    syscall(SYS_futex, futex, op, val, NULL, NULL, 0)

int futex_lock(futex_t *futex) __CHK_FN__
{
    // Check if the futex is not available, forcing the process to wait
    while (!__sync_val_compare_and_swap(&futex->word, 1, 0))
    {
        __sync_add_and_fetch(&futex->wcnt, 1);
        
        CHKB((FUTEX_FN(&futex->word, FUTEX_WAIT, 0) < 0 && errno != EAGAIN),
             ENOLCK);
        
        __sync_sub_and_fetch(&futex->wcnt, 1);
    }
    
    return CHK_SUCCESS(CHK_EMPTY_ERROR_FN);
}

int futex_unlock(futex_t *futex) __CHK_FN__
{
    // Important: The counter is required to optimize the performance of the
    //            unlock function (i.e., "FUTEX_WAKE" requires switching to
    //            the kernel from user-space).
    
    // If the futex was taken, try to wake-up one of the waiting processes
    if (!__sync_val_compare_and_swap(&futex->word, 0, 1) && futex->wcnt > 0)
    {
        CHKB((FUTEX_FN(&futex->word, FUTEX_WAKE, 1) < 0), ENOLCK);
    }
    
    return CHK_SUCCESS(CHK_EMPTY_ERROR_FN);
}

