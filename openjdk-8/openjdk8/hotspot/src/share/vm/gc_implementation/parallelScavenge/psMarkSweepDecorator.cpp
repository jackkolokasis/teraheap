/*
 * Copyright (c) 2001, 2013, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#include "gc_implementation/parallelScavenge/psYoungGen.hpp"
#include "memory/sharedDefines.h"
#include "precompiled.hpp"
#include "classfile/systemDictionary.hpp"
#include "gc_implementation/parallelScavenge/objectStartArray.hpp"
#include "gc_implementation/parallelScavenge/parallelScavengeHeap.hpp"
#include "gc_implementation/parallelScavenge/psMarkSweep.hpp"
#include "gc_implementation/parallelScavenge/psMarkSweepDecorator.hpp"
#include "gc_implementation/shared/liveRange.hpp"
#include "gc_implementation/shared/markSweep.inline.hpp"
#include "gc_implementation/shared/spaceDecorator.hpp"
#include "oops/oop.inline.hpp"
#include "runtime/globals.hpp"
#include "utilities/globalDefinitions.hpp"
#include <cstring>

PSMarkSweepDecorator* PSMarkSweepDecorator::_destination_decorator = NULL;


void PSMarkSweepDecorator::set_destination_decorator_tenured() {
  ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
  assert(heap->kind() == CollectedHeap::ParallelScavengeHeap, "Sanity");

  _destination_decorator = heap->old_gen()->object_mark_sweep();
}

void PSMarkSweepDecorator::advance_destination_decorator() {
  ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
  assert(heap->kind() == CollectedHeap::ParallelScavengeHeap, "Sanity");

  assert(_destination_decorator != NULL, "Sanity");

  PSMarkSweepDecorator* first = heap->old_gen()->object_mark_sweep();
  PSMarkSweepDecorator* second = heap->young_gen()->eden_mark_sweep();
  PSMarkSweepDecorator* third = heap->young_gen()->from_mark_sweep();
  PSMarkSweepDecorator* fourth = heap->young_gen()->to_mark_sweep();

  if ( _destination_decorator == first ) {
    _destination_decorator = second;
  } else if ( _destination_decorator == second ) {
    _destination_decorator = third;
  } else if ( _destination_decorator == third ) {
    _destination_decorator = fourth;
  } else {
    fatal("PSMarkSweep attempting to advance past last compaction area");
  }
}

PSMarkSweepDecorator* PSMarkSweepDecorator::destination_decorator() {
  assert(_destination_decorator != NULL, "Sanity");

  return _destination_decorator;
}

#if TC_POLICY
// Implementation of policies that move objects to the TeraCache This function
// takes as argument the position `q` of the object and its `size` in the Java
// heap and return `true` if the policy is satisfied, and `false` otherwise.
bool PSMarkSweepDecorator::tc_policy(HeapWord *q, size_t size) {
#if P_SIMPLE
	return oop(q)->is_tera_cache() && !PSScavenge::is_obj_in_young(oop(q));

#elif P_SIZE
	return oop(q)->is_tera_cache() && !PSScavenge::is_obj_in_young(oop(q))  
		&& size >= TeraCacheThreshold;

#elif P_DISTINCT
	return (oop(q)->is_tera_cache() 
		&& !PSScavenge::is_obj_in_young(oop(q))  
		&& size >= TeraCacheThreshold) || (oop(q)->is_tc_to_old());
#else
	return false;
#endif

}
#endif


// FIX ME FIX ME FIX ME FIX ME!!!!!!!!!
// The object forwarding code is duplicated. Factor this out!!!!!
//
// This method "precompacts" objects inside its space to dest. It places forwarding
// pointers into markOops for use by adjust_pointers. If "dest" should overflow, we
// finish by compacting into our own space.

void PSMarkSweepDecorator::precompact() {
	// Reset our own compact top.
	set_compaction_top(space()->bottom());

	/* We allow some amount of garbage towards the bottom of the space, so
	 * we don't start compacting before there is a significant gain to be made.
	 * Occasionally, we want to ensure a full compaction, which is determined
	 * by the MarkSweepAlwaysCompactCount parameter. This is a significant
	 * performance improvement!
	 */
	bool skip_dead = ((PSMarkSweep::total_invocations() % MarkSweepAlwaysCompactCount) != 0);

	// Number of allowed space
	size_t allowed_deadspace = 0;

	if (skip_dead) {
		const size_t ratio = allowed_dead_ratio();
		allowed_deadspace = space()->capacity_in_words() * ratio / 100;
	}

	// Fetch the current destination decorator
	PSMarkSweepDecorator* dest = destination_decorator();
	ObjectStartArray* start_array = dest->start_array();

	/* Compaction area start address */
	HeapWord* compact_top = dest->compaction_top();

	/* Compaction area end address */
	HeapWord* compact_end = dest->space()->end();

	/* Start address of the space */
	HeapWord* q = space()->bottom();
	/* End address of the space */
	HeapWord* t = space()->top();

	/* One byte beyond the last byte of the last live object. */
	HeapWord*  end_of_live= q;    

	/* The first dead object */
	HeapWord*  first_dead = space()->end();

	/* The current live range, recorded in the first header of proceding free
	 * area
	 */
	LiveRange* liveRange  = NULL;

	/* First dead object */
	_first_dead = first_dead;

	/* Prefetch interval */
	const intx interval = PrefetchScanIntervalInBytes;

