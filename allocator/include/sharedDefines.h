#ifndef __SHAREDDEFINES_H__
#define __SHAREDDEFINES_H__

#include <errno.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#define H2_DYNAMIC_FILE_ALLOCATION

#ifndef H2_DYNAMIC_FILE_ALLOCATION
	#define DEV "/mnt/fmap/file.txt"	     //< Device name
	#define DEV_SIZE (700*1024LU*1024*1024)  //< Device size (in bytes)
#else
	extern char dev[150];
	extern uint64_t dev_size;
#endif

//#define ASSERT

#ifdef ASSERT
#define clean_errno() (errno == 0 ? "None" : strerror(errno))
#define log_error(M, ...) fprintf(stderr, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)
#define assertf(A, M, ...) if(!(A)) {log_error(M, ##__VA_ARGS__); assert(A);}
#else
#define assertf(A, M, ...) ;
#endif

#define ANONYMOUS  0                //< Set to 1 for small mmaps

#define MAX_REQS	 64				  //< Maximum requests

#define BUFFER_SIZE  (8*1024LU*1024)  //< Buffer Size (in bytes) for async I/O

#define MALLOC_ON	1				  //< Allocate buffers dynamically

#define REGION_SIZE	(256*1024LU*1024) //< Region size (in bytes) for allignment
									                    // version

#if ANONYMOUS
#define V_SPACE (100*1024LU*1024*1024*1024) //< Virtual address space size for 
											// small mmaps
#define REGION_ARRAY_SIZE ((V_SPACE)/(REGION_SIZE))
#define MAX_PARTITIONS 256			  // Maximum partitions per RDD, affects 
									  // id array size

#define MAX_RDD_ID ((REGION_ARRAY_SIZE)/(MAX_PARTITIONS)) //< Total different rdds

#else

#ifndef H2_DYNAMIC_FILE_ALLOCATION
	#define REGION_ARRAY_SIZE ((DEV_SIZE)/(REGION_SIZE))
#else
	extern uint64_t  region_array_size;
#endif

#define MAX_PARTITIONS 256			  // Maximum partitions per RDD, affects 
									  // id array size
#ifndef H2_DYNAMIC_FILE_ALLOCATION
	#define MAX_RDD_ID ((REGION_ARRAY_SIZE)/(MAX_PARTITIONS)) //< Total different rdds
#else
	extern uint64_t max_rdd_id;
#endif

#endif

#ifndef H2_DYNAMIC_FILE_ALLOCATION
	#define GROUP_ARRAY_SIZE ((REGION_ARRAY_SIZE)/2)
#else
	extern uint64_t group_array_size;
#endif

#define MMAP_SIZE (4*1024*1024)       //< Size of small mmaps in Anonymous mode

#define STATISTICS 0				  //< Enable allocator to print statistics

#define DEBUG_PRINT 0			      //< Enable debug prints

#endif
