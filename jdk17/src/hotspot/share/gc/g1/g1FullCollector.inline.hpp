/*
 * Copyright (c) 2020, 2021, Oracle and/or its affiliates. All rights reserved.
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

#ifndef SHARE_GC_G1_G1FULLCOLLECTOR_INLINE_HPP
#define SHARE_GC_G1_G1FULLCOLLECTOR_INLINE_HPP

#include "gc/g1/g1FullCollector.hpp"

#include "gc/g1/g1FullGCHeapRegionAttr.hpp"
#include "gc/teraHeap/teraHeap.hpp"
#include "oops/oopsHierarchy.hpp"

bool G1FullCollector::should_compact_humongous(HeapRegion* hr) const {
  if (!EnableTeraHeap || !hr->is_humongous())
    return false;

  oop hum_obj = cast_to_oop(hr->humongous_start_region()->bottom());
  return hum_obj->is_marked_move_h2();
}

bool G1FullCollector::is_compacting(oop obj) const {
  return _region_attr_table.is_compacting(cast_from_oop<HeapWord *>(obj));
}

bool G1FullCollector::is_skip_compacting(uint region_index) const {
  return _region_attr_table.is_skip_compacting(region_index);
}

bool G1FullCollector::is_skip_marking(oop obj) const {
  return _region_attr_table.is_skip_marking(cast_from_oop<HeapWord*>(obj));
}

void G1FullCollector::set_invalid(uint region_idx) {
  _region_attr_table.set_invalid(region_idx);
}

void G1FullCollector::update_from_compacting_to_skip_compacting(uint region_idx) {
  _region_attr_table.verify_is_compacting(region_idx);
  _region_attr_table.set_skip_compacting(region_idx);
}

template<class T>
inline bool G1FullCollector::h2_should_trace(T* p) {
  T heap_oop = RawAccess<>::oop_load(p);

  if (CompressedOops::is_null(heap_oop))
    return false;

  oop obj = CompressedOops::decode_not_null(heap_oop);

  if (Universe::teraHeap()->is_obj_in_h2(obj)) {
    // Group regions if the references belong to two individual groups
    Universe::teraHeap()->group_regions((HeapWord *)p, cast_from_oop<HeapWord *>(obj));
    return false;
  }

  G1CollectedHeap *g1h = G1CollectedHeap::heap();

  assert(g1h->is_in_young(cast_to_oop(heap_oop)) ||
         Universe::teraHeap()->is_field_in_h2((void *) p), "Error in h2_should_trace");

  Universe::teraHeap()->h2_push_backward_reference((void *)p, obj);

  g1h->th_card_table()->inline_write_ref_field_gc((void *) p, obj, !g1h->is_in_young(cast_to_oop(heap_oop)));

  return false;
}

#endif // SHARE_GC_G1_G1FULLCOLLECTOR_INLINE_HPP

