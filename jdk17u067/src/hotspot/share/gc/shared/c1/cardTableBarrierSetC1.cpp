/*
 * Copyright (c) 2018, 2021, Oracle and/or its affiliates. All rights reserved.
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
#include "gc/shared/c1/cardTableBarrierSetC1.hpp"
#include "gc/shared/cardTable.hpp"
#include "gc/shared/cardTableBarrierSet.hpp"
#include "gc/shared/gc_globals.hpp"
#include "gc/teraHeap/teraHeap.hpp"
#include "utilities/macros.hpp"

#ifdef ASSERT
#define __ gen->lir(__FILE__, __LINE__)->
#else
#define __ gen->lir()->
#endif

void CardTableBarrierSetC1::post_barrier(LIRAccess& access, LIR_OprDesc* addr, LIR_OprDesc* new_val) {
  DecoratorSet decorators = access.decorators();
  LIRGenerator* gen = access.gen();
  bool in_heap = (decorators & IN_HEAP) != 0;
  if (!in_heap) {
    return;
  }

  BarrierSet* bs = BarrierSet::barrier_set();
  CardTableBarrierSet* ctbs = barrier_set_cast<CardTableBarrierSet>(bs);
  CardTable* ct = ctbs->card_table();
  LIR_Const* card_table_base = new LIR_Const(ct->byte_map_base());
	LIR_Opr heap_ct = gen->load_constant(card_table_base);

#ifdef TERA_C1
	// These registers are used for TeraCache and TeraCard tables
	LIR_Const* tera_card_table_base = NULL;
	LIR_Opr tera_ct = NULL;

  if (EnableTeraHeap) {
    tera_card_table_base = new LIR_Const(ct->th_byte_map_base());
    tera_ct = gen->load_constant(tera_card_table_base);
  }
#endif // TERA_C1

  if (addr->is_address()) {
    LIR_Address* address = addr->as_address_ptr();
    // ptr cannot be an object because we use this barrier for array card marks
    // and addr can point in the middle of an array.
    LIR_Opr ptr = gen->new_pointer_register();
    if (!address->index()->is_valid() && address->disp() == 0) {
      __ move(address->base(), ptr);
    } else {
      assert(address->disp() != max_jint, "lea doesn't support patched addresses!");
      __ leal(addr, ptr);
    }
    addr = ptr;
  }
  assert(addr->is_register(), "must be a register at this point");
  
  LabelObj* L = new LabelObj();
	LabelObj* M = new LabelObj();

#ifdef TERA_C1
	// Check if the object belongs to TeraCache or in the heap. If belongs to
	// TeraCache then jump to mark the tera card tables, otherwise continue to
	// mark the heap card tables.
	if (EnableTeraHeap) {
		LIR_Opr h2_start_addr = gen->new_register(T_LONG);
		__ move(LIR_OprFact::intptrConst((address)Universe::teraHeap()->h2_start_addr()), h2_start_addr);

		assert(h2_start_addr->is_register(), "must be a register at this point");
		assert(h2_start_addr->is_double_cpu(), "must be a single cpu");

		if (addr->is_single_cpu()) {
			LIR_Opr tmp_addr = gen->new_register(T_LONG);
			__ move(addr, tmp_addr);

			assert(tmp_addr->is_double_cpu(), "must be a single cpu");

			__ cmp(lir_cond_greaterEqual, tmp_addr, h2_start_addr);
			__ branch(lir_cond_greaterEqual, M->label());

		} 
		else if (addr->is_double_cpu()) {
			__ cmp(lir_cond_greaterEqual, addr, h2_start_addr);
			__ branch(lir_cond_greaterEqual, M->label());
		} 
		else 
			ShouldNotReachHere();
	}
#endif //TERA_C1

#ifdef CARDTABLEBARRIERSET_POST_BARRIER_HELPER
  gen->CardTableBarrierSet_post_barrier_helper(addr, card_table_base);
#else
  LIR_Opr tmp = gen->new_pointer_register();
  if (TwoOperandLIRForm) {
    __ move(addr, tmp);
    __ unsigned_shift_right(tmp, CardTable::card_shift, tmp);
  } else {
    __ unsigned_shift_right(addr, CardTable::card_shift, tmp);
  }

  LIR_Address* card_addr;
  if (gen->can_inline_as_constant(card_table_base)) {
    card_addr = new LIR_Address(tmp, card_table_base->as_jint(), T_BYTE);
  } else {
    card_addr = new LIR_Address(tmp, heap_ct, T_BYTE);
  }

  LIR_Opr dirty = LIR_OprFact::intConst(CardTable::dirty_card_val());
  if (UseCondCardMark) {
    LIR_Opr cur_value = gen->new_register(T_INT);
    __ move(card_addr, cur_value);

    LabelObj* L_already_dirty = new LabelObj();
    __ cmp(lir_cond_equal, cur_value, dirty);
    __ branch(lir_cond_equal, L_already_dirty->label());
    __ move(dirty, card_addr);
    __ branch_destination(L_already_dirty->label());
  } else {
    __ move(dirty, card_addr);
  }

#ifdef TERA_C1
	// Mark teracache card tables
	if (EnableTeraHeap) {

		__ branch(lir_cond_always, L->label());

		__ branch_destination(M->label());

		LIR_Opr tmp_reg = gen->new_pointer_register();

		if (TwoOperandLIRForm) {
			__ move(addr, tmp_reg);
			__ unsigned_shift_right(tmp_reg, CardTable::th_card_shift, tmp_reg);
		} else {
			__ unsigned_shift_right(addr, CardTable::th_card_shift, tmp_reg);
		}
  
    LIR_Address* th_card_addr;
    if (gen->can_inline_as_constant(tera_card_table_base)) {
      th_card_addr = new LIR_Address(tmp_reg, tera_card_table_base->as_jint(), T_BYTE);
    } else {
      th_card_addr = new LIR_Address(tmp_reg, tera_ct, T_BYTE);
    }

  LIR_Opr dirty = LIR_OprFact::intConst(CardTable::dirty_card_val());
  if (UseCondCardMark) {
    LIR_Opr th_cur_value = gen->new_register(T_INT);
    __ move(th_card_addr, th_cur_value);

    LabelObj* L_th_already_dirty = new LabelObj();
    __ cmp(lir_cond_equal, th_cur_value, dirty);
    __ branch(lir_cond_equal, L_th_already_dirty->label());
    __ move(dirty, th_card_addr);
    __ branch_destination(L_th_already_dirty->label());
  } else {
    __ move(dirty, th_card_addr);
  }

		__ branch_destination(L->label());
	}
#endif // TERA_C1
#endif // CARDTABLEBARRIERSET_POST_BARRIER_HELPER
}
