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
//#if !NDEBUG
//#define assertf(A, M, ...) if(!(A)) {log_error(M, ##__VA_ARGS__); assert(A); os::abort();}
//#elif
#define assertf(A, M, ...) ;
//#endif

/***********************************
 * Transient field bitmap array
 **********************************/
#define SetBit(A,k)     ( A[(k/32)] |= (1 << (k%32)) )
#define ClearBit(A,k)   ( A[(k/32)] &= ~(1 << (k%32)) )
#define TestBit(A,k)    ( A[(k/32)] & (1 << (k%32)) )

#define DEBUG_SLOWPATH_INTR		 0	//< Use only interpreter for object allocation

#define DEBUG_ANNO_INTR     	 0	//< Debug @Cache annotation, TODO Disable in Spark experiments

#define DEBUG_TERACACHE     	 0	//< Debug prints for teraCache, TODO Disable in experiments

#define DISABLE_TERACACHE		 0  //< Disable teraCache

#define DISABLE_PRECOMPACT		 0  //< Disable precompact of tera objects

#define TERA_CARDS				 1  //< Disable teraCache, TODO Set to 1

#define TERA_FLAG				 1  //< Define teraFlag word, TODO Set to 1

#define TERA_C1				     1  //< Enable C1 to support TeraCache, TODO Set to 1

#define TERA_C2				     1  //< Enable C1 to support TeraCache, TODO Set to 1

#define C2_ONLY_LEAF_CALL		 0  //< C2 Compiler version - Comparisons and
									// card marking are all implemented in the
									// make_leaf_call()

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

#define PERF_TEST				 0  //< Performance tests in minor collection.
									// Keep this here until testing a lot and
									// then remove it.

#define SYNC				     0  //< Enable explicit I/O path for the writes
									// in TeraCache during major GC

#define ASYNC				     1  //< Asynchronous I/O path for the writes in
									// TeraCache

#define PR_BUFFER			     0  //< Enable promotion buffer for async I/O to
									// reduce the number of system calls 

#define PR_BUFFER_SIZE (2 * 1024 * 1024) //< Size of the promotion buffer in TeraCache

#define FMAP				     0  //< When we use fastmap we need to ensure
									// that all the writes in buffered cached
									// will be flushed to the device because the
									// memory of fast map is different from
									// buffer cache. 

#define FMAP_ASYNC				 0  //< When we use fastmap we need to ensure
									// that all the writes in buffered cached
									// will be flushed to the device because the
									// memory of fast map is different from
									// buffer cache. 

#define TERA_CARD_SIZE			14 // This is the size of each card in
									// TeraCache card table. The size is in bit
									// e.g 9 = 512bytes

/**********************************
 * Policies for TeraCache
 **********************************/
#define TC_POLICY				1	//< Enable TeraCahce Policies (Always ON)

#define P_BALANCE				0	//< Balance Policy. Move the first neighbors
									//  of cached data

#define P_AGGRESSIVE            0	//< Aggressive Policy. Move all the closure

#define P_DISTINCT				1   //< Move objects to TeraCache based on their
									//  size. We use -XX:TeraCacheThreshold to
									//  define the size lmit 

#define P_SIMPLE                0	//< Move Objects to TeraCache based on their
									//  teraflag value. This policy should be
									//  used in combination with P_Balance or
									//  P_Aggressive

#define P_SIZE                  0	//< Move Objects to TeraCache based on their
									//  size. This policy should be used in
									//  combination with P_Balance or
									//  P_Aggressive

#define P_SIZE                  0	//< Move Objects to TeraCache based on their
									//  size. This policy should be used in
									//  combination with P_Balance or
									//  P_Aggressive

#define P_SD					1	//< Move Objects to TeraCache based on
									//  serialization policy.  This policy
									//  should be used in combination with
									//  P_DISTINCT 

#define P_SD_BITMAP				0	//< Bitmap to optimize the search in tree
									//  

#define P_NO_TRANSFER           0	//< This policy is ONLY for debugging.
									//  Calculate the closure but do not move
									//  anything to TeraCache. This policy
									//  should be used in combination with
									//  P_POLICY and P_DISTINCT

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

#define VERBOSE_TC				  0  //< Print objects in T

// TODO: This define must be removed
#define NEW_FEAT				  1  //< Enable when you add new feature

#endif  // _SHARE_DEFINES_H_