#if !DISABLE_TERACACHE
	/* Get TeraCache instance */
	TeraCache* tc; 
	/* Get the current allocation top pointer in TeraCache */
	HeapWord* tc_alloc_top_pointer; 

	if (EnableTeraCache) {
		tc = Universe::teraCache();

		tc_alloc_top_pointer = (HeapWord *) Universe::teraCache()->tc_region_cur_ptr();
	}
#endif

	/* Previous object */
	HeapWord* prev_compact_top = NULL;

	while (q < t) {
		assertf(oop(q)->mark()->is_marked() || oop(q)->mark()->is_unlocked() ||
				oop(q)->mark()->has_bias_pattern(),
				"these are the only valid states during a mark sweep");
		
		assertf(q >= compact_top, "Current pointer must be greater than compact");
		assertf(Universe::heap()->is_in_reserved(compact_top), "Compact pointer must be in the reserved area");
			
#if DEBUG_TERACACHE
		if (EnableTeraCache)
		{
			std::cerr << "[PRECOMPACT]"  
				      << " | OBJECT = " << (HeapWord*) oop(q) 
					  << " | MARK = " << (HeapWord*) oop(q)->mark() 
					  << " | TC = " << oop(q)->is_tera_cache() 
					  << std::endl;
		}
#endif
		prev_compact_top = compact_top;

		// Check if this object is marked
		if (oop(q)->is_gc_marked()) 
		{
			/* Prefetch beyond q */
			Prefetch::write(q, interval);

			/* Size of the object */
			size_t size = oop(q)->size();

#if !DISABLE_PRECOMPACT
			// Check if the object needs to be moved in TeraCache based on the
			// current policy
			if (EnableTeraCache && tc_policy(q, size)) {
				// Take a pointer from the region
				HeapWord* region_top = (HeapWord*) tc->tc_region_top(oop(q), size);
				assertf(tc->tc_check(oop(region_top)), "Pointer from teraCache is not valid");

				// Store the forwarding pointer into the mark word
				oop(q)->forward_to(oop(region_top));

				// Encoding the pointer should preserve the mark
				assertf(oop(q)->is_gc_marked(),  "encoding the pointer should preserve the mark");

				// Mark TeraCache Card Table

				// Move to the next object
				q += size;

				// Set this object as live in the in the precompact space
				end_of_live = q;

				// Continue with the next object
				continue;
			}
#endif

			size_t compaction_max_size = pointer_delta(compact_end, compact_top);
			// This should only happen if a space in the young gen overflows the
			// old gen. If that should happen, we null out the start_array, because
			// the young spaces are not covered by one.
			while(size > compaction_max_size) {
				// First record the last compact_top
				dest->set_compaction_top(compact_top);

				// Advance to the next compaction decorator
				advance_destination_decorator();
				dest = destination_decorator();

				// Update compaction info
				start_array = dest->start_array();
				compact_top = dest->compaction_top();
				compact_end = dest->space()->end();

				assertf(compact_top == dest->space()->bottom(), "Advanced to space already in use");
				assertf(compact_end > compact_top, "Must always be space remaining");
				compaction_max_size = pointer_delta(compact_end, compact_top);
			}

			if (q != compact_top) {
				oop(q)->forward_to(oop(compact_top));
				assertf(oop(q)->is_gc_marked(),  "encoding the pointer should preserve the mark");
			} 
			else {
				/*
				 * In this case objects will not change posision in the heap. As
				 * a result their new location should be the same as the current
				 * location and forwardee() should return NULL
				 *
				 * If the object isn't moving we can just set the mark to the
				 * default mark and handle it specially later on. Check this
				 * maybe we do not need to do init_mark() again
				 */
				oop(q)->init_mark();
				assertf(oop(q)->forwardee() == NULL,  "should be forwarded to NULL");
			}

			/* Update object start array */
			if (start_array) {
				start_array->allocate_block(compact_top);
			}

			/* Increase compation pointer */
			compact_top += size;
			assertf(compact_top == prev_compact_top + size,
					"Compact top change | Obj = %p | Size = %d", oop(q), oop(q)->size());

			assertf(compact_top <= dest->space()->end(), "Exceeding space in destination");

			/* Move to the next object */
			q += size;

			/* Mark this object as the last lived object */
			end_of_live = q;
		} 
		else  {

			/* Run over all the contiguous dead objects */
			HeapWord* end = q;

			do {
				/* Prefetch beyond end */
				Prefetch::write(end, interval);

				end += oop(end)->size();

			} while (end < t && (!oop(end)->is_gc_marked()));

			/* See if we might want to pretend this object is alive so that
			 * we don't have to compact quite as often.
			 */

			/*
			 * If the amount of garbage does not reach the allowed_deadspace
			 * limit and the current object is equall to the compact_top pointer
			 * then we precess the following block.
			 *
			 * In this case, it is actually a garbage object, but it is treated
			 * as a live object. (To be more precise, it is considered that one
			 * live object occupies from the current location (q) to the next
			 * live object position (end),  and the forwarding pointer is
			 * embedded in it. 
			 *
			 * In this case, since the block continues until the end, the
			 * statements after are not executed. Then, the process proceeds to
			 * the next object in the loop.
			 *
			 * (whether or not the allowed_deadspace amount has been reached is
			 * Determined by PSMarkSweepDecorator::insert_deadspace().
			 *
			 * To be precise, PSMarkSweepDecorator::insert_deadspace () has an
			 * argument passed by reference, so the value of allowed_deadspace
			 * decreases each time it is called. When the amount of garbage
			 * reaches the initial amount of allowed_deadspace,
			 * allowed_deadspace The value will be 0)
			 *
			 */
			
			if (allowed_deadspace > 0 && q == compact_top) {
				size_t sz = pointer_delta(end, q);
				if (insert_deadspace(allowed_deadspace, q, sz)) {
					size_t compaction_max_size = pointer_delta(compact_end, compact_top);

					// This should only happen if a space in the young gen
					// overflows the old gen. If that should happen, we null out
					// the start_array, because the young spaces are not covered
					// by one.
					while (sz > compaction_max_size) {
						// First record the last compact_top
						dest->set_compaction_top(compact_top);

						// Advance to the next compaction decorator
						advance_destination_decorator();
						dest = destination_decorator();

						// Update compaction info
						start_array = dest->start_array();
						compact_top = dest->compaction_top();
						compact_end = dest->space()->end();
						assertf(compact_top == dest->space()->bottom(), "Advanced to space already in use");
						assertf(compact_end > compact_top, "Must always be space remaining");
						compaction_max_size = pointer_delta(compact_end, compact_top);
					}

					// store the forwarding pointer into the mark word
					if (q != compact_top) {
						oop(q)->forward_to(oop(compact_top));
						assertf(oop(q)->is_gc_marked(), "encoding the pointer should preserve the mark");
					} else {
						// if the object isn't moving we can just set the mark
						// to the default mark and handle it specially later on.
						oop(q)->init_mark();
						assertf(oop(q)->forwardee() == NULL, "should be forwarded to NULL");
					}

					// Update object start array
					if (start_array) {
						start_array->allocate_block(compact_top);
					}

					compact_top += sz;
					assertf(compact_top == prev_compact_top + sz, "Compact top change");
					assertf(compact_top <= dest->space()->end(), "Exceeding space in destination");

					q = end;
					end_of_live = end;
					continue;
				}
			}


			/* For the previous LiveRange, record the end of the live objects. */
			if (liveRange) {
				liveRange->set_end(q);
			}

			/* Record the current LiveRange object.
			 * liveRange->start() is overlaid on the mark word.
			 */
			liveRange = (LiveRange*)q;
			liveRange->set_start(end);
			liveRange->set_end(end);

			/* See if this is the first dead region. */
			if (q < first_dead) {
				first_dead = q;
			}

			/* Move on to the next object */
			assertf(compact_top == prev_compact_top, "Compact top change");
			q = end;

		} /*< End of else  */
	}     /*< End of while */

	assertf(q == t, "just checking");

	if (liveRange != NULL) {
		liveRange->set_end(q);
	}
	_end_of_live = end_of_live;
	if (end_of_live < first_dead) {
		first_dead = end_of_live;
	}

	_first_dead = first_dead;

