/*
 * Copyright (c) 2000, 2013, Oracle and/or its affiliates. All rights reserved.
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

#ifndef SHARE_VM_GC_IMPLEMENTATION_SHARED_MARKSWEEP_INLINE_HPP
#define SHARE_VM_GC_IMPLEMENTATION_SHARED_MARKSWEEP_INLINE_HPP

#include "gc_implementation/shared/markSweep.hpp"
#include "gc_interface/collectedHeap.hpp"
#include "runtime/globals.hpp"
#include "utilities/stack.inline.hpp"
#include "utilities/macros.hpp"
#include <iostream>
#include <cstring>
#include "memory/sharedDefines.h"
#include "gc_implementation/parallelScavenge/psScavenge.hpp"
#if INCLUDE_ALL_GCS
#include "gc_implementation/parallelScavenge/psParallelCompact.hpp"
#include "gc_implementation/teraCache/teraCache.hpp"
#endif // INCLUDE_ALL_GCS

inline void MarkSweep::mark_object(oop obj) {
  // some marks may contain information we need to preserve so we store them away
  // and overwrite the mark.  We'll restore it at the end of markSweep.
  markOop mark = obj->mark();
  obj->set_mark(markOopDesc::prototype()->set_marked());


  if (mark->must_be_preserved(obj)) {
    preserve_mark(obj, mark);
  }
}

inline void MarkSweep::follow_klass(Klass* klass, bool is_tera) {
	oop op = klass->klass_holder();

	MarkSweep::mark_and_push(&op);
}

inline void MarkSweep::follow_klass_tera_cache(Klass* klass) {
  oop op = klass->klass_holder();
  MarkSweep::trace_tera_cache(&op, true);
}


template <class T> inline void MarkSweep::follow_root(T* p) {
	assertf(!Universe::heap()->is_in_reserved(p),
			"roots shouldn't be things within the heap");
	T heap_oop = oopDesc::load_heap_oop(p);
	if (!oopDesc::is_null(heap_oop)) {
		oop obj = oopDesc::decode_heap_oop_not_null(heap_oop);

		if (!obj->mark()->is_marked()) {
			mark_object(obj);
			obj->follow_contents();
		}
	}
	follow_stack();
}

template <class T> inline void MarkSweep::mark_and_push(T* p) {
	//assertf(Universe::heap()->is_in_reserved(p), "should be in object space");
	T heap_oop = oopDesc::load_heap_oop(p);
	if (!oopDesc::is_null(heap_oop)) {
		oop obj = oopDesc::decode_heap_oop_not_null(heap_oop);

#if !DISABLE_TERACACHE
		if (EnableTeraCache && Universe::teraCache()->tc_check(obj))
		{
			if (TeraCacheStatistics)
				Universe::teraCache()->tc_increase_forward_ptrs();

			return;
		}
#endif

#if TEST_CLONE
		if (EnableTeraCache && !Universe::teraCache()->tc_check(obj))
			assertf(obj->get_obj_state() == MOVE_TO_TERA  // Object will move to TC
					|| obj->get_obj_state() == INIT_TF,      // TF init value
					"Non valid teraflag value %p | %lu | %s", 
					obj, obj->get_obj_state(), obj->klass()->internal_name());
#endif

		if (!obj->mark()->is_marked()) {
			mark_object(obj);

#if DEBUG_TERACACHE
			if (EnableTeraCache)
			{
				std::cerr <<"[MARK_AND_PUSH]" 
					<< " | P = " << p
					<< " | OBJECT = " 
					<< (HeapWord*)obj
					<< " | MARKED = "
					<< obj->mark()
					<< " | TERA = "
					<< obj->get_obj_state()
					<< std::endl;
			}
#endif

			_marking_stack.push(obj);
		}
	}
}

// Debug Trace TeraCache objects to check if they point back to heap
template <class T> inline void MarkSweep::trace_tera_cache(T* p, bool assert_on) {
	//assertf(Universe::heap()->is_in_reserved(p), "should be in object space");
	assertf(false, "HERE");
	T heap_oop = oopDesc::load_heap_oop(p);
	if (!oopDesc::is_null(heap_oop)) {
		oop obj = oopDesc::decode_heap_oop_not_null(heap_oop);

		// The object must belong to TeraCache only. If the object points back
		// to the heap, then we have backward pointers.
		if (assert_on) {
			assertf(Universe::teraCache()->tc_check(obj), 
					"Not allowed backward pointers: obj = %p | name = %s", obj, 
					obj->klass()->internal_name());
		}
		else {
			if (!Universe::teraCache()->tc_check(obj)) {
#if TERA_FLAG
				obj->set_tera_cache();
#endif
				Universe::teraCache()->add_tc_back_ptr((HeapWord *) obj);
				Universe::teraCache()->tc_print_map();
			}
		}
	}
}

template <class T> inline void MarkSweep::tera_mark_and_push(T* p) {
	//  assert(Universe::heap()->is_in_reserved(p), "should be in object space");
	T heap_oop = oopDesc::load_heap_oop(p);

	if (!oopDesc::is_null(heap_oop)) {
		oop obj = oopDesc::decode_heap_oop_not_null(heap_oop);

#if CLOSURE

		if (EnableTeraCache && (Universe::teraCache()->tc_check(obj)))
		{
			if (TeraCacheStatistics)
				Universe::teraCache()->tc_increase_forward_ptrs();
			return;
		}

#if TEST_CLONE
		if (EnableTeraCache && !Universe::teraCache()->tc_check(obj))
			assertf(obj->get_obj_state() == MOVE_TO_TERA  // Object will move to TC
					|| obj->get_obj_state() == INIT_TF,   // TF init value
					"Non valid teraflag value %p | %lu ", 
					obj, obj->get_obj_state());

#endif

#if P_BALANCE
		obj->set_tera_cache();

		if (!obj->mark()->is_marked()) {
			mark_object(obj);
#endif

#if P_AGGRESSIVE
			if (!(obj->mark()->is_marked() && obj->is_tera_cache())) {
				if (!obj->mark()->is_marked()) 
					mark_object(obj);

				obj->set_tera_cache();
#endif

#if DEBUG_TERACACHE
			if (EnableTeraCache) {
				std::cerr <<"[TERA_MARK_AND_PUSH]" 
					<< " | P = " << p
					<< " | OBJECT = " << (HeapWord*)obj
					<< " | MARKED = " << obj->mark()
					<< " | TERA = "   << obj->get_obj_state()
					<< std::endl;
			}
#endif
			_marking_stack.push(obj);
		}
#endif
	}
}

void MarkSweep::push_objarray(oop obj, size_t index) {
  ObjArrayTask task(obj, index);
  assert(task.is_valid(), "bad ObjArrayTask");
  _objarray_stack.push(task);
}

// Adust the pointers to the new location
template <class T> inline void MarkSweep::adjust_pointer(T* p) {
	T heap_oop = oopDesc::load_heap_oop(p);
	if (!oopDesc::is_null(heap_oop) ) {
		oop obj     = oopDesc::decode_heap_oop_not_null(heap_oop);
        assertf(Universe::heap()->is_in(obj) || 
				Universe::teraCache()->tc_check(obj), "should be in heap");

#if !DISABLE_TERACACHE
		oop new_obj = NULL;

		if (EnableTeraCache && Universe::teraCache()->tc_check(obj)) {
			new_obj = obj;
		}
		else {
			new_obj = oop(obj->mark()->decode_pointer());
		}
#else
		oop new_obj = oop(obj->mark()->decode_pointer());

#endif
	
			
#if DEBUG_TERACACHE
		if (EnableTeraCache)
		{
			std::cerr << "[ADJUST_CHECK] | P = " << p 
					  << " | O = "  << obj 
					  << " | MARK = "  << obj->mark()
					  << " | KLASS = "  << obj->klass() 
					  << " | TC = "  << obj->is_tera_cache()
				      << " | NEW_OBJ = " << new_obj << std::endl;
		}
#endif

		assertf(new_obj != NULL ||                                     // is forwarding ptr?
				obj->mark() == markOopDesc::prototype() ||             // not gc marked?
				(UseBiasedLocking && obj->mark()->has_bias_pattern()), // not gc marked?
				"should be forwarded");

		if (new_obj != NULL) {
			assertf(Universe::heap()->is_in_reserved(new_obj) 
					|| Universe::teraCache()->tc_check(new_obj), 
					"should be in object space");
			oopDesc::encode_store_heap_oop_not_null(p, new_obj);
		}
	}
}


template <class T> inline void MarkSweep::KeepAliveClosure::do_oop_work(T* p) {
  os::abort();
  mark_and_push(p);
}

#endif // SHARE_VM_GC_IMPLEMENTATION_SHARED_MARKSWEEP_INLINE_HPP
