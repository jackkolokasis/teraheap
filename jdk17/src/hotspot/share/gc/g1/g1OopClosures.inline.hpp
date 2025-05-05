/*
 * Copyright (c) 2001, 2020, Oracle and/or its affiliates. All rights reserved.
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

#ifndef SHARE_GC_G1_G1OOPCLOSURES_INLINE_HPP
#define SHARE_GC_G1_G1OOPCLOSURES_INLINE_HPP

#include "gc/g1/g1OopClosures.hpp"

#include "gc/g1/g1CollectedHeap.hpp"
#include "gc/g1/g1FullCollector.inline.hpp"
#include "gc/g1/g1ConcurrentMark.inline.hpp"
#include "gc/g1/g1ParScanThreadState.inline.hpp"
#include "gc/g1/g1RemSet.hpp"
#include "gc/g1/heapRegion.inline.hpp"
#include "gc/g1/heapRegionRemSet.hpp"
#include "memory/iterator.inline.hpp"
#include "oops/access.inline.hpp"
#include "oops/compressedOops.inline.hpp"
#include "oops/oopsHierarchy.hpp"
#include "oops/oop.inline.hpp"
#include "runtime/prefetch.inline.hpp"
#include "utilities/align.hpp"

template <class T>
inline void G1ScanClosureBase::prefetch_and_push(T* p, const oop obj) {
   
#ifdef TERA_ASSERT
      //assert( obj is not in H2 )
      DEBUG_ONLY( if(EnableTeraHeap) assert( !Universe::is_in_h2(obj) , "H2 objects should have been filtered out"); )
#endif
  
  // We're not going to even bother checking whether the object is
  // already forwarded or not, as this usually causes an immediate
  // stall. We'll try to prefetch the object (for write, given that
  // we might need to install the forwarding reference) and we'll
  // get back to it when pop it from the queue
  Prefetch::write(obj->mark_addr(), 0);
  Prefetch::read(obj->mark_addr(), (HeapWordSize*2));

  // slightly paranoid test; I'm trying to catch potential
  // problems before we go into push_on_queue to know where the
  // problem is coming from
  assert((obj == RawAccess<>::oop_load(p)) ||
         (obj->is_forwarded() &&
         obj->forwardee() == RawAccess<>::oop_load(p)),
         "p should still be pointing to obj or to its forwardee");

  _par_scan_state->push_on_queue(ScannerTask(p));
}

template <class T>
inline void G1ScanClosureBase::handle_non_cset_obj_common(G1HeapRegionAttr const region_attr, T* p, oop const obj) {
  if (region_attr.is_humongous() ) {
    _g1h->set_humongous_is_live(obj);
  } else if (region_attr.is_optional()) {
    _par_scan_state->remember_reference_into_optional_region(p);
  }
}

#ifdef TERA_CARDS
template <class T>
inline void G1ScanClosureBase::handle_non_cset_obj_common_tera(G1HeapRegionAttr const region_attr, T* p, oop const obj) {
  assert(EnableTeraHeap 
  && Universe::is_field_in_h2((void*) p)
  && !Universe::is_in_h2(obj)  , "Sanity check");

  assert(!_g1h->card_table()->is_in_young(obj) , "obj should be in old");

  //h2->h1
  //back ref found: update h2 card table flag
  _g1h->th_card_table()->inline_write_ref_field_gc((void*) p, obj, true ); 
  // if h1 obj is in opt cset, remember
  handle_non_cset_obj_common(region_attr,p,obj);
}
#endif

inline void G1ScanClosureBase::trim_queue_partially() {
  _par_scan_state->trim_queue_partially();
}


//an object was evacuated in h1 and we are now scanning its fields
//*p is the field of the newly evacuated object
// p(h1) -> obj (h1 or h2)
template <class T>
inline void G1ScanEvacuatedObjClosure::do_oop_work(T* p) {
  T heap_oop = RawAccess<>::oop_load(p);

  if (CompressedOops::is_null(heap_oop)) {
    return;
  }
  
  oop obj = CompressedOops::decode_not_null(heap_oop);
  assert(  !Universe::is_field_in_h2(p) , "Parent is an h2 obj. wrong closure");


#ifdef TERA_MAINTENANCE
  // h1->h2 : Fence
  if (EnableTeraHeap && (Universe::is_in_h2(obj))) return;
#endif

  

  const G1HeapRegionAttr region_attr = _g1h->region_attr(obj);
  if (region_attr.is_in_cset()) {
    // h1 -> h1 (in the cset)    
    prefetch_and_push(p, obj);

  } else if (!HeapRegion::is_in_same_region(p, obj)) {
    
    // h1->h1 (out of cset)
    handle_non_cset_obj_common(region_attr, p, obj);
    
    assert(_scanning_in_young != Uninitialized, "Scan location has not been initialized.");
    
    // p (ref of already evac object) points to -->  obj
    // _scanning_in_young == True  :  evac obj (young) --> obj (old/young)
    if (_scanning_in_young == True) {
      return;
    }

    // _scanning_in_young == False  : evac obj (old) --> obj (old/young)
    // rem set of obj-region needs to be updated
    // bcs RemSets only hold infos about old->young , old->old  incoming ptrs
    _par_scan_state->enqueue_card_if_tracked(region_attr, p, obj);
  }
}


#ifdef TERA_EVAC_MOVE
//an object was found that needs to be evacuated in h2, and we are now scanning its fields
//*p is the field of the h2 obj
// p(just evacuated in h2) -> obj (h1 or h2)
template <class T>
inline void ScanH2ObjClosure::do_oop_work(T* p) {
  T heap_oop = RawAccess<>::oop_load(p);

  if (CompressedOops::is_null(heap_oop)) {
    return;
  }
  
  oop obj = CompressedOops::decode_not_null(heap_oop);

  assert( Universe::is_field_in_h2(p) , "Parent must be an h2 obj. wrong closure");


  // h2->h2 : Fence, Update dependency list
  if (EnableTeraHeap && (Universe::is_in_h2(obj))){
    Universe::teraHeap()->group_regions((HeapWord *)p, cast_from_oop<HeapWord*>(obj));
    return;
  }
  

  //HERE : h2 -> (h1 in or out the cset)
  if(TeraHeapStatistics)
    Universe::teraHeap()->get_tera_stats()->add_back_ref();


  const G1HeapRegionAttr region_attr = _g1h->region_attr(obj);
  if (region_attr.is_in_cset()) {
    prefetch_and_push(p, obj); 
  } else {
    
#ifdef TERA_CARDS
    // h2 -> (h1 out of cset) : Update h2 card table flag
   handle_non_cset_obj_common_tera(region_attr, p, obj);      
#endif
   
  }
}
#endif


template <class T>
inline void G1CMOopClosure::do_oop_work(T* p) {  
  _task->deal_with_reference(p);
}


template <class T>
inline void G1RootRegionScanClosure::do_oop_work(T* p) {
  T heap_oop = RawAccess<MO_RELAXED>::oop_load(p);
  if (CompressedOops::is_null(heap_oop)) {
    return;
  }
  oop obj = CompressedOops::decode_not_null(heap_oop);


#ifdef TERA_MAINTENANCE     
      // the root regions are iterated, and every oop found in them
      // it is also iterated by oop_iterate.
      // So here we find all the outgoing pointers, from the root regions.
      // An outgoing pointer may be in H1 old gen or in H2

      //If obj is in H2
      //  (1) set H2 region live bit
      //  (2) Fence heap traversal to H2
      if (EnableTeraHeap && (Universe::is_in_h2(obj))){    
        Universe::teraHeap()->mark_used_region(cast_from_oop<HeapWord*>(obj));
        return;
      }
#endif

#ifdef TERA_CONC_MARKING  
  // if parent object has tera flag enabled
  // enable tera flag at child too
  if( EnableTeraHeap && _is_tera_enabled && !Universe::teraHeap()->is_metadata(obj)) 
    obj->mark_move_h2( _rdd_id, _part_id);
#endif

  _cm->mark_in_next_bitmap(_worker_id, obj); // mark only the objs that are bellow TAMPs
                                             // young regions have bottom == TAMPs
}

template <class T>
inline static void check_obj_during_refinement(T* p, oop const obj) {
#ifdef ASSERT

#ifdef TERA_ASSERT
  // p->obj 
  // p is found during dirty card scanning
  // it may point to an obj in H2 (H1->H2)
  // but this should already be filtered out by the calling function
  DEBUG_ONLY( if(EnableTeraHeap) assert( !Universe::is_in_h2(obj) , "H2 objects should have been filtered out"); )
#endif
  

  G1CollectedHeap* g1h = G1CollectedHeap::heap();
  // can't do because of races
  // assert(oopDesc::is_oop_or_null(obj), "expected an oop");
  assert(is_object_aligned(obj), "oop must be aligned");
  assert(g1h->is_in_reserved(obj), "oop must be in reserved");

  HeapRegion* from = g1h->heap_region_containing(p);

  assert(from != NULL, "from region must be non-NULL");
  assert(from->is_in_reserved(p) ||
         (from->is_humongous() &&
          g1h->heap_region_containing(p)->is_humongous() &&
          from->humongous_start_region() == g1h->heap_region_containing(p)->humongous_start_region()),
         "p " PTR_FORMAT " is not in the same region %u or part of the correct humongous object starting at region %u.",
         p2i(p), from->hrm_index(), from->humongous_start_region()->hrm_index());
#endif // ASSERT
}

template <class T>
inline void G1ConcurrentRefineOopClosure::do_oop_work(T* p) {
  T o = RawAccess<MO_RELAXED>::oop_load(p);
  if (CompressedOops::is_null(o)) {
    return;
  }
  oop obj = CompressedOops::decode_not_null(o);

#ifdef TERA_MAINTENANCE
  if (EnableTeraHeap && (Universe::is_in_h2(obj))){
    return;
  }
#endif

  check_obj_during_refinement(p, obj);

  if (HeapRegion::is_in_same_region(p, obj)) {
    // Normally this closure should only be called with cross-region references.
    // But since Java threads are manipulating the references concurrently and we
    // reload the values things may have changed.
    // Also this check lets slip through references from a humongous continues region
    // to its humongous start region, as they are in different regions, and adds a
    // remembered set entry. This is benign (apart from memory usage), as we never
    // try to either evacuate or eager reclaim humonguous arrays of j.l.O.
    return;
  }

  HeapRegionRemSet* to_rem_set = _g1h->heap_region_containing(obj)->rem_set();

  assert(to_rem_set != NULL, "Need per-region 'into' remsets.");
  if (to_rem_set->is_tracked()) {
    to_rem_set->add_reference(p, _worker_id);
  }
}

template <class T>
// in initial cset evacuation
// p-> obj  : h1 -> h1/h2
// *p is always in h1 bcs we are scanning h1 region cards 
// in optional cset evacuation
// h1/h2 -> h1
// where *p are the references pointing in the opt cset, and obj is in the opt region that now is in the cset
inline void G1ScanCardClosure::do_oop_work(T* p) {

  T o = RawAccess<>::oop_load(p);
  if (CompressedOops::is_null(o)) {
    return;
  }
  oop obj = CompressedOops::decode_not_null(o);

  // If obj is in H2
  //  (1) set H2 region live bit 
  //  (2) Fence heap traversal to H2
#ifdef TERA_MAINTENANCE
  if (EnableTeraHeap){
    if ( Universe::is_in_h2(obj) ) return;
  }

  if( !Universe::is_field_in_h2((void*) p) )    
    check_obj_during_refinement(p, obj);
  
  assert(Universe::is_field_in_h2((void*) p) || !_g1h->is_in_cset((HeapWord*)p),
      "Oop originates from " PTR_FORMAT " (region: %u) which is in the collection set.",
      p2i(p), _g1h->addr_to_region((HeapWord*)p));  
  
#elif 
  check_obj_during_refinement(p, obj);

  assert(!_g1h->is_in_cset((HeapWord*)p),
        "Oop originates from " PTR_FORMAT " (region: %u) which is in the collection set.",
        p2i(p), _g1h->addr_to_region((HeapWord*)p));
#endif  

  const G1HeapRegionAttr region_attr = _g1h->region_attr(obj);
  if (region_attr.is_in_cset()) {
    // Since the source is always from outside the collection set, here we implicitly know
    // that this is a cross-region reference too.
    prefetch_and_push(p, obj);
  } else if (!HeapRegion::is_in_same_region(p, obj)) {

    handle_non_cset_obj_common(region_attr, p, obj);
    _par_scan_state->enqueue_card_if_tracked(region_attr, p, obj);
  }
}


#ifdef TERA_CARDS

// p(h2) -> obj (h2/h1)
// h2->h2 : update dependency list
// h2-> (h1 in cset)  : enable tera flag && push p in queue
// h2-> (h1 out cset) : update h2 card table flag && enable tera flag && mark the obj as live
template <class T>
inline void H2ToH1Closure::do_oop_work(T* p) {
  // TODO: propably move this code to a separate closure
  if (in_full_gc) {
    // During full GC we scan all the H2 cards to find the backward references and put them
    // in the stacks to process them later.
    G1FullCollector::h2_should_trace(p);
    return;
  }

  T o = RawAccess<>::oop_load(p);
  if (CompressedOops::is_null(o)) {
    return;
  }
  oop obj = CompressedOops::decode_not_null(o);
  

  // h2->h2
  if (Universe::is_in_h2(obj)) {

    Universe::teraHeap()->group_regions((HeapWord *)p, cast_from_oop<HeapWord*>(obj));
    if( should_mark ) Universe::teraHeap()->mark_used_region(cast_from_oop<HeapWord*>(obj));
    
		return;	
  }

  const G1HeapRegionAttr region_attr = _g1h->region_attr(obj);
  
  if (region_attr.is_in_cset()) {
    // h2->h1 (in cset)   
    prefetch_and_push(p, obj);
   
  }else{
    // h2->h1 (out of cset)
    // meaning obj is not in young (bcs its excluded from the cset)    
    handle_non_cset_obj_common_tera(region_attr, p, obj);
	
    if( should_mark ){
      enable_tera_flag( (void*) p, obj);
      mark_object(obj);
    }
      
  }
}

void H2ToH1Closure::mark_object(oop obj) {
  assert(!_g1h->heap_region_containing(obj)->in_collection_set(), "should not mark objects in the CSet");
  // TODO: enable this assertion when we mark backrefs to transfer in H2.
  // assert(obj->is_marked_move_h2() , "The back ref should have already enable the tera flag of the H1 obj");
  // mark it as live
  _cm->mark_in_next_bitmap(_worker_id, obj);
}

void H2ToH1Closure::enable_tera_flag(void *p, oop obj){
   // enable its tera flag
  if ( !obj->is_marked_move_h2() && !Universe::teraHeap()->is_metadata(obj) ) {   
    obj->mark_move_h2(Universe::teraHeap()->h2_get_region_groupId(p),
                      Universe::teraHeap()->h2_get_region_partId(p));
  }
}
#endif


template <class T>
// deals with all the references pointing into the optional region
// those references can be h1 or h2, and point in the h1 opt region.
// h1/h2 -> h1
inline void G1ScanRSForOptionalClosure::do_oop_work(T* p) {

#ifdef TERA_MAINTENANCE
  // h2->h1
  if ( EnableTeraHeap && Universe::is_field_in_h2((void*) p) ) {
    _scan_cl->do_oop_work(p);
    _scan_cl->trim_queue_partially();
    return;
  }
#endif

  // h1->h1

  const G1HeapRegionAttr region_attr = _g1h->region_attr(p);
  // Entries in the optional collection set may start to originate from the collection
  // set after one or more increments. In this case, previously optional regions
  // became actual collection set regions. Filter them out here.
  if (region_attr.is_in_cset()) {
    return;
  }
  _scan_cl->do_oop_work(p);
  _scan_cl->trim_queue_partially();
}

void G1ParCopyHelper::do_cld_barrier(oop new_obj) {

#ifdef TERA_MAINTENANCE
  if( EnableTeraHeap && Universe::is_in_h2(new_obj) ) return;
#endif

  if (_g1h->heap_region_containing(new_obj)->is_young()) {
    _scanned_cld->record_modified_oops();
  }
}

void G1ParCopyHelper::mark_object(oop obj) {
  assert(!_g1h->heap_region_containing(obj)->in_collection_set(), "should not mark objects in the CSet");

  // We know that the object is not moving so it's safe to read its size.
  _cm->mark_in_next_bitmap(_worker_id, obj);
}

void G1ParCopyHelper::trim_queue_partially() {
  _par_scan_state->trim_queue_partially();
}

template <G1Barrier barrier, bool should_mark>
template <class T>
// root -> h1/h2
// root is in stack frames etc. It is not in h1 nor in h2
void G1ParCopyClosure<barrier, should_mark>::do_oop_work(T* p) {
  T heap_oop = RawAccess<>::oop_load(p);

  if (CompressedOops::is_null(heap_oop)) {
    return;
  }

  oop obj = CompressedOops::decode_not_null(heap_oop);

  //If obj is in H2
  //  (1) if (should_mark) set H2 region live bit
  //  (2) Fence heap traversal to H2
#ifdef TERA_MAINTENANCE  
  if(EnableTeraHeap){
    if( Universe::is_in_h2(obj) ){
      //set H2 region live bit
      if( should_mark ) Universe::teraHeap()->mark_used_region(cast_from_oop<HeapWord*>(obj));
      return;
    }    
  }
#endif

  assert(_worker_id == _par_scan_state->worker_id(), "sanity");

  const G1HeapRegionAttr state = _g1h->region_attr(obj);
  if (state.is_in_cset()) {
    oop forwardee;
    markWord m = obj->mark();
    if (m.is_marked()) {
      forwardee = cast_to_oop(m.decode_pointer());
    } else {


#ifdef TERA_EVAC_MOVE

      if( EnableTeraHeap  
          && _g1h->collector_state()->in_mixed_phase() 
          && obj->is_marked_move_h2()
        ){   
          forwardee = _par_scan_state->copy_to_h2_space(state, obj, m);            
      }else{
          forwardee = _par_scan_state->copy_to_survivor_space(state, obj, m);
      }
#else

      forwardee = _par_scan_state->copy_to_survivor_space(state, obj, m);
#endif
      
    }

    
    assert(forwardee != NULL, "forwardee should not be NULL");
    RawAccess<IS_NOT_NULL>::oop_store(p, forwardee);


    if (barrier == G1BarrierCLD ) {
      do_cld_barrier(forwardee);
    }


  } else {

    //IF NOT IN CSET THEN ...

    // here obj is not in cset, and is not in H2
    // but obj may be marked to be moved in H2 
    // => ignore it, move only the objects that are in the cset

    if (state.is_humongous()) {
      _g1h->set_humongous_is_live(obj);

    } else if ((barrier != G1BarrierNoOptRoots) && state.is_optional()) {
      //if the root is pointing into a region that is in the optional cset
      //then remember this root, to scan it when the evacuation of the opt cset comes
      _par_scan_state->remember_root_into_optional_region(p);
    }

   
    // The object is not in collection set. If we're a root scanning
    // closure during an initial mark pause then attempt to mark the object.    
    //Start of a concurrent marking = young gc + initial marking (piggybacked)


    if (should_mark) {
      mark_object(obj);
    }

  }
  
  trim_queue_partially();
}

template <class T> void G1RebuildRemSetClosure::do_oop_work(T* p) {
  oop const obj = RawAccess<MO_RELAXED>::oop_load(p);
  if (obj == NULL) {
    return;
  }

#ifdef TERA_MAINTENANCE
     
  //If obj is in H2
  //    -Fence heap traversal to H2
  //    -mark live bit of h2 region
  // no need to update the rem set of pointed region
  // bcs its an H2 region, and it does not have a rem set

  if (EnableTeraHeap && (Universe::is_in_h2(obj))){    
    Universe::teraHeap()->mark_used_region(cast_from_oop<HeapWord*>(obj));
    return;
  }

#endif

  if (HeapRegion::is_in_same_region(p, obj)) {
    return;
  }

  HeapRegion* to = _g1h->heap_region_containing(obj);
  HeapRegionRemSet* rem_set = to->rem_set();
  rem_set->add_reference(p, _worker_id);
}

#endif // SHARE_GC_G1_G1OOPCLOSURES_INLINE_HPP
