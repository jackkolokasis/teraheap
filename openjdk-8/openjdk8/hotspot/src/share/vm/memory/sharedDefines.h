/**************************************************
 *
 * file: sharedDefines.h
 *
 * @Author:   Iacovos G. Kolokasis
 * @Version:  21-05-2020
 * @email:    kolokasis@ics.forth.gr
 *
 ***************************************************
 */

#ifndef _SHARE_DEFINES_H_
#define _SHARE_DEFINES_H_

#include <csignal>

//#define BREAKPOINT asm volatile("int3;");
#define clean_errno() (errno == 0 ? "None" : strerror(errno))
#define log_error(M, ...) fprintf(stderr, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__) 
#define assertf(A, M, ...) if(!(A)) {log_error(M, ##__VA_ARGS__); assert(A); os::abort();}

#define DEBUG_SLOWPATH_INTR 0

#define DEBUG_ANNO_INTR 1

#define DEBUG_EXTRA_FIELD_MARK 1

#define DEBUG_PRINT 1

#define DEBUG_TERA_MARK_SWEEP 0

#define DEBUG_TERA_CACHE 0

#define DEBUG_PRINTS 0		 // Remember to delete all these prints

/* States of the object */
#define MOVE_TO_TERA			255	// Move this object to tera cache
		
#define INIT_VALUE      		325	// Initial object state

#define VALID_DEAD      		425 // Object found as dead during precompaction

#define VALID_PRECOMPACT		525 // Object found at precompact phase and need to be move in new location

#define INVALID_PLACE_TO_MOVE	655	// The place contains an already copied object that has been moved by this GC


////////////////////////////////////////
// Statistics
///////////////////////////////////////
#define STATISTICS			0 // Count regions

#endif  // _SHARE_DEFINES_H_
