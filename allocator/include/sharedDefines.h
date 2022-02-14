#ifndef __SHAREDDEFINES_H__
#define __SHAREDDEFINES_H__

#include <errno.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#define DEV "/mnt/pmem_fsdax0/file.txt"	     //< Device name
#define DEV_SIZE (800*1024LU*1024*1024)  //< Device size (in bytes)

#define clean_errno() (errno == 0 ? "None" : strerror(errno))
#define log_error(M, ...) fprintf(stderr, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)
#define assertf(A, M, ...) if(!(A)) {log_error(M, ##__VA_ARGS__); assert(A);}
// #define assertf(A, M, ...) ;

#define ANONYMOUS    0                //< Set to 1 for small mmaps

#define MAX_REQS	 64				  //< Maximum requests

#define BUFFER_SIZE  (8*1024LU*1024)  //< Buffer Size (in bytes) for async I/O

#define MALLOC_ON	1				  //< Allocate buffers dynamically

#define ALIGN_ON	0				  //< Enable allocation with allignment in TC

#define REGION_SIZE	(64LU*1024*1024)//< Region size (in bytes) for allignment version

#define REGIONS 1

#define MAX_RDD_ID 128                // Maximum RDD_ID, affects id array size

#define MAX_PARTITIONS 256            // Maximum partitions per RDD, affects id array size

#define V_SPACE (100*1024LU*1024*1024*1024) //< Virtual address space size for small mmaps

#if ANONYMOUS
    #define REGION_ARRAY_SIZE ((V_SPACE)/(REGION_SIZE))
#else
    #define REGION_ARRAY_SIZE ((DEV_SIZE)/(REGION_SIZE))
#endif

#define GROUP_ARRAY_SIZE ((REGION_ARRAY_SIZE)/2)

#define MMAP_SIZE (4*1024*1024)       //< Size of small mmaps in Anonymous mode

#define GROUP_DEBUG 0				  //< Enable debugging of grouping operation

#define STATISTICS 1				  //< Enable allocator to print statistics

#define DEBUG_PRINT 1			      //< Enable debug prints

#endif
