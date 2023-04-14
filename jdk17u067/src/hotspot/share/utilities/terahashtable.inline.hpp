/*
 * Copyright (c) 2003, 2021, Oracle and/or its affiliates. All rights reserved.
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

#ifndef SHARE_UTILITIES_TERAHASHTABLE_INLINE_HPP
#define SHARE_UTILITIES_TERAHASHTABLE_INLINE_HPP

#include "utilities/terahashtable.hpp"

#include "memory/allocation.inline.hpp"
#include <tera_allocator.h>

// Inline function definitions for teraHashTable.hpp.

// --------------------------------------------------------------------------

// Initialize a table.

template <MEMFLAGS F> inline TeraBasicHashtable<F>::TeraBasicHashtable(int table_size, int entry_size) {
  // Called on startup, no locking needed
  initialize(table_size, entry_size, 0);
  //_buckets = NEW_C_HEAP_ARRAY(TeraHashtableBucket<F>, table_size, F);
  _buckets = (TeraHashtableBucket<F> *) tera_malloc(table_size * sizeof(TeraHashtableBucket<F>));
  for (int index = 0; index < _table_size; index++) {
    _buckets[index].clear();
  }
}


template <MEMFLAGS F> inline TeraBasicHashtable<F>::TeraBasicHashtable(int table_size, int entry_size,
                                      TeraHashtableBucket<F>* buckets,
                                      int number_of_entries) {

  // Called on startup, no locking needed
  initialize(table_size, entry_size, number_of_entries);
  _buckets = buckets;
}

template <MEMFLAGS F> inline TeraBasicHashtable<F>::~TeraBasicHashtable() {
  //free_buckets();
}

template <MEMFLAGS F> inline void TeraBasicHashtable<F>::initialize(int table_size, int entry_size,
                                       int number_of_entries) {
  // Called on startup, no locking needed
  _table_size = table_size;
  _entry_size = entry_size;
  _number_of_entries = number_of_entries;
}


// The following method is MT-safe and may be used with caution.
template <MEMFLAGS F> inline TeraBasicHashtableEntry<F>* TeraBasicHashtable<F>::bucket(int i) const {
  return _buckets[i].get_entry();
}

template <MEMFLAGS F> inline TeraBasicHashtableEntry<F>* TeraHashtableBucket<F>::get_entry() const {
  return _entry;
}

template <MEMFLAGS F> inline void TeraHashtableBucket<F>::set_entry(TeraBasicHashtableEntry<F>* l) {
  _entry = l;
}

template <MEMFLAGS F> inline void TeraBasicHashtable<F>::set_entry(int index, TeraBasicHashtableEntry<F>* entry) {
  _buckets[index].set_entry(entry);
}


template <MEMFLAGS F> inline void TeraBasicHashtable<F>::add_entry(int index, TeraBasicHashtableEntry<F>* entry) {
  entry->set_next(bucket(index));
  _buckets[index].set_entry(entry);
  ++_number_of_entries;
}

#endif // SHARE_GC_TERAHEAP_TERAHASHTABLE_INLINE_HPP