#if !DISABLE_TERACACHE
	// Invalidate tera cards for the new objects that will be allocated in the
	// EnableTeraCache. Objects that are moved in TeraCache might have backward
	// pointers to the heap. So we invalidate these cards that map the new
	// allocated objects and they will be cleared during the next minor GC.
	if (EnableTeraCache && 
			tc_alloc_top_pointer != (HeapWord *)Universe::teraCache()->tc_region_cur_ptr()) {
		BarrierSet *bs = Universe::heap()->barrier_set();
		ModRefBarrierSet* modBS = (ModRefBarrierSet*)bs;

		modBS->tc_invalidate(tc_alloc_top_pointer, (HeapWord *)Universe::teraCache()->tc_region_cur_ptr());
	}
#endif

	// Update compaction top
	dest->set_compaction_top(compact_top);
}


/*
 * PSMarkSweepDecorator::insert_deadspace() creates a dummy object of the same
 * size as the garbage object area specified in the compaction destination until
 * the amount of garbage reaches the allowed_deadspace amount, and returns true.
 * When the amount of garbage reaches the allowed_deadspace amount, returns
 * false. 			 
 */
bool PSMarkSweepDecorator::insert_deadspace(size_t& allowed_deadspace_words,
                                            HeapWord* q, size_t deadlength) {
  if (allowed_deadspace_words >= deadlength) {
    allowed_deadspace_words -= deadlength;

    CollectedHeap::fill_with_object(q, deadlength);
    oop(q)->set_mark(oop(q)->mark()->set_marked());

#if TERA_FLAG
	assertf(oop(q)->get_obj_state() == INIT_TF, "Object state is wrong %lu", oop(q)->get_obj_state());
#endif

    assertf((int) deadlength == oop(q)->size(), "bad filler object size");
    // Recall that we required "q == compaction_top".
    return true;
  } else {
    allowed_deadspace_words = 0;
    return false;
  }
}

