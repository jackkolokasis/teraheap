
#if DEBUG_PRINT
    #warning Debug prints are enabled and may incur in performance penalties.
    #define DBGPRINT(input, ...) \
        fprintf(stderr, "[P%d] %s:%d -> %s() / " input "\n", \
                        getpid(), __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
    #define DBGPRINT(input, ...) // Ignoring all the prints for debugging
#endif

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
} while (0)
