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

#define CLOSURE					 1  //< Closure Calculation TEST  !!!!!

#define DEBUG_INTR               0  //< Debug Interpreter

#define DEBUG_VECTORS			 0  //< Enable debug vectors in compaction phase
									//  to check every memmove operation if
									//  overwrites other objects

#define DEBUG_PLACEMENT          0  //< Enable debug object placement in
									// TeraCache showing their RDD ID and
									// Partition ID

#define TEST_CLOSURE             0

#define TEST_CLONE				 0

#define PERF_TEST				 0  //< Performance tests in minor collection.
									// Keep this here until testing a lot and
									// then remove it.
									
#define SYNC				     0  //< Enable explicit I/O path for the writes
									// in TeraCache during major GC

#define ASYNC				     1  //< Asynchronous I/O path for the writes in
									// TeraCache

#define PR_BUFFER			     1  //< Enable promotion buffer for async I/O to
									// reduce the number of system calls 

#define FMAP				     0  //< When we use fastmap we need to ensure
									// that all the writes in buffered cached
									// will be flushed to the device because the
									// memory of fast map is different from
									// buffer cache. 

#define FMAP_HYBRID				 0  //< When we use fastmap hybrid version we
									// employ huge pages for mutator threads and
									// regular pages for GC 

#define MADVISE_ON				 1  //< During minor gc we advise kernel for
									// random accesses. During mutator thread
									// execution we advise kernel for sequential
									// accesses

#define TERA_CARD_SIZE			 13 // This is the size of each card in
									// TeraCache card table. The size is in bit
									// e.g 9 = 512bytes

#define REGIONS     			 1  // We set this to 1 when we are using
                                    // the region allocator and 0 when
                                    // we are not
/**********************************
 * Policies for TeraCache
 **********************************/
#define TC_POLICY				 1	//< Enable TeraCahce Policies (Always ON)

#define P_SD					 1	//< Move Objects to TeraCache based on
									//  serialization policy.  This policy
									//  should be used in combination with
									//  P_DISTINCT 

#define P_SD_BITMAP				 1	//< Bitmap to optimize the search in tree
									//  

#define P_SD_BACK_REF_CLOSURE	 1	//< Find the transitive closure of backward
									// edges

#define P_NO_TRANSFER            0	//< This policy is ONLY for debugging.
									//  Calculate the closure but do not move
									//  anything to TeraCache. This policy
									//  should be used in combination with
									//  P_POLICY and P_DISTINCT
									//
#define SPARK_POLICY			 0	// Policy that we use for Spark

#define P_SD_EXCLUDE_CLOSURE	 1	//< Exclude objects from the closure

#define P_SD_REF_EXCLUDE_CLOSURE 1  //< Exclude reference objects from the closure

#define P_GIRAPH_NOHINT_HIGH_WATERMARK 0 //< No hint with high watermark policy

#define P_GIRAPH_NOHINT_HIGH_LOW_WATERMARK 0 //< No hint with hihg + low threshold

#define P_GIRAPH_HINT_HIGH_LOW_WATERMARK 1 //< Hint with high and low watermark

/**********************************
 * States of TeraFlag  
 **********************************/
#define MOVE_TO_TERA			255	//< Move this object to tera cache

#define TERA_TO_OLD		        328	//< Pointer from TeraCache to Old Gen. Move
									// this object to TeraCache

#define IN_TERA_CACHE    2147483561	//< This object is located in TeraCache

#define INIT_TF				   2035	//< Initial object state

#define LIVE_TERA_OBJ           202 //< Object marked as live during GC Analysis

#define VISITED_TERA_OBJ        203 //< Object visited during GC Analysis

#define TRANSIENT_FIELD		    428	//< Objects is pointed by transient field


/***********************************
 * Statistics
 **********************************/
#define STATISTICS			      0  //< Enable statistics for
                                     // TeraCache Allocator. Do NOT
                                     // use this flag in performance
                                     // measurements

#define VERBOSE_TC				  0  //< Print objects in TeraCache during
									 // allocation

#define PREFETCHING				  0  //< Enable read ahead in TeraCache

#define BACK_REF_STAT             0  //< Collect statistics for backward refenrences

#define FWD_REF_STAT              0  //< Collect statistics for class object

#define DISABLE_TRAVERSE_OLD_GEN  1  //< Disable backward reference traversal
									 // from H2 to old generation (H1) during
									 // minor GC
#define GC_ANALYSIS               0

/************************************
 * Source code that we need to remove after testing
 ************************************/
#define CHECK_TERA_CARDS		  0  //< Check if we need this. At this time we
									 // do not need it
#endif  // _SHARE_DEFINES_H_
