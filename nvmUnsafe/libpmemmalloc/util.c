
/**************************************************
 *
 * file: util.c
 *
 * @Author:   Iacovos G. Kolokasis
 * @Version:  02-09-2018
 * @email:    kolokasis@ics.forth.gr
 *
 * Implementation of util.h interface
 *
 ***************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <dirent.h>

#include "util.h"

/**
 * @desc Printf-like debug messages
 *
 * @param file  Output file name
 * @param line  Line number
 * @param func  Function name
 * @param fmt   printf-like format
 *
 **/
void
debug(const char *file, int line, const char *func, const char *fmt, ...)
{
	va_list ap;             /* Object of va_list hold the information needed  */ 
	int save_errno;         /* Error number                                   */

	if (!Debug)
		return;

    /* 
     * Save the number of the last error
     */
	save_errno = errno;

	fprintf(stderr, "debug: %s:%d %s()", file, line, func);

	if (fmt) {
		fprintf(stderr, ": ");
		va_start(ap, fmt);
		vfprintf(stderr, fmt, ap);
		va_end(ap);
	}
	fprintf(stderr, "\n");
	errno = save_errno;
}

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
void
fatal(int err, const char *file, int line, const char *func,
        const char *fmt, ...)
{
	va_list ap;             /* Object of va_list hold the information needed  */

	fprintf(stderr, "ERROR: %s:%d %s()", file, line, func);

	if (fmt) {
		fprintf(stderr, ": ");
		va_start(ap, fmt);
		vfprintf(stderr, fmt, ap);
		va_end(ap);
	}

	if (err)
		fprintf(stderr, ": %s", strerror(err));

	fprintf(stderr, "\n");
	
    exit(1);
}

/**
 * @desc Figure out the name used to run this program
 *       Use internally by usage()
 *
 **/
static const char *
exename(void)
{
	char proc[PATH_MAX];                /* Proceedure name                    */
	static char exename[PATH_MAX];      /* Executable file name               */
	int nbytes;                         /* Number of bytes                    */

	snprintf(proc, PATH_MAX, "/proc/%d/exe", getpid());
	if ((nbytes = readlink(proc, exename, PATH_MAX)) < 0)
		strcpy(exename, "Unknown");
	else
		exename[nbytes] = '\0';

	return exename;
}

/*
 * usage -- printf-like usage message emitter
 */
void
usage(const char *argfmt, const char *fmt, ...)
{
	va_list ap;             /* Object of va_list hold the information needed  */

	fprintf(stderr, "Usage: %s", (Myname == NULL) ? exename() : Myname);

    if (argfmt)
		fprintf(stderr, " %s", argfmt);

    if (fmt) {
		fprintf(stderr, ": ");
		va_start(ap, fmt);
		vfprintf(stderr, fmt, ap);
		va_end(ap);
	}
	fprintf(stderr, "\n");
	exit(1);
}
