#ifndef __COMMON_H__
#define __COMMON_H__

#include <assert.h>

#define FALSE 0
#define TRUE  1

#if DEBUG_PRINT
    #warning Debug prints are enabled and may incur in performance penalties.
    #define DBGPRINT(input, ...) \
        fprintf(stderr, "[P%d] %s:%d -> %s() / " input "\n", \
                        getpid(), __FILE__, __LINE__, __func__, ##__VA_ARGS__)
    #define clean_errno() (errno == 0 ? "None" : strerror(errno))
    #define log_error(M, ...) fprintf(stderr, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)
    #define assertf(A, M, ...) if(!(A)) {log_error(M, ##__VA_ARGS__); assert(A);}
#else
    #define DBGPRINT(input, ...) // Ignoring all the prints for debugging
    #define assertf(A, M, ...) ;
#endif

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
} while (0)

#endif // __COMMON_H__
