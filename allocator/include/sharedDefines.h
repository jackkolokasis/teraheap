#ifndef __SHAREDDEFINES_H__
#define __SHAREDDEFINES_H__

#include <errno.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

/************************************************
 * Sith 6 
 ************************************************/
#define DEV "/mnt/spark/file.txt"	     //< Device name
#define DEV_SIZE (200*1024LU*1024*1024)  //< Device size (in bytes)

#define clean_errno() (errno == 0 ? "None" : strerror(errno))
#define log_error(M, ...) fprintf(stderr, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)
#define assertf(A, M, ...) if(!(A)) {log_error(M, ##__VA_ARGS__); assert(A);}

#define ANONYMOUS 0

#define TEST 1

#define SEG_SIZE 1073741824				 //< Segment size

#define MAX_REQS 64						 //< Maximum requests

#endif
