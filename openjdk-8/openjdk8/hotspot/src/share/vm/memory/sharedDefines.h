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

/***********************************
 * DEBUG
 **********************************/
#define clean_errno() (errno == 0 ? "None" : strerror(errno))
#define log_error(M, ...) fprintf(stderr, "[ERROR] (%s:%d: errno: %s) " M "\n",\
		                  __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__) 
#define assertf(A, M, ...) if(!(A)) {log_error(M, ##__VA_ARGS__); assert(A); os::abort();}

#define DEBUG_SLOWPATH_INTR		 0	//< Use only interpreter for object allocation

#define DEBUG_ANNO_INTR     	 1	//< Debug @Cache annotation, TODO Disable in Spark experiments

#define DEBUG_TERACACHE     	 0	//< Debug prints for teraCache, TODO Disable in experiments

#define DISABLE_TERACACHE		 0  //< Disable teraCache

#define DISABLE_PRECOMPACT		 0  //< Disable precompact of tera objects

#define TERA_CARDS				 1  //< Disable teraCache, TODO Set to 1

#define TERA_FLAG				 1  //< Define teraFlag word, TODO Set to 1

#define TERA_C1				     1  //< Enable C1 to support TeraCache, TODO Set to 1

#define TERA_C2				     1  //< Enable C1 to support TeraCache, TODO Set to 1

#define TERA_INT			     1  //< Enable Interpreter to support TeraCache, TODO Set to 1

#define MT_STACK				 1  //< Enable multi threaded stack in Minor GC 
								    //  for TeraCache, TODO Set to 1
#define CLOSURE					 1  //< Closure Calculation TEST  !!!!!

#define DEBUG_INTR               0  //< Debug Interpreter

#define DEBUG_VECTORS			 0  //< Enable debug vectors in compaction phase
									//  to check every memmove operation if
									//  overwrites other objects

#define TEST_CLOSURE             0

#define TEST_CLONE				 0

#define EXPLICIT				 1  //< Enable explicit I/O path for the writes
									// in TeraCache during major GC


/**********************************
 * Policies for TeraCache
 **********************************/
#define TC_POLICY				1	//< Enable TeraCahce Policies (Always ON)

#define P_BALANCE				0	//< Balance Policy

#define P_AGGRESSIVE            0	//< Aggressive Policy

#define P_DISTINCT				1

#define P_SIMPLE                0	//< Move Objects to TeraCache based on their
									//  teraflag value. This policy should be
									//  used in combination with P_Balance or
									//  P_Aggressive

#define P_SIZE                  0	//< Move Objects to TeraCache based on their
									//  size. This policy should be used in
									//  combination with P_Balance or
									//  P_Aggressive

/**********************************
 * States of TeraFlag  
 **********************************/
#define MOVE_TO_TERA			255	//< Move this object to tera cache

#define TERA_TO_OLD		        328	//< Pointer from TeraCache to Old Gen. Move
									// this object to TeraCache

#define IN_TERA_CACHE    2147483561	//< This object is located in TeraCache

#define INIT_TF				   2035	//< Initial object state


/***********************************
 * Statistics
 **********************************/
#define STATISTICS			      0  //< Enable statistics for TeraCache



#endif  // _SHARE_DEFINES_H_
