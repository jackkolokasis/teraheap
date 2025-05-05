/*
 * Copyright (c) 2018, 2020, Oracle and/or its affiliates. All rights reserved.
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
#include "gc/g1/g1BarrierSet.inline.hpp"
#include "gc/g1/g1BarrierSetRuntime.hpp"
#include "gc/g1/g1ThreadLocalData.hpp"
#include "runtime/interfaceSupport.inline.hpp"
#include "utilities/macros.hpp"

void G1BarrierSetRuntime::write_ref_array_pre_oop_entry(oop* dst, size_t length) {
  G1BarrierSet *bs = barrier_set_cast<G1BarrierSet>(BarrierSet::barrier_set());
  bs->write_ref_array_pre(dst, length, false);
}

void G1BarrierSetRuntime::write_ref_array_pre_narrow_oop_entry(narrowOop* dst, size_t length) {
  G1BarrierSet *bs = barrier_set_cast<G1BarrierSet>(BarrierSet::barrier_set());
  bs->write_ref_array_pre(dst, length, false);
}

void G1BarrierSetRuntime::write_ref_array_post_entry(HeapWord* dst, size_t length) {
  G1BarrierSet *bs = barrier_set_cast<G1BarrierSet>(BarrierSet::barrier_set());
  bs->G1BarrierSet::write_ref_array(dst, length);
}

// G1 pre write barrier slowpath
JRT_LEAF(void, G1BarrierSetRuntime::write_ref_field_pre_entry(oopDesc* orig, JavaThread* thread))
  assert(orig != nullptr, "should be optimized out");
  assert(oopDesc::is_oop(orig, true /* ignore mark word */), "Error");
  // store the original value that was in the field reference
  SATBMarkQueue& queue = G1ThreadLocalData::satb_mark_queue(thread);
  G1BarrierSet::satb_mark_queue_set().enqueue(queue, orig);
JRT_END

// G1 post write barrier slowpath
JRT_LEAF(void, G1BarrierSetRuntime::write_ref_field_post_entry(volatile G1CardTable::CardValue* card_addr,
                                                               JavaThread* thread))

  DEBUG_ONLY(
    if( EnableTeraHeap ){
    BarrierSet *bs = BarrierSet::barrier_set();
    CardTableBarrierSet* ctbs = barrier_set_cast<CardTableBarrierSet>(bs);
    assert(Universe::heap()->is_in(ctbs->card_table()->addr_for((CardValue*)card_addr) ) , "should not be called at an h2 card" );
  })

  G1DirtyCardQueue& queue = G1ThreadLocalData::dirty_card_queue(thread);
  G1BarrierSet::dirty_card_queue_set().enqueue(queue, card_addr);
JRT_END


#ifdef TERA_C2
JRT_LEAF(void, G1BarrierSetRuntime::h2_wb_post(void* obj))
  BarrierSet* bs = BarrierSet::barrier_set();
  CardTableBarrierSet* ctbs = barrier_set_cast<CardTableBarrierSet>(bs);
  CardTable* ct = ctbs->th_card_table();

#ifdef C2_ONLY_LEAF_CALL
  assert(ct->th_byte_map_base() != NULL, "TeraHeap card table is NULL");
  assert(ct->byte_map_base() != NULL, "Heap card table is NULL");
  assert(Universe::teraHeap()->is_field_in_h2(obj) || Universe::heap()->is_in(obj),
        "Objects is out of reserved space %p", (HeapWord *) obj);

	if (Universe::teraHeap()->is_field_in_h2(obj))
		ct->th_byte_map_base()[uintptr_t(obj) >> CardTable::th_card_shift] = CardTable::dirty_card_val();
	else
		ct->byte_map_base()[uintptr_t(obj) >> CardTable::card_shift] = CardTable::dirty_card_val();

#else

  assert(sizeof(*ct->th_byte_map_base()) == sizeof(jbyte), "adjust users of this code");
  assert(ct->th_byte_map_base() != NULL, "TeraCache card table is NULL");
  assert(Universe::teraHeap()->is_field_in_h2(obj), "Objects is out of reserved space %p | is in H1 = %d", 
         (HeapWord*)obj, Universe::heap()->is_in(obj));
		
	ct->th_byte_map_base()[uintptr_t(obj) >> CardTable::th_card_shift] = CardTable::dirty_card_val();

#endif // C2_ONLY_LEAF_CALL
	JRT_END
#endif // TERA_C2

