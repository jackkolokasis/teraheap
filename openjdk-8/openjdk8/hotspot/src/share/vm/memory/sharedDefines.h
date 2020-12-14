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

// JK: Remember code that nee to be tested is marked with TODO XX: TESTED

#ifndef _SHARE_DEFINES_H_
#define _SHARE_DEFINES_H_

#include <csignal>

/***********************************
 * DEBUG
 **********************************/
#define clean_errno() (errno == 0 ? "None" : strerror(errno))

#define log_error(M, ...) fprintf(stderr, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__,\
						  clean_errno(), ##__VA_ARGS__) 

#define assertf(A, M, ...) if(!(A)) {log_error(M, ##__VA_ARGS__); assert(A); os::abort();}

#define DEBUG_SLOWPATH_INTR		 0	//< Use only interpreter for object allocation

#define DEBUG_ANNO_INTR     	 1	//< Debug @Cache annotation, TODO Disable in experiments

#define DEBUG_TERACACHE     	 1	//< Debug prints for teraCache, TODO Disable in experiments

#define DEBUG_CLOSURE     	     1	//< Debug prints for teraCache, TODO Disable in experiments

#define DISABLE_TERACACHE		 0  //< Disable teraCache

#define DISABLE_PRECOMPACT		 0  //< Disable precompact of tera objects

#define TERA_CARDS				 1  //< Disable teraCache

#define TERA_FLAG				 1  //< Define teraFlag word, TODO Set to 1

#define CLOSURE					 1  //< Closure Calculation

#define DISABLE_TERACACHE_2		 1  //< Disable teraCache


/***********************************
 * States of the objects  
 **********************************/
#define MOVE_TO_TERA			255	//< Move this object to tera cache
		
#define INIT					325	//< Initial object state

#define DEAD      		        425 //< Object found as dead during precompaction

#define PRECOMPACT		        525 //< Object in precompact phase and need to be move in new location

#define VALID	                655	//< The place contains an already copied object that has been moved by this GC

#define INVALID					755	//< The place contains an old object

#define FLUSHED					575	//< The place contains a dummy object

#define IN_TERA_CACHE			339	//< This object is located in TeraCache

/***********************************
 * Statistics
 **********************************/
#define STATISTICS			      0  //< Enable statistics for TeraCache

#endif  // _SHARE_DEFINES_H_
