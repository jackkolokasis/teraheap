#ifndef __SHAREDDEFINES_H__
#define __SHAREDDEFINES_H__

#include <errno.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
extern char dev[150];
extern uint64_t dev_size;

#define ASSERT
// #define DEV "/mnt/fmap/file.txt"	     //< Device name
//#define DEV "/mnt/fmap/h2-100.heap"	     //< Device name
//#define DEV_SIZE (100*1024LU*1024*1024)  //< Device size (in bytes)


#ifdef ASSERT
#define clean_errno() (errno == 0 ? "None" : strerror(errno))
#define log_error(M, ...) fprintf(stderr, "[ERROR] (%s:%s:%d: errno: %s) " M "\n", __FILE__, __func__, __LINE__, clean_errno(), ##__VA_ARGS__)
#define assertf(A, ...) if(!(A)) {log_error(__VA_ARGS__); assert(A);}
#else
#define assertf(A, ...) ;
#endif

#define ANONYMOUS  0                //< Set to 1 for small mmaps

#define MAX_REQS       64				  //< Maximum requests

#define BUFFER_SIZE  (8*1024LU*1024)  //< Buffer Size (in bytes) for async I/O

#define MALLOC_ON	1				  //< Allocate buffers dynamically

#define REGION_SIZE	(256*1024LU*1024) //< Region size (in bytes) for allignment
									                    // version
extern uint64_t region_array_size;

//#if ANONYMOUS
//#define V_SPACE (100*1024LU*1024*1024*1024) //< Virtual address space size for 
											// small mmaps
//#define REGION_ARRAY_SIZE ((V_SPACE)/(REGION_SIZE))
#define MAX_PARTITIONS 256			  // Maximum partitions per RDD, affects 
									  // id array size
extern uint64_t max_rdd_id;

extern uint64_t group_array_size;
/*
#define MAX_RDD_ID ((REGION_ARRAY_SIZE)/(MAX_PARTITIONS)) //< Total different rdds

#else
#define REGION_ARRAY_SIZE ((DEV_SIZE)/(REGION_SIZE))

#define MAX_PARTITIONS 256			  // Maximum partitions per RDD, affects 
									  // id array size
#define MAX_RDD_ID ((REGION_ARRAY_SIZE)/(MAX_PARTITIONS)) //< Total different rdds

#endif

#define GROUP_ARRAY_SIZE ((REGION_ARRAY_SIZE)/2)

#define MMAP_SIZE (4*1024*1024)       //< Size of small mmaps in Anonymous mode
*/

#define STATISTICS 0				  //< Enable allocator to print statistics
#define DEBUG_PRINT 0			      //< Enable debug prints


#define GB ((uint64_t)1 << 30) //1GB
#define CONVERT_TO_GB(bytes) (uint64_t)bytes >> 30
#define CONVERT_TO_MB(bytes) (uint64_t)bytes >> 20
#define CONVERT_TO_KB(bytes) (uint64_t)bytes >> 10
#define H1_CARD_SIZE ((uint64_t) (1 << 9))
#define H2_CARD_SIZE ((uint64_t) (1 << 13))
#define PAGE_SIZE ((uint64_t)sysconf(_SC_PAGESIZE))
#define H1_ALIGNMENT H1_CARD_SIZE * PAGE_SIZE
#define H2_ALIGNMENT H2_CARD_SIZE * PAGE_SIZE
#define ERRNO_CHECK if(errno){ \
      exit(EXIT_FAILURE);\
    }
extern FILE *allocator_log_fp;

#endif
