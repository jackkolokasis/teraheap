#ifndef __SHAREDDEFINES_H__
#define __SHAREDDEFINES_H__

#include <errno.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#define DEV "/mnt/spark/file.txt"	     //< Device name
#define DEV_SIZE (200*1024LU*1024*1024)    //< Device size (in bytes)

#define clean_errno() (errno == 0 ? "None" : strerror(errno))
#define log_error(M, ...) fprintf(stderr, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)
#define assertf(A, M, ...) if(!(A)) {log_error(M, ##__VA_ARGS__); assert(A);}
// #define assertf(A, M, ...) ;

#define ANONYMOUS    0

#define MAX_REQS	 64				  //< Maximum requests

#define BUFFER_SIZE  (8*1024LU*1024)  //< Buffer Size (in bytes) for async I/O

#define MALLOC_ON	1				  //< Allocate buffers dynamically

#define ALIGN_ON	1				  //< Enable allocation with allignment in TC

#define REGION_SIZE	(512*1024LU*1024) //< Region size (in bytes) for allignment version

#endif