void PSMarkSweepDecorator::adjust_pointers() {
	// adjust all the interior pointers to point at the new locations of objects
	// Used by MarkSweep::mark_sweep_phase3()

	HeapWord* q = space()->bottom();
	HeapWord* t = _end_of_live;  // Established by "prepare_for_compaction".

	assertf(_first_dead <= _end_of_live, "Stands to reason, no?");

	if (q < t && _first_dead > q && !oop(q)->is_gc_marked()) {
		// We have a chunk of the space which hasn't moved and we've
		// reinitialized the mark word during the previous pass, so we can't
		// use is_gc_marked for the traversal.
		HeapWord* end = _first_dead;

		// Point all the oops to the new location
		while (q < end) {
			size_t size = oop(q)->adjust_pointers();
#if DEBUG_TERACACHE
			if (EnableTeraCache) {
			std::cerr << "[ADJUST]"  
				<< " | OBJECT = "  << (HeapWord*) oop(q) 
				<< " | MARK = " << (HeapWord*) oop(q)->mark() 
				<< " | STATE = " << oop(q)->get_obj_state() 
				<< " | SIZE = " << oop(q)->size() << std::endl;
			}
#endif
			q += size;
		}

		if (_first_dead == t) {
			q = t;
		} else {
			// $$$ This is funky.  Using this to read the previously written
			// LiveRange.  See also use below.
			q = (HeapWord*)oop(_first_dead)->mark()->decode_pointer();
		}
	}
	const intx interval = PrefetchScanIntervalInBytes;

	debug_only(HeapWord* prev_q = NULL);
	while (q < t) {
		// prefetch beyond q
		Prefetch::write(q, interval);
		if (oop(q)->is_gc_marked()) {
			// q is alive
			// point all the oops to the new location
			size_t size = oop(q)->adjust_pointers();
			debug_only(prev_q = q);
			q += size;
		} else {
			// q is not a live object, so its mark should point at the next
			// live object
			debug_only(prev_q = q);
			q = (HeapWord*) oop(q)->mark()->decode_pointer();
		}
	}

	assertf(q == t, "just checking");
}

