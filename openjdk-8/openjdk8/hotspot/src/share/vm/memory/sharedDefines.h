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

#define clean_errno() (errno == 0 ? "None" : strerror(errno))
#define log_error(M, ...) fprintf(stderr, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__) 
#define assertf(A, M, ...) if(!(A)) {log_error(M, ##__VA_ARGS__); assert(A);}

#define DEBUG_SLOWPATH_INTR 0

#define DEBUG_ANNO_INTR 1

#define DEBUG_EXTRA_FIELD_MARK 1

#define DEBUG_PRINT 1

#define DEBUG_TERA_MARK_SWEEP 0

#define DEBUG_TERA_CACHE 0

#define DEBUG_PRINTS 0  // remember to delete all these prints

//#define BREAKPOINT asm volatile("int3;")

////////////////////////////////////////
// Statistics
///////////////////////////////////////
#define STATISTICS 0 // Count regions

#endif  // _SHARE_DEFINES_H_
