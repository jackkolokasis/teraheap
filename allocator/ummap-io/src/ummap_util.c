
#include "common.h"
#include "ummap_util.h"

typedef struct sysinfo  sysinfo_t;
typedef struct stat     stat_t;

#define SHM_FFLAGS  (O_CREAT | O_RDWR)
#define SHM_FPERM   (S_IRUSR | S_IWUSR)
#define TIME_LIMIT  50000000LL // 50ms

uint32_t log2s(uint64_t n)
{
    uint64_t factor = 16 + (n > UINT32_MAX) * 16;
    uint64_t logn   = (n >= (1 << factor)) * factor;
    uint64_t limit  = factor + logn;
    
    while ((n >> logn) > 1)
    {
        logn += ((limit-logn) >> 1);
    }
    
    while (!(n >> logn))
    {
        logn--;
    }
    
    return (uint32_t)logn;
}

int ts_set(timespec_t *ts, time_t tv_sec, long tv_nsec) __CHK_FN__
{
    CHK(clock_gettime(CLOCK_REALTIME, ts));
    
    ts->tv_sec  += tv_sec;
    ts->tv_nsec += tv_nsec;
    
    return CHK_SUCCESS(CHK_EMPTY_ERROR_FN);
}

int get_totalram(size_t *totalram) __CHK_FN__
{
    sysinfo_t sinfo = { 0 };
    CHK(sysinfo(&sinfo));
    
    *totalram = sinfo.totalram * (size_t)sinfo.mem_unit;
    
    return CHK_SUCCESS(CHK_EMPTY_ERROR_FN);
}

int get_usedram(size_t *_usedram) __CHK_FN__
{
    static int        fd       = -1;
    // static size_t     baseram  = UINT64_MAX;
    static size_t     totalram = 0;
    static size_t     usedram  = 0;
    static off_t      offset   = 0;
    static size_t     key_size = 0;
    static timespec_t ts[2]    = { 0 };
    uint64_t          elapsed  = 0;
    
    // Open the meminfo file and cache the total amount of memory
    if (fd < 0)
    {
        char buffer[256] = { 0 };
        char *key_pos    = NULL;
        char *value_pos  = NULL;
        
        CHKB(((fd = open("/proc/meminfo", O_RDONLY)) < 0), EIO);
        CHK(get_totalram(&totalram));
        
        // Look for the position of the "MemAvailable:" keyword and its value
        CHKB((pread(fd, buffer, 256, 0) != 256 ||
             (key_pos   = strstr(buffer, "MemAvailable:")) == NULL ||
             (value_pos = strstr(key_pos, " kB")) == NULL), EIO);
        
        offset   = ((uintptr_t)key_pos   - (uintptr_t)buffer) + 13;
        key_size = ((uintptr_t)value_pos - (uintptr_t)buffer) - offset;
        
        CHK(ts_set(&ts[0], 0, -TIME_LIMIT));
    }
    
    CHK(clock_gettime(CLOCK_REALTIME, &ts[1]));
    
    // Calculated the elapsed time between the last retrieval
    elapsed = ((ts[1].tv_sec - ts[0].tv_sec) << 30) + // Approximate the seconds
              (ts[1].tv_nsec - ts[0].tv_nsec);
    
    // Read the last-known available memory, if needed
    if (elapsed >= TIME_LIMIT)
    {
        char buffer[16] = { 0 };
        
        CHKB((pread(fd, buffer, key_size, offset) != key_size ||
              sscanf(buffer, "%zu", &usedram) != 1), EIO);
        
        usedram = totalram - (usedram << 10);
        
        // Cache the base reference RSS (i.e., assuming used at the beginning)
        // if (baseram == UINT64_MAX || (usedram < baseram))
        // {
        //     baseram = usedram;
        // }
        
        // usedram -= baseram;
        
        CHK(clock_gettime(CLOCK_REALTIME, &ts[0]));
    }
    
    *_usedram = usedram;
    
    return CHK_SUCCESS(CHK_EMPTY_ERROR_FN);
}

int get_env(const char *name, const char *format, void *target) __CHK_FN__
{
    const char *buffer = getenv(name);
   
    CHKB((buffer != NULL && sscanf(buffer, format, target) != 1), EINVAL);
    
    return CHK_SUCCESS(CHK_EMPTY_ERROR_FN);
}

int open_shm(const char *name, size_t size, int8_t incr, void **addr,
             size_t *count) __CHK_FN__
{
    int32_t fd = -1;
    stat_t  st = { 0 };
    
    fd = shm_open(name, SHM_FFLAGS, SHM_FPERM);
    CHKB((fd < 0 || fstat(fd, &st)), ENOENT);
        
    if (size > 0 && (incr || st.st_size == 0))
    {
        st.st_size += size;
        CHK(ftruncate(fd, st.st_size));
    }
    
    *addr = mmap(NULL, st.st_size, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, 0);
    CHKB((*addr == MAP_FAILED), ENOMEM);
    CHK(close(fd));
    
    if (size > 0 && count != NULL)
    {
        *count = st.st_size / size;
    }
    
    return CHK_SUCCESS(CHK_EMPTY_ERROR_FN);
}

int open_sem(const char *name, uint32_t value, sem_t **sem) __CHK_FN__
{
    // Reset the errno variable to avoid issues when checking for EEXIST
    errno = 0;
    
    // Create and open the semaphore atomically, if it does not exist
    *sem = sem_open(name, SHM_FFLAGS | O_EXCL, SHM_FPERM, value);
    CHKB((*sem == SEM_FAILED && errno != EEXIST), ENOLCK);
    
    // If the semaphore is already created, try to open it
    if (*sem == SEM_FAILED)
    {
        *sem = sem_open(name, SHM_FFLAGS, SHM_FPERM, value);
        CHKB((*sem == SEM_FAILED), ENOLCK);
    }
    
    return CHK_SUCCESS(CHK_EMPTY_ERROR_FN);
}