// Verify on every compaction that objects contains a valid teraflag. 
// Valid values for Teraflag are:
//		- DEFAULT      (       2035) => Object belongs to heap
//		- MOVE_TO_TERA (        255) => Object should be move in TeraCache
//		- IN_TERA_CACHE( 2147483561) => Object is in TeraCache
//
#if DEBUG_VECTORS
void PSMarkSweepDecorator::verify_compacted_objects()
{
	for (size_t i = 0; i < _verify_objects.size(); i++)
	{
		oop obj = oop(_verify_objects[i]);

		if (Universe::teraCache()->tc_check(obj)) {
			assertf(obj->get_obj_state() == IN_TERA_CACHE,  "Error in compaction");
		}
		else {
			assertf(obj->get_obj_state() == MOVE_TO_TERA 
					|| obj->get_obj_state() == INIT_TF,  "Error in compaction");
		}
	}
}
#endif



void PSMarkSweepDecorator::compact(bool mangle_free_space ) {
  // Copy all live objects to their new location
  // Used by MarkSweep::mark_sweep_phase4()

  HeapWord*       q = space()->bottom();
  HeapWord* const t = _end_of_live;
  debug_only(HeapWord* prev_q = NULL);

  if (q < t && _first_dead > q && !oop(q)->is_gc_marked()) {

#ifdef ASSERT
    // We have a chunk of the space which hasn't moved and we've reinitialized the
    // mark word during the previous pass, so we can't use is_gc_marked for the
    // traversal.
    HeapWord* const end = _first_dead;

    while (q < end) {
      size_t size = oop(q)->size();
      assert(!oop(q)->is_gc_marked(), "should be unmarked (special dense prefix handling)");
      debug_only(prev_q = q);
      q += size;
    }
#endif

	// There are some cases that the new object are marked but the are not
	// compact because the first_dead object is equall to the end_of_live.
	// As a result of that, GC ignores these objects and there is a corruption
	// in the heap. In this space there are objects that need to move in
	// teracache.
#if !DISABLE_PRECOMPACT
	if (EnableTeraCache)
	{
		HeapWord* const end = _first_dead;

		// Give advise to kernel to prefetch pages for TeraCache random
		Universe::teraCache()->tc_enable_rand();

		while (q < end) {
			/* Get the size of the object */
			size_t size = oop(q)->size();

			if(oop(q)->is_gc_marked() && oop(q)->forwardee() != NULL) {
				HeapWord* compaction_top = (HeapWord*)oop(q)->forwardee();

#if DEBUG_TERACACHE
				std::cerr << "[COMPACT_1] | " << "O = " << q 
						  << " | MARK = "     << oop(q)->mark()
						  << " | TC = "       << oop(q)->get_obj_state();
#endif


				if (Universe::teraCache()->tc_check(oop(compaction_top))) {
					// Size is in words. Each word is 8 bytes. I use memcpy instead of
					// memmove to avoid the extra copy of the data in the buffer.
#if SYNC
					Universe::teraCache()->tc_write((char *)q, (char *)compaction_top, size);
					// Change the value of teraflag in the new location of the object
					oop(compaction_top)->set_obj_in_tc();
					/* Initialize mark word of the destination */
					oop(compaction_top)->init_mark();
#elif FMAP
					// Change the value of teraflag in the new location of the object
					oop(q)->set_obj_in_tc();
					/* Initialize mark word of the destination */
					oop(q)->init_mark();
					Universe::teraCache()->tc_write((char *)q, (char *)compaction_top, size);

#elif ASYNC
					// Change the value of teraflag in the new location of the object
					oop(q)->set_obj_in_tc();
					/* Initialize mark word of the destination */
					oop(q)->init_mark();
					Universe::teraCache()->tc_awrite((char *)q, (char *)compaction_top, size);
#else
					memcpy(compaction_top, q, size * 8);
					// Change the value of teraflag in the new location of the object
					oop(compaction_top)->set_obj_in_tc();
					/* Initialize mark word of the destination */
					oop(compaction_top)->init_mark();
#endif
				}
				else {
					/* Copy object to the new destination */
					Copy::aligned_conjoint_words(q, compaction_top, size);
					/* Initialize mark word of the destination */
					oop(compaction_top)->init_mark();
				}
			    
#if DEBUG_TERACACHE
				std::cerr << "=> NEW_ADDR = " << compaction_top 
						  << " | NEW_MARK = " << oop(compaction_top)->mark()
						  << " | NEW_TC = "     << oop(compaction_top)->get_obj_state()
						  << std::endl;
#endif

#if DEBUG_VECTORS
				_verify_objects.push_back(compaction_top);
				verify_compacted_objects();
#endif
			}
			else {
#if DEBUG_TERACACHE
					std::cerr << "[COMPACT_2] | "  << "O = " << q 
						  	  << " | MARK = "      << oop(q)->mark()
							  << " | STATE = " 	   << oop(q)->get_obj_state() 
				        	  << "=> NEW_ADDR = "  << q  << std::endl;
#endif
				oop(q)->init_mark();

				/* 
				 * Set the object state to show that this place holds a valid object
				 */
#if DEBUG_VECTORS
				_verify_objects.push_back(q);
				verify_compacted_objects();
#endif
			}

			/* Move to the next object */
			q += size;
		}
	}
#endif

	/* 
	 * +----------------------------------------------------+
	 * |													|
	 * +----------------------------------------------------+
	 * v													v
	 * q												last_lived_object
	 *														v
	 *													_first_dead_object
	 *														
	 * _first_dead_object is equall with the last_live_object
	 * As a result of that, the space is already compacted and the objects will
	 * not copy to a new place 
	 */
	if (_first_dead == t) 
	{
		q = t;
	} 
	/* 
	 * +----------------------------------------------------+
	 * |			     |	     							|
	 * +----------------------------------------------------+
	 * v				 |									v
	 * q			     |							last_lived_object
	 *					 v
	 *			_first_dead_object
	 *
	 * _first_dead_object is not equall with the last_live_object but the
	 * _first_dead_object is greater than the start of the space. In the mark
	 * word of the _first_dead_object is placed the address of the next live
	 * range.
	 * Between q and _first_dead_object there are objects that need to be moved
	 * in TeraCache
	 */
	else 
	{
		// $$$ Funky
		q = (HeapWord*) oop(_first_dead)->mark()->decode_pointer();
	}
  }

  const intx scan_interval = PrefetchScanIntervalInBytes;
  const intx copy_interval = PrefetchCopyIntervalInBytes;

  while (q < t) {

	  if (!oop(q)->is_gc_marked()) {

		  // mark is pointer to next marked oop
		  debug_only(prev_q = q);

		  // Get the object from the next live range
		  q = (HeapWord*) oop(q)->mark()->decode_pointer();

		  assert(q > prev_q, "we should be moving forward through memory");

	  } 
	  else {
		  // prefetch beyond q
		  Prefetch::read(q, scan_interval);

		  // size and destination
		  size_t size = oop(q)->size();
		  HeapWord* compaction_top = (HeapWord*)oop(q)->forwardee();

		  // prefetch beyond compaction_top
		  Prefetch::write(compaction_top, copy_interval);

		  // copy object and reinit its mark
		  assertf(q != compaction_top, "everything in this pass should be moving");

#if DEBUG_TERACACHE
		  if (EnableTeraCache) {
			  std::cerr << "[COMPACT] | "    << "O = " << q 
						<< " | MARK = "      << oop(q)->mark() 
						<< " | STATE = "     << oop(q)->get_obj_state();
		  }
#endif

#if !DISABLE_PRECOMPACT
		  // Change the value of teraflag in the new location of the object
		  if (EnableTeraCache && Universe::teraCache()->tc_check(oop(compaction_top))) {
			  // Size is in words. Each word is 8 bytes. I use memcpy instead of
			  // memmove to avoid the extra copy of the data in the buffer.
#if SYNC
			  Universe::teraCache()->tc_write((char *)q, (char *)compaction_top, size);
			  oop(compaction_top)->set_obj_in_tc();
			  oop(compaction_top)->init_mark();
#elif FMAP
			  oop(q)->set_obj_in_tc();
			  oop(q)->init_mark();
			  Universe::teraCache()->tc_write((char *)q, (char *)compaction_top, size);
#elif ASYNC
			  oop(q)->set_obj_in_tc();
			  oop(q)->init_mark();
			  Universe::teraCache()->tc_awrite((char *)q, (char *)compaction_top, size);
#else
			  memcpy(compaction_top, q, size * 8);
			  oop(compaction_top)->set_obj_in_tc();
			  oop(compaction_top)->init_mark();
#endif
		  }
		  else {
			  Copy::aligned_conjoint_words(q, compaction_top, size);
			  // Change the value of teraflag in the new location of the object
			  oop(compaction_top)->init_mark();
		  }
#else
		  Copy::aligned_conjoint_words(q, compaction_top, size);
		  oop(compaction_top)->init_mark();
#endif


#if DEBUG_TERACACHE
		  if (EnableTeraCache) {
			  std::cerr << "=> NEW_ADDR = "  << compaction_top 
						<< " | NEW_MARK = "  << oop(compaction_top)->mark() 
						<< " | NEW_STATE = " << oop(compaction_top)->get_obj_state() 
					    << std::endl;
		  }
#endif


#if DEBUG_VECTORS
		  if (EnableTeraCache) {
		      _verify_objects.push_back(compaction_top);
		      verify_compacted_objects();
		  }
#endif

		  //assertf(oop(compaction_top)->klass() != NULL, "should have a class");

		  debug_only(prev_q = q);
		  q += size;
	  }
  }
  
#if DEBUG_VECTORS
  if (EnableTeraCache) 
	  _verify_objects.clear();
#endif

  assertf(compaction_top() >= space()->bottom() && compaction_top() <= space()->end(),
         "should point inside space");
  space()->set_top(compaction_top());

  if (mangle_free_space) {
    space()->mangle_unused_area();
  }

  if (EnableTeraCache) {
#if ASYNC
  while(!Universe::teraCache()->tc_areq_completed());
#elif FMAP
  Universe::teraCache()->tc_fsync();
#endif
  }
}
