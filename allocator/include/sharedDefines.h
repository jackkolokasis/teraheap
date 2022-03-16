#ifndef __SHAREDDEFINES_H__
#define __SHAREDDEFINES_H__

#include <errno.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#define DEV "/mnt/fmap/file.txt"	     //< Device name
#define DEV_SIZE (900*1024LU*1024*1024)  //< Device size (in bytes)

#define clean_errno() (errno == 0 ? "None" : strerror(errno))
#define log_error(M, ...) fprintf(stderr, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)
#define assertf(A, M, ...) if(!(A)) {log_error(M, ##__VA_ARGS__); assert(A);}
// #define assertf(A, M, ...) ;

#define ANONYMOUS    0

#define MAX_REQS	 64				  //< Maximum requests

#define BUFFER_SIZE  (8*1024LU*1024)  //< Buffer Size (in bytes) for async I/O

#define MALLOC_ON	1				  //< Allocate buffers dynamically

#define ALIGN_ON	0				  //< Enable allocation with allignment in TC

#define REGION_SIZE	(512*1024LU*1024) //< Region size (in bytes) for allignment version

#define REGIONS 1					  //< Enable region allocation mode

#define REGION_ARRAY_SIZE ((DEV_SIZE)/(REGION_SIZE))

#define GROUP_ARRAY_SIZE ((REGION_ARRAY_SIZE)/2)

#define GROUP_DEBUG 0				  //< Enable debugging of grouping operation

#define STATISTICS 0				  //< Enable allocator to print statistics

#define DEBUG_PRINT 0			      //< Enable debug prints

#define TOTAL_PARTITIONS 1			  //< Total number of partitions

#define TOTAL_RDDS ((REGION_ARRAY_SIZE)/(TOTAL_PARTITIONS)) //< Support up to 28 RDDs

#endif
