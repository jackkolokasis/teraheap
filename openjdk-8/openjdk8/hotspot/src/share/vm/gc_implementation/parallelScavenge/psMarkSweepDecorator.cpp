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

	/* Get TeraCache instance */
	TeraCache* tc = Universe::teraCache();

	/* Previous object */
	HeapWord* prev_q = NULL;

	while (q < t) {
		assert(oop(q)->mark()->is_marked() || oop(q)->mark()->is_unlocked() ||
				oop(q)->mark()->has_bias_pattern(),
				"these are the only valid states during a mark sweep");

		// Check if this object is marked
		if (oop(q)->is_gc_marked()) 
		{
			/* Prefetch beyond q */
			Prefetch::write(q, interval);

			/* Size of the object */
			size_t size = oop(q)->size();

			// Debug prints
			if (EnableTeraCache)
			{
				std::cerr << "[PRECOMPACT]"  << " | OBJECT = "  << oop(q) << " | CLASS = "
					<< oop(q)->klass()->signature_name() << " | MARK = " << oop(q)->mark() 
					<< std::endl;
			}

			// Check if the object is marked to be moved to teracache
			if (EnableTeraCache && oop(q)->is_tera_cache())
			{
				// Take a pointer from the region
				HeapWord* region_top = (HeapWord*) tc->tc_region_top(oop(q), size);

				// Store the forwarding pointer into the mark word
				oop(q)->forward_to(oop(region_top));

				/* Previous object */
				prev_q = q;

				/* Move to the next object */
				q += size;

				/* Set this object as live in the in the precompact space */
				end_of_live = q;
				/* Continue with the next object */
				continue;
			}

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
				assert(compact_top == dest->space()->bottom(), "Advanced to space already in use");
				assert(compact_end > compact_top, "Must always be space remaining");
				compaction_max_size = pointer_delta(compact_end, compact_top);
			}

			if (q != compact_top) 
			{
				if (EnableTeraCache)
				{
				std::cerr << "[PRECOMPACT_1]"  << " | OBJECT = "  << oop(q) << " | COMPACTION_TOP = "
					<< compact_top << "PREV_Q = " << prev_q << std::endl;
				}
				oop(q)->forward_to(oop(compact_top));
				assert(oop(q)->is_gc_marked(),  "encoding the pointer should preserve the mark");
			} 
			else 
			{
				/*
				 * In this case objects will not change posision in the heap. As
				 * a result their new location should be the same as the current
				 * location and forwardee() should return NULL
				 *
				 * If the object isn't moving we can just set the mark to the
				 * default mark and handle it specially later on. Check this
				 * maybe we do not need to do init_mark() again
				 */
				if (EnableTeraCache)
				{
					std::cerr << "[B_PRECOMPACT_2] " << " | O = " << q << " | MARK = " << oop(q)->mark() << std::endl; 
					oop(q)->init_mark();
					std::cerr << "[A_PRECOMPACT_2] " << " | O = " << q << " | MARK = " << oop(q)->mark() << std::endl; 
				}
				else 
				{
					oop(q)->init_mark();
				}

				assert(oop(q)->forwardee() == NULL,  "should be forwarded to NULL");
			}

			/* Update object start array */
			if (start_array) {
				start_array->allocate_block(compact_top);
			}

			/* Increase compation pointer */
			compact_top += size;
			if (compact_top > dest->space()->end())
				std::cerr << "Exceeding space in destination!!!" << std::endl;
			assert(compact_top <= dest->space()->end(), "Exceeding space in destination");

			/* Set the pointer to the previous object */
			prev_q = q;

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
				prev_q = end;
				end += oop(end)->size();

				// teracache 
				// add code for teracache

			} while (end < t && (!oop(end)->is_gc_marked()));// || oop(end)->is_tera_cache()));


			/* See if we might want to pretend this object is alive so that
			 * we don't have to compact quite as often.
			 */
			if (allowed_deadspace > 0 && q == compact_top) {
				size_t sz = pointer_delta(end, q);
				if (insert_deadspace(allowed_deadspace, q, sz)) {
					size_t compaction_max_size = pointer_delta(compact_end, compact_top);

					// This should only happen if a space in the young gen overflows the
					// old gen. If that should happen, we null out the start_array, because
					// the young spaces are not covered by one.
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
						assert(compact_top == dest->space()->bottom(), "Advanced to space already in use");
						assert(compact_end > compact_top, "Must always be space remaining");
						compaction_max_size =
							pointer_delta(compact_end, compact_top);
					}

					// store the forwarding pointer into the mark word
					if (q != compact_top) {
						oop(q)->forward_to(oop(compact_top));
						assert(oop(q)->is_gc_marked(), "encoding the pointer should preserve the mark");
					} else {
						// if the object isn't moving we can just set the mark to the default
						// mark and handle it specially later on.
						oop(q)->init_mark();
						assert(oop(q)->forwardee() == NULL, "should be forwarded to NULL");
					}

					// Update object start array
					if (start_array) {
						start_array->allocate_block(compact_top);
					}

					compact_top += sz;
					assert(compact_top <= dest->space()->end(), "Exceeding space in destination");

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
			q = end;
		} /*< End of else  */
	}     /*< End of while */

	assert(q == t, "just checking");
	if (liveRange != NULL) {
		liveRange->set_end(q);
	}
	_end_of_live = end_of_live;
	if (end_of_live < first_dead) {
		first_dead = end_of_live;
	}

	_first_dead = first_dead;

	// Update compaction top
	dest->set_compaction_top(compact_top);
}

