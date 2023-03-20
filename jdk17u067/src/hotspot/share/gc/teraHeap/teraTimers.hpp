#ifndef SHARE_GC_TERAHEAP_TERATIMERS_HPP
#define SHARE_GC_TERAHEAP_TERATIMERS_HPP

#include "memory/allocation.hpp"
#include <sys/time.h>

class TeraTimers: public CHeapObj<mtInternal> {
private:
  struct timeval h2_scavenge_start_time;
  struct timeval h2_scavenge_end_time;

  struct timeval h1_marking_phase_start_time;
  struct timeval h1_marking_phase_end_time;

  struct timeval h2_mark_bwd_ref_start_time;
  struct timeval h2_mark_bwd_ref_end_time;
  
  struct timeval h2_precompact_start_time;
  struct timeval h2_precompact_end_time;

  struct timeval h1_summary_phase_start_time;
  struct timeval h1_summary_phase_end_time;
  
  struct timeval h2_compact_start_time;
  struct timeval h2_compact_end_time;
  
  struct timeval h2_adjust_bwd_ref_start_time;
  struct timeval h2_adjust_bwd_ref_end_time;
  
  struct timeval h1_adjust_roots_start_time;
  struct timeval h1_adjust_roots_end_time;
  
  struct timeval h1_compact_start_time;
  struct timeval h1_compact_end_time;
  
  struct timeval h2_clear_fwd_table_start_time;
  struct timeval h2_clear_fwd_table_end_time;

  void print_ellapsed_time(struct timeval start_time,
                           struct timeval end_time, char* msg);

public:
  TeraTimers();
  ~TeraTimers();

  void h2_scavenge_start();
  void h2_scavenge_end();

  void h1_marking_phase_start();
  void h1_marking_phase_end();

  void h2_mark_bwd_ref_start();
  void h2_mark_bwd_ref_end();
  
  void h2_precompact_start();
  void h2_precompact_end();

  void h1_summary_phase_start();
  void h1_summary_phase_end();

  void h2_compact_start();
  void h2_compact_end();

  void h2_adjust_bwd_ref_start();
  void h2_adjust_bwd_ref_end();

  void h1_adjust_roots_start();
  void h1_adjust_roots_end();

  void h1_compact_start();
  void h1_compact_end();
  
  void h2_clear_fwd_table_start();
  void h2_clear_fwd_table_end();
};
#endif
