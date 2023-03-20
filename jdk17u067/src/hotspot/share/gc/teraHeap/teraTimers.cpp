#include "gc/teraHeap/teraTimers.hpp"
#include "gc/shared/gc_globals.hpp"
#include "runtime/java.hpp"

void TeraTimers::print_ellapsed_time(struct timeval start_time,
                                     struct timeval end_time, char* msg) {

  thlog_or_tty->print_cr("[STATISTICS] | %s %llu\n",
                         msg,
                         (unsigned long long)((end_time.tv_sec - start_time.tv_sec) * 1000) + // convert to ms
                         (unsigned long long)((end_time.tv_usec - start_time.tv_usec) / 1000)); // convert to ms
}

TeraTimers::TeraTimers() {
  if (!TeraHeapStatistics)
    vm_exit_during_initialization("Enable -XX:+TeraHeapStatistics");
};

void TeraTimers::h2_scavenge_start() {
  gettimeofday(&h2_scavenge_start_time, NULL);
}

void TeraTimers::h2_scavenge_end() {
  char msg[12] = "H2_SCAVENGE";

  gettimeofday(&h2_scavenge_end_time, NULL);
  print_ellapsed_time(h2_scavenge_start_time, h2_scavenge_end_time, msg);
}

void TeraTimers::h1_marking_phase_start() {
  gettimeofday(&h1_marking_phase_start_time, NULL);
}

void TeraTimers::h1_marking_phase_end() {
  char msg[17] = "H1_MARKING_PHASE";

  gettimeofday(&h1_marking_phase_end_time, NULL);
  print_ellapsed_time(h1_marking_phase_start_time, h1_marking_phase_end_time, msg);
}

void TeraTimers::h2_mark_bwd_ref_start() {
  gettimeofday(&h2_mark_bwd_ref_start_time, NULL);
  
}

void TeraTimers::h2_mark_bwd_ref_end() {
  char msg[19] = "H2_MARKING_BWD_REF";

  gettimeofday(&h2_mark_bwd_ref_end_time, NULL);
  print_ellapsed_time(h2_mark_bwd_ref_start_time, h2_mark_bwd_ref_end_time, msg);
}

void TeraTimers::h2_precompact_start() {
  gettimeofday(&h2_precompact_start_time, NULL);
}

void TeraTimers::h2_precompact_end() {
  char msg[14] = "H2_PRECOMPACT";

  gettimeofday(&h2_precompact_end_time, NULL);
  print_ellapsed_time(h2_precompact_start_time, h2_precompact_end_time, msg);
}

void TeraTimers::h1_summary_phase_start() {
  gettimeofday(&h1_summary_phase_start_time, NULL);
}

void TeraTimers::h1_summary_phase_end() {
  char msg[17] = "H1_SUMMARY_PHASE";

  gettimeofday(&h1_summary_phase_end_time, NULL);
  print_ellapsed_time(h1_summary_phase_start_time, h1_summary_phase_end_time, msg);
}

void TeraTimers::h2_compact_start() {
  gettimeofday(&h2_compact_start_time, NULL);
}

void TeraTimers::h2_compact_end() {
  char msg[17] = "H2_COMPACT_PHASE";

  gettimeofday(&h2_compact_end_time, NULL);
  print_ellapsed_time(h2_compact_start_time, h2_compact_end_time, msg);
}

void TeraTimers::h2_adjust_bwd_ref_start() {
  gettimeofday(&h2_adjust_bwd_ref_start_time, NULL);
}

void TeraTimers::h2_adjust_bwd_ref_end() {
  char msg[18] = "H2_ADJUST_BWD_REF";

  gettimeofday(&h2_adjust_bwd_ref_end_time, NULL);
  print_ellapsed_time(h2_adjust_bwd_ref_start_time, h2_adjust_bwd_ref_end_time, msg);
}

void TeraTimers::h1_adjust_roots_start() {
  gettimeofday(&h1_adjust_roots_start_time, NULL);
}

void TeraTimers::h1_adjust_roots_end() {
  char msg[16] = "H1_ADJUST_ROOTS";
  gettimeofday(&h1_adjust_roots_end_time, NULL);

  print_ellapsed_time(h1_adjust_roots_start_time, h1_adjust_roots_end_time, msg);
}

void TeraTimers::h1_compact_start() {
  gettimeofday(&h1_compact_start_time, NULL);
}

void TeraTimers::h1_compact_end() {
  char msg[11] = "H1_COMPACT";

  gettimeofday(&h1_compact_end_time, NULL);
  print_ellapsed_time(h1_compact_start_time, h1_compact_end_time, msg);
}
  
void TeraTimers::h2_clear_fwd_table_start() {
  gettimeofday(&h2_clear_fwd_table_start_time, NULL);
}

void TeraTimers::h2_clear_fwd_table_end() {
  char msg[19] = "H2_CLEAR_FWD_TABLE";

  gettimeofday(&h2_clear_fwd_table_end_time, NULL);
  print_ellapsed_time(h2_clear_fwd_table_start_time, h2_clear_fwd_table_end_time, msg);
  thlog_or_tty->flush();
}
