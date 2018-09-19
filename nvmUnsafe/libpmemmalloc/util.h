
/**************************************************
 *
 * file: util.h
 *
 * @Author:   Iacovos G. Kolokasis
 * @Version:  02-09-2018
 * @email:    kolokasis@ics.forth.gr
 *
 * Global defines for utile mode. This library
 * just contains common routines for printing
 * errors and debug information.  Nothing
 * particularly interesting here and nothing to do
 * with Persistent Memory.  
 *
 ***************************************************
 */

#ifndef UTIL_H_
#define UTIL_H_

#include <errno.h>

int Debug;
const char *Myname;

#define DEBUG(...)\
	debug(__FILE__, __LINE__, __func__, __VA_ARGS__)

#define FATALSYS(...)\
	fatal(errno, __FILE__, __LINE__, __func__, __VA_ARGS__)

#define FATAL(...)\
	fatal(0, __FILE__, __LINE__, __func__, __VA_ARGS__)

#define USAGE(...)\
	usage(Usage, __VA_ARGS__)

/* 
 * Assert a condition is true 
 */
#define	ASSERT(cnd)\
	((void)((cnd) || (fatal(0, __FILE__, __LINE__, __func__,\
	"assertion failure: %s", #cnd), 0)))

/* 
 * Assertion with extra info printed if assertion fails 
 */
#define	ASSERTinfo(cnd, info) \
	((void)((cnd) || (fatal(0, __FILE__, __LINE__, __func__,\
	"assertion failure: %s (%s = %s)", #cnd, #info, info), 0)))

/* 
 * Assert two integer values are equal 
 */
#define	ASSERTeq(lhs, rhs)\
	((void)(((lhs) == (rhs)) || (fatal(0, __FILE__, __LINE__, __func__,\
	"assertion failure: %s (%d) == %s (%d)", #lhs,\
	(lhs), #rhs, (rhs)), 0)))

/* 
 * Assert two integer values are not equal 
 */
#define	ASSERTne(lhs, rhs)\
	((void)(((lhs) != (rhs)) || (fatal(0, __FILE__, __LINE__, __func__,\
	"assertion failure: %s (%d) != %s (%d)", #lhs,\
	(lhs), #rhs, (rhs)), 0)))


/**
 * @desc Printf-like debug messages
 *
 * @param file  Output file name
 * @param line  Line number
 * @param func  Function name
 * @param fmt   printf-like format
 *
 **/
void debug(const char *file, int line, const char *func,
		const char *fmt, ...);

/**
 * @desc Printf-like error exits, with and without errno printing
 *
 * @param err   Error number
 * @param file  File name
 * @param line  Line number
 * @param func  Function name
 * @param fmt   Printf-like format
 *
 **/
void fatal(int err, const char *file, int line, const char *func,
		const char *fmt, ...);

/**
 * @desc Figure out the name used to run this program
 *       Use internally by usage()
 *
 **/
void usage(const char *argfmt, const char *fmt, ...);

#endif	// UTIL_H_
