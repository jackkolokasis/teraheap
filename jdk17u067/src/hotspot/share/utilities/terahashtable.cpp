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

#include "precompiled.hpp"
#include "classfile/dictionary.hpp"
#include "classfile/javaClasses.inline.hpp"
#include "classfile/placeholders.hpp"
#include "classfile/protectionDomainCache.hpp"
#include "classfile/vmClasses.hpp"
#include "memory/allocation.inline.hpp"
#include "memory/resourceArea.hpp"
#include "oops/oop.inline.hpp"
#include "oops/symbol.hpp"
#include "oops/weakHandle.inline.hpp"
#include "runtime/safepoint.hpp"
#include "utilities/terahashtable.inline.hpp"
#include "utilities/terahashtable.hpp"
#include "gc/teraHeap/teraHeap.hpp"
#include "gc/teraHeap/teraHeap.inline.hpp"

// This hashtable is implemented as an open hash table with a fixed number of buckets.

// Hashtable entry allocates in the C heap directly.

template <MEMFLAGS F> TeraBasicHashtableEntry<F>* TeraBasicHashtable<F>::new_entry(unsigned int hashValue) {
  TeraBasicHashtableEntry<F>* entry = ::new (Universe::teraHeap()->tera_dram_malloc(this->entry_size()))
                                        TeraBasicHashtableEntry<F>(hashValue);
  return entry;
}


template <class T, MEMFLAGS F> TeraHashtableEntry<T, F>* TeraHashtable<T, F>::new_entry(unsigned int hashValue, T obj) {
  TeraHashtableEntry<T, F>* entry = ::new (Universe::teraHeap()->tera_dram_malloc(this->entry_size()))
                                      TeraHashtableEntry<T, F>(hashValue, obj);
  return entry;
}

template <MEMFLAGS F> inline void TeraBasicHashtable<F>::free_entry(TeraBasicHashtableEntry<F>* entry) {
  // Unlink from the Hashtable prior to freeing
  unlink_entry(entry);
  FREE_C_HEAP_ARRAY(char, entry);
}


template <MEMFLAGS F> void TeraBasicHashtable<F>::free_buckets() {
  FREE_C_HEAP_ARRAY(TeraHashtableBucket, _buckets);
  _buckets = NULL;
}

const double _resize_factor    = 2.0;     // by how much we will resize using current number of entries
const int _small_table_sizes[] = { 107, 1009, 2017, 4049, 5051, 10103, 20201, 40423 } ;
const int _small_array_size = sizeof(_small_table_sizes)/sizeof(int);

// possible hashmap sizes - odd primes that roughly double in size.
// To avoid excessive resizing the odd primes from 4801-76831 and
// 76831-307261 have been removed.
const int _large_table_sizes[] =  { 4801, 76831, 307261, 614563, 1228891,
    2457733, 4915219, 9830479, 19660831, 39321619, 78643219 };
const int _large_array_size = sizeof(_large_table_sizes)/sizeof(int);

// Calculate next "good" hashtable size based on requested count
template <MEMFLAGS F> int TeraBasicHashtable<F>::calculate_resize(bool use_large_table_sizes) const {
  int requested = (int)(_resize_factor*number_of_entries());
  const int* primelist = use_large_table_sizes ? _large_table_sizes : _small_table_sizes;
  int arraysize =  use_large_table_sizes ? _large_array_size  : _small_array_size;
  int newsize;
  for (int i = 0; i < arraysize; i++) {
    newsize = primelist[i];
    if (newsize >= requested)
      break;
  }
  return newsize;
}

template <MEMFLAGS F> bool TeraBasicHashtable<F>::resize(int new_size) {

  // Allocate new buckets
  TeraHashtableBucket<F>* buckets_new = NEW_C_HEAP_ARRAY2_RETURN_NULL(TeraHashtableBucket<F>, new_size, F, CURRENT_PC);
  if (buckets_new == NULL) {
    return false;
  }

  // Clear the new buckets
  for (int i = 0; i < new_size; i++) {
    buckets_new[i].clear();
  }

  int table_size_old = _table_size;
  // hash_to_index() uses _table_size, so switch the sizes now
  _table_size = new_size;

  // Move entries from the old table to a new table
  for (int index_old = 0; index_old < table_size_old; index_old++) {
    for (TeraBasicHashtableEntry<F>* p = _buckets[index_old].get_entry(); p != NULL; ) {
      TeraBasicHashtableEntry<F>* next = p->next();
      int index_new = hash_to_index(p->hash());

      p->set_next(buckets_new[index_new].get_entry());
      buckets_new[index_new].set_entry(p);
      p = next;
    }
  }

  // The old backets now can be released
  TeraBasicHashtable<F>::free_buckets();

  // Switch to the new storage
  _buckets = buckets_new;

  return true;
}

template <MEMFLAGS F> bool TeraBasicHashtable<F>::maybe_grow(int max_size, int load_factor) {
  assert(SafepointSynchronize::is_at_safepoint(), "must be at safepoint");

  if (table_size() >= max_size) {
    return false;
  }
  if (number_of_entries() / table_size() > load_factor) {
    resize(MIN2<int>(table_size() * 2, max_size));
    return true;
  } else {
    return false;
  }
}

template class TeraBasicHashtable<mtNMT>;
template class TeraHashtable<HeapWord *, mtNMT>;
