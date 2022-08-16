#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "ummap.h"

#define TRUE  1
#define FALSE 0
#define CHK(_hr) { int hr = _hr; if(hr != 0) return hr; }
#define CHKB(b)  { if(b) CHK(errno); }

int main(int argc, char **argv)
{
    int    flags    = (O_CREAT   | O_RDWR);
    mode_t mode     = (S_IRUSR   | S_IWUSR);
    int    prot     = (PROT_READ | PROT_WRITE);
    size_t size     = 1073741824; // 1GB allocation
    size_t segsize  = 16777216;   // 16MB segments
    off_t  offset   = 0;          // File offset
    int    fd       = 0;          // File descriptor
    int8_t *baseptr = NULL;       // Base pointer

    // Open the file descriptor for the mapping
    fd = open("./example.data", flags, mode);
    CHKB(fd == -1);
    
    // Ensure that the file has space (optional)
    CHK(ftruncate(fd, size));

    // Create the memory-mapping with uMMAP-IO
    CHK(ummap(size, segsize, prot, fd, offset, -1, FALSE, 0, (void **)&baseptr));

    // It is now safe to close the file descriptor
    CHK(close(fd));

    // Set some random value on the allocation
    for (off_t i = 0; i < size; i++)
    {
        baseptr[i] = 21;
    }

    // Alternative: Use traditional mem. functions
    memset(baseptr, 21, size);

    // Synchronize with storage to ensure that the latest changes are flushed
    CHK(umsync(baseptr, FALSE));

    // Finally, release the mapping if not needed (note that the storage
    // synchronization is not needed because of the previous statement)
    CHK(umunmap(baseptr, FALSE));
    
    return 0;
}

