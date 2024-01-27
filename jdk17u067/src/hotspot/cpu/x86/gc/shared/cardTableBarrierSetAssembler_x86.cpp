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
#include "asm/macroAssembler.inline.hpp"
#include "gc/shared/barrierSet.hpp"
#include "gc/shared/cardTable.hpp"
#include "gc/shared/cardTableBarrierSet.hpp"
#include "gc/shared/cardTableBarrierSetAssembler.hpp"
#include "gc/shared/gc_globals.hpp"
#include "gc/teraHeap/teraHeap.hpp"

#define __ masm->

#ifdef PRODUCT
#define BLOCK_COMMENT(str) /* nothing */
#else
#define BLOCK_COMMENT(str) __ block_comment(str)
#endif

#define BIND(label) bind(label); BLOCK_COMMENT(#label ":")

#define TIMES_OOP (UseCompressedOops ? Address::times_4 : Address::times_8)

void CardTableBarrierSetAssembler::gen_write_ref_array_post_barrier(MacroAssembler* masm, DecoratorSet decorators,
                                                                    Register addr, Register count, Register tmp) {
  BarrierSet *bs = BarrierSet::barrier_set();
  CardTableBarrierSet* ctbs = barrier_set_cast<CardTableBarrierSet>(bs);
  CardTable* ct = ctbs->card_table();

#ifdef TERA_INTERPRETER
  Label L_loop, L_done;
  const Register end = count;
  assert_different_registers(addr, end);

  __ testl(count, count);
  __ jcc(Assembler::zero, L_done); // zero count - nothing to do

  if (EnableTeraHeap) {
    Label L_h2_done, L_h2;

    __ leaq(end, Address(addr, count, TIMES_OOP, 0));  // end == addr+count*oop_size
    __ subptr(end, BytesPerHeapOop); // end - 1 to make inclusive
        
    // Get h2 start address
    AddressLiteral h2_start_addr((address)Universe::teraHeap()->h2_start_addr(), relocInfo::none);
    __ lea(tmp, h2_start_addr);
        
    // Check if array is in h1 or h2 
    __ cmpptr(addr, tmp);
    __ jcc(Assembler::greaterEqual, L_h2);
  
    __ shrptr(addr, CardTable::card_shift);
    __ shrptr(end, CardTable::card_shift);
    __ subptr(end, addr); // end --> cards count
  
    intptr_t disp = (intptr_t) ct->byte_map_base();
    __ mov64(tmp, disp);
    __ addptr(addr, tmp);
    __ jmp(L_h2_done);

    __ BIND(L_h2);
    
    __ shrptr(addr, CardTable::th_card_shift);
    __ shrptr(end, CardTable::th_card_shift);
    __ subptr(end, addr); // end --> cards count

    intptr_t th_disp = (intptr_t) ct->th_byte_map_base();
    __ mov64(tmp, th_disp);
    __ addptr(addr, tmp);

    __ BIND(L_h2_done);
    __ BIND(L_loop);
    __ movb(Address(addr, count, Address::times_1), 0);
    __ decrement(count);
    __ jcc(Assembler::greaterEqual, L_loop);
  } else {
    intptr_t disp = (intptr_t) ct->byte_map_base();

#ifdef _LP64
    __ leaq(end, Address(addr, count, TIMES_OOP, 0));  // end == addr+count*oop_size
    __ subptr(end, BytesPerHeapOop); // end - 1 to make inclusive
    __ shrptr(addr, CardTable::card_shift);
    __ shrptr(end, CardTable::card_shift);
    __ subptr(end, addr); // end --> cards count

    __ mov64(tmp, disp);
    __ addptr(addr, tmp);
    __ BIND(L_loop);
    __ movb(Address(addr, count, Address::times_1), 0);
    __ decrement(count);
    __ jcc(Assembler::greaterEqual, L_loop);
#else
    __ lea(end,  Address(addr, count, Address::times_ptr, -wordSize));
    __ shrptr(addr, CardTable::card_shift);
    __ shrptr(end,   CardTable::card_shift);
    __ subptr(end, addr); // end --> count
    __ BIND(L_loop);
    Address cardtable(addr, count, Address::times_1, disp);
    __ movb(cardtable, 0);
    __ decrement(count);
    __ jcc(Assembler::greaterEqual, L_loop);
#endif // _LP64
  }
  __ BIND(L_done);
#else
    Label L_loop, L_done;
    const Register end = count;
    assert_different_registers(addr, end);
    intptr_t disp = (intptr_t) ct->byte_map_base();

    __ testl(count, count);
    __ jcc(Assembler::zero, L_done); // zero count - nothing to do

#ifdef _LP64
    __ leaq(end, Address(addr, count, TIMES_OOP, 0));  // end == addr+count*oop_size
    __ subptr(end, BytesPerHeapOop); // end - 1 to make inclusive
    __ shrptr(addr, CardTable::card_shift);
    __ shrptr(end, CardTable::card_shift);
    __ subptr(end, addr); // end --> cards count

    __ mov64(tmp, disp);
    __ addptr(addr, tmp);
    __ BIND(L_loop);
    __ movb(Address(addr, count, Address::times_1), 0);
    __ decrement(count);
    __ jcc(Assembler::greaterEqual, L_loop);
#else
    __ lea(end,  Address(addr, count, Address::times_ptr, -wordSize));
    __ shrptr(addr, CardTable::card_shift);
    __ shrptr(end,   CardTable::card_shift);
    __ subptr(end, addr); // end --> count
    __ BIND(L_loop);
    Address cardtable(addr, count, Address::times_1, disp);
    __ movb(cardtable, 0);
    __ decrement(count);
    __ jcc(Assembler::greaterEqual, L_loop);
#endif // _LP64
    __ BIND(L_done);
#endif // TERA_INTERPRETER
}