bool PSMarkSweepDecorator::insert_deadspace(size_t& allowed_deadspace_words,
                                            HeapWord* q, size_t deadlength) {
  if (allowed_deadspace_words >= deadlength) {
    allowed_deadspace_words -= deadlength;
    CollectedHeap::fill_with_object(q, deadlength);
    oop(q)->set_mark(oop(q)->mark()->set_marked());
    assert((int) deadlength == oop(q)->size(), "bad filler object size");
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

	assert(_first_dead <= _end_of_live, "Stands to reason, no?");

	if (q < t && _first_dead > q && !oop(q)->is_gc_marked()) {
		// We have a chunk of the space which hasn't moved and we've
		// reinitialized the mark word during the previous pass, so we can't
		// use is_gc_marked for the traversal.
		HeapWord* end = _first_dead;

		while (q < end) {
			// point all the oops to the new location
			size_t size = oop(q)->adjust_pointers();
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
			assert(q > prev_q, "we should be moving forward through memory");
		}
	}

	assert(q == t, "just checking");
}

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

	if (EnableTeraCache)
	{
		HeapWord* const end = _first_dead;
		while (q < end)
		{
			/* Get the size of the object */
			size_t size = oop(q)->size();

			/* Check if this object have to be moved in TeraCache */
			std::cerr << "COMPACT_0 | " << q << " " << oop(q)->mark() << " " << oop(q)->is_tera_cache() << std::endl;
			if(oop(q)->is_gc_marked() && oop(q)->forwardee() != NULL)
			{
				HeapWord* compaction_top = (HeapWord*)oop(q)->forwardee();

				/* Copy object to the new destination */
				Copy::aligned_conjoint_words(q, compaction_top, size);

				/* Initialize mark word of the destination */
				oop(compaction_top)->init_mark();

				/* Initialize teracache word to zero */
				oop(compaction_top)->init_tera_cache();
			}

			/* Move to the next object */
			q += size;
		}
	}

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
		std::cerr << "[COMPACT_1]"  << " | OBJECT = "  << oop(q) << std::endl;
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
		std::cerr << "[COMPACT_2]"  << " | OBJECT = "  << q << std::endl;
		std::cerr << "[COMPACT_2]"  << " | FIRST_DEAD = "  << _first_dead << std::endl;
		q = (HeapWord*) oop(_first_dead)->mark()->decode_pointer();
		std::cerr << "[COMPACT_2]"  << " | NEW_OBJ = "  << q << std::endl;
	}
  }

  const intx scan_interval = PrefetchScanIntervalInBytes;
  const intx copy_interval = PrefetchCopyIntervalInBytes;

  while (q < t) {
	  if (EnableTeraCache)
	  {
	  std::cerr << "[COMPACT]" 
	      << " | OBJECT = " 
	      << oop(q)
	      << std::endl;
	  }

	  if (!oop(q)->is_gc_marked()) {

		  // mark is pointer to next marked oop
		  debug_only(prev_q = q);

		  // Get the object from the next live range
		  q = (HeapWord*) oop(q)->mark()->decode_pointer();

		  assert(q > prev_q, "we should be moving forward through memory");

	  } 
	  else 
	  {
		  // prefetch beyond q
		  Prefetch::read(q, scan_interval);

		  // size and destination
		  size_t size = oop(q)->size();
		  HeapWord* compaction_top = (HeapWord*)oop(q)->forwardee();

		  // prefetch beyond compaction_top
		  Prefetch::write(compaction_top, copy_interval);

		  // copy object and reinit its mark
		  assert(q != compaction_top, "everything in this pass should be moving");

		  Copy::aligned_conjoint_words(q, compaction_top, size);
		  oop(compaction_top)->init_mark();
		  oop(compaction_top)->init_tera_cache();

		  assert(oop(compaction_top)->klass() != NULL, "should have a class");

		  debug_only(prev_q = q);
		  q += size;
	  }
  }

  assert(compaction_top() >= space()->bottom() && compaction_top() <= space()->end(),
         "should point inside space");
  space()->set_top(compaction_top());

  if (mangle_free_space) {
    space()->mangle_unused_area();
  }
}
