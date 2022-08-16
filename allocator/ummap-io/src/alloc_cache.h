#ifndef _ALLOC_CACHE_H
#define _ALLOC_CACHE_H

#ifdef __cplusplus
extern "C" {
#endif

#define CACHE_SIZE_STEP 21

/**
 * Macro that allows to declare functions for managing the internal cache of a
 * certain type (e.g., MPI_Win_Alloc cache).
 */
#define DECLARE_CACHE(S,T,SN) \
    int add##S(T SN); \
    int remove##S(T SN);

/**
 * Macro that defines the structure and functions for the internal cache of a
 * certain type (e.g., MPI_Win_Alloc cache).
 */
#define CREATE_CACHE(S,T,SN,...) \
    static struct S##Store \
    { \
        T   *data; \
        int count; \
        int size;  \
    } SN##_cache = { 0 }; \
    \
    __VA_ARGS__ int add##S(T SN) \
    { \
        if (SN##_cache.count == SN##_cache.size) \
        { \
            SN##_cache.size += CACHE_SIZE_STEP; \
            SN##_cache.data  = (T *)realloc(SN##_cache.data, \
                                            sizeof(T) * SN##_cache.size); \
        } \
        \
        SN##_cache.data[SN##_cache.count++] = SN; \
        \
        return ESUCCESS; \
    } \
    \
    __VA_ARGS__ void delete##S##Entry(int index) \
    { \
        memmove(&SN##_cache.data[index], &SN##_cache.data[index+1], \
                sizeof(T) * (SN##_cache.count - index - 1)); \
        SN##_cache.count--; \
        \
        if (SN##_cache.count == 0) \
        { \
            free(SN##_cache.data); \
            SN##_cache.data = NULL; \
            SN##_cache.size = 0; \
        } \
        else if (!(SN##_cache.count % CACHE_SIZE_STEP)) \
        { \
            SN##_cache.size -= CACHE_SIZE_STEP; \
            SN##_cache.data  = (T *)realloc(SN##_cache.data, \
                                            sizeof(T) * SN##_cache.size); \
        } \
    } \
    \
    __VA_ARGS__ int remove##S(T SN) \
    { \
        for (int i = 0; i < SN##_cache.count; i++) \
        { \
            if (SN##_cache.data[i] == SN) \
            { \
                delete##S##Entry(i); \
                \
                return ESUCCESS; \
            } \
        } \
        \
        return EINVAL; \
    }

#ifdef __cplusplus
}
#endif

#endif