#ifdef TERA_INTERPRETER
void CardTableBarrierSetAssembler::store_check(MacroAssembler* masm, Register obj, Address dst) { 
  Label L_in_h2;
	Label L_Done;

  if (EnableTeraHeap) {
    AddressLiteral h2_start_addr((address)Universe::teraHeap()->h2_start_addr(), relocInfo::none);
    // Push the teraCache address in r11
    __ lea(r11, h2_start_addr);
		
    __ cmpptr(obj, r11); 
		
		__ jcc(Assembler::greaterEqual, L_in_h2);
    
    // Does a store check for the oop in register obj. The content of
    // register obj is destroyed afterwards.
		store_check_part1(masm, obj);
		store_check_part2(masm, obj);
		__ jmp(L_Done);
		
    __ BIND(L_in_h2);
    h2_store_check_part1(masm, obj);
		h2_store_check_part2(masm, obj);

		__ BIND(L_Done);
  } else {
    store_check_part1(masm, obj);
    store_check_part2(masm, obj);
  }
}
#else 
void CardTableBarrierSetAssembler::store_check(MacroAssembler* masm, Register obj, Address dst) {
  // Does a store check for the oop in register obj. The content of
  // register obj is destroyed afterwards.
  BarrierSet* bs = BarrierSet::barrier_set();

  CardTableBarrierSet* ctbs = barrier_set_cast<CardTableBarrierSet>(bs);
  CardTable* ct = ctbs->card_table();
  __ shrptr(obj, CardTable::card_shift);

  Address card_addr;

  // The calculation for byte_map_base is as follows:
  // byte_map_base = _byte_map - (uintptr_t(low_bound) >> card_shift);
  // So this essentially converts an address to a displacement and it will
  // never need to be relocated. On 64bit however the value may be too
  // large for a 32bit displacement.
  intptr_t byte_map_base = (intptr_t)ct->byte_map_base();
  if (__ is_simm32(byte_map_base)) {
    card_addr = Address(noreg, obj, Address::times_1, byte_map_base);
  } else {
    // By doing it as an ExternalAddress 'byte_map_base' could be converted to a rip-relative
    // displacement and done in a single instruction given favorable mapping and a
    // smarter version of as_Address. However, 'ExternalAddress' generates a relocation
    // entry and that entry is not properly handled by the relocation code.
    AddressLiteral cardtable((address)byte_map_base, relocInfo::none);
    Address index(noreg, obj, Address::times_1);
    card_addr = __ as_Address(ArrayAddress(cardtable, index));
  }

  int dirty = CardTable::dirty_card_val();
  if (UseCondCardMark) {
    Label L_already_dirty;
    __ cmpb(card_addr, dirty);
    __ jcc(Assembler::equal, L_already_dirty);
    __ movb(card_addr, dirty);
    __ bind(L_already_dirty);
  } else {
    __ movb(card_addr, dirty);
  }
}
#endif // TERA_INTERPRETER

#ifdef TERA_INTERPRETER
void CardTableBarrierSetAssembler::store_check_part1(MacroAssembler* masm, Register obj) {
    __ shrptr(obj, CardTable::card_shift);
}

void CardTableBarrierSetAssembler::h2_store_check_part1(MacroAssembler* masm, Register obj) {
    __ shrptr(obj, CardTable::th_card_shift);
}

