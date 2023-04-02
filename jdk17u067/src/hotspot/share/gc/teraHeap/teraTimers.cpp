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
  
  h1_card_table_start_time = NEW_C_HEAP_ARRAY(timeval, ParallelGCThreads, mtGC);
  h2_card_table_start_time = NEW_C_HEAP_ARRAY(timeval, ParallelGCThreads, mtGC);
};
  
TeraTimers::~TeraTimers() {
  FREE_C_HEAP_ARRAY(timeval, h1_card_table_start_time);
  FREE_C_HEAP_ARRAY(timeval, h2_card_table_start_time);
}

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
}

// Keep for each GC thread the time that need to traverse the H1
// card table.
// Each thread writes the time in a table based on their ID and then we
// take the maximum time from all the threads as the total time.
void TeraTimers::h1_card_table_start(unsigned int worker_id) {
  assert(worker_id < ParallelGCThreads, "Index out of bound");
  gettimeofday(&h1_card_table_start_time[worker_id], NULL);
}

void TeraTimers::h1_card_table_end(unsigned int worker_id) {
  assert(worker_id < ParallelGCThreads, "Index out of bound");
  timeval start_time = h1_card_table_start_time[worker_id];
  timeval end_time;

  gettimeofday(&end_time, NULL);
  long timeDiff = ((end_time.tv_sec - start_time.tv_sec) * 1000) +
    ((end_time.tv_usec - start_time.tv_usec) / 1000);

  // Store the time difference in the array. Avoid to allocate extra
  // metadata space
  h1_card_table_start_time[worker_id].tv_sec = 0;
  h1_card_table_start_time[worker_id].tv_usec = timeDiff;
}

// Keep for each GC thread the time that need to traverse the H2
// card table.
// Each thread writes the time in a table based on each ID and then we
// take the maximum time from all the threads as the total time.
void TeraTimers::h2_card_table_start(unsigned int worker_id) {
  assert(worker_id < ParallelGCThreads, "Index out of bound");
  gettimeofday(&h2_card_table_start_time[worker_id], NULL);
}

void TeraTimers::h2_card_table_end(unsigned int worker_id) {
  assert(worker_id < ParallelGCThreads, "Index out of bound");
  timeval start_time = h2_card_table_start_time[worker_id];
  timeval end_time;

  gettimeofday(&end_time, NULL);
  long timeDiff = ((end_time.tv_sec - start_time.tv_sec) * 1000) +
    ((end_time.tv_usec - start_time.tv_usec) / 1000);

  // Store the time difference in the array. Avoid to allocate extra
  // metadata space
  h2_card_table_start_time[worker_id].tv_sec = 0;
  h2_card_table_start_time[worker_id].tv_usec = timeDiff;
}

// Print the time to traverse the TeraHeap dirty card tables
// and the time to traverse the Heap dirty card tables during minor
// GC.
void TeraTimers::print_card_table_scanning_time() {
	long h1_max_time = 0;
	long h2_max_time = 0;

	for (unsigned int i = 0; i < ParallelGCThreads; i++) {
		if (h1_max_time < h1_card_table_start_time[i].tv_usec)
			h1_max_time = h1_card_table_start_time[i].tv_usec;
		
		if (h2_max_time < h2_card_table_start_time[i].tv_usec)
			h2_max_time = h2_card_table_start_time[i].tv_usec;
	}

	thlog_or_tty->print_cr("[STATISTICS] | H1_CT_TIME = %lu\n", h1_max_time);
	thlog_or_tty->print_cr("[STATISTICS] | H2_CT_TIME = %lu\n", h2_max_time);

	// Initialize arrays for the next minor collection
  memset(h1_card_table_start_time, 0, ParallelGCThreads * sizeof(timeval));
  memset(h2_card_table_start_time, 0, ParallelGCThreads * sizeof(timeval));
}