void CardTableBarrierSetAssembler::store_check_part2(MacroAssembler* masm, Register obj) {
  // Does a store check for the oop in register obj. The content of
  // register obj is destroyed afterwards.
  BarrierSet* bs = BarrierSet::barrier_set();

  CardTableBarrierSet* ctbs = barrier_set_cast<CardTableBarrierSet>(bs);
  CardTable* ct = ctbs->card_table();
  Address card_addr;

  // The calculation for byte_map_base is as follows:
  // byte_map_base = _byte_map - (uintptr_t(low_bound) >> card_shift);
  // So this essentially converts an address to a displacement and it will
  // never need to be relocated. On 64bit however the value may be too
  // large for a 32bit displacement.
  intptr_t byte_map_base = (intptr_t)ct->byte_map_base();
  if (__ is_simm32(byte_map_base)) {
    card_addr = Address(noreg, obj, Address::times_1, byte_map_base);
  } else {
    // By doing it as an ExternalAddress 'byte_map_base' could be converted to a rip-relative
    // displacement and done in a single instruction given favorable mapping and a
    // smarter version of as_Address. However, 'ExternalAddress' generates a relocation
    // entry and that entry is not properly handled by the relocation code.
    AddressLiteral cardtable((address)byte_map_base, relocInfo::none);
    Address index(noreg, obj, Address::times_1);
    card_addr = __ as_Address(ArrayAddress(cardtable, index));
  }

  int dirty = CardTable::dirty_card_val();
  if (UseCondCardMark) {
    Label L_already_dirty;
    __ cmpb(card_addr, dirty);
    __ jcc(Assembler::equal, L_already_dirty);
    __ movb(card_addr, dirty);
    __ bind(L_already_dirty);
  } else {
    __ movb(card_addr, dirty);
  }
}

void CardTableBarrierSetAssembler::h2_store_check_part2(MacroAssembler* masm, Register obj) {
  // Does a store check for the oop in register obj. The content of
  // register obj is destroyed afterwards.
  BarrierSet* bs = BarrierSet::barrier_set();

  CardTableBarrierSet* ctbs = barrier_set_cast<CardTableBarrierSet>(bs);
  CardTable* ct = ctbs->card_table();
  Address card_addr;

  // The calculation for byte_map_base is as follows:
  // byte_map_base = _byte_map - (uintptr_t(low_bound) >> card_shift);
  // So this essentially converts an address to a displacement and it will
  // never need to be relocated. On 64bit however the value may be too
  // large for a 32bit displacement.
  intptr_t th_byte_map_base = (intptr_t)ct->th_byte_map_base();
  if (__ is_simm32(th_byte_map_base)) {
    card_addr = Address(noreg, obj, Address::times_1, th_byte_map_base);
  } else {
    // By doing it as an ExternalAddress 'byte_map_base' could be converted to a rip-relative
    // displacement and done in a single instruction given favorable mapping and a
    // smarter version of as_Address. However, 'ExternalAddress' generates a relocation
    // entry and that entry is not properly handled by the relocation code.
    AddressLiteral cardtable((address)th_byte_map_base, relocInfo::none);
    Address index(noreg, obj, Address::times_1);
    card_addr = __ as_Address(ArrayAddress(cardtable, index));
  }

  int dirty = CardTable::dirty_card_val();
  if (UseCondCardMark) {
    Label L_already_dirty;
    __ cmpb(card_addr, dirty);
    __ jcc(Assembler::equal, L_already_dirty);
    __ movb(card_addr, dirty);
    __ bind(L_already_dirty);
  } else {
    __ movb(card_addr, dirty);
  }
}

#endif // TERA_INTERPRETER

void CardTableBarrierSetAssembler::oop_store_at(MacroAssembler* masm, DecoratorSet decorators, BasicType type,
                                                Address dst, Register val, Register tmp1, Register tmp2) {
  bool in_heap = (decorators & IN_HEAP) != 0;

  bool is_array = (decorators & IS_ARRAY) != 0;
  bool on_anonymous = (decorators & ON_UNKNOWN_OOP_REF) != 0;
  bool precise = is_array || on_anonymous;

  bool needs_post_barrier = val != noreg && in_heap;

  BarrierSetAssembler::store_at(masm, decorators, type, dst, val, noreg, noreg);
  if (needs_post_barrier) {
    // flatten object address if needed
    if (!precise || (dst.index() == noreg && dst.disp() == 0)) {
      store_check(masm, dst.base(), dst);
    } else {
      __ lea(tmp1, dst);
      store_check(masm, tmp1, dst);
    }
  }
}
