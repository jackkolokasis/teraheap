#ifndef SHARE_GC_TERAHEAP_TERACPUUSAGE_HPP
#define SHARE_GC_TERAHEAP_TERACPUUSAGE_HPP

#include "memory/allocation.hpp"
#include <sys/resource.h>

#define STAT_START true
#define STAT_END false
#define GC_STAT true
#define MUTATOR_STAT false

// TODO: Should we need to calculte the system time?
class TeraCPUUsage : public CHeapObj<mtInternal> {

public:
  // Read CPU usage
  virtual void read_cpu_usage(bool is_start, bool is_gc) = 0;
  
  // Calculate iowait time of the process
  virtual void calculate_iowait_time(double duration, double *iowait_time,
                                     bool is_gc) = 0;
};

class TeraSimpleCPUUsage : public TeraCPUUsage {
private:
  unsigned long long iowait_start;    //< CPU iowait at the start of the window
  unsigned long long iowait_end;      //< CPU iowait at the end of the window

  unsigned long long cpu_start;       //< CPU usage at the start of the window
  unsigned long long cpu_end;         //< CPU usage at the end of the window
  
  unsigned long long gc_iowait_start; //< IO wait time created during gc
  unsigned long long gc_iowait_end;   //< IO wait time created during gc
  
  unsigned long long gc_cpu_start;    //< IO wait time created during gc
  unsigned long long gc_cpu_end;      //< IO wait time created during gc

public:
  // Read CPU usage
  void read_cpu_usage(bool is_start, bool is_gc);
  
  // Calculate iowait time of the process
  void calculate_iowait_time(double duration, double *iowait_time, bool is_gc);
};

class TeraMultiExecutorCPUUsage : public TeraCPUUsage {
private:
  struct rusage start, end;

public:
  // Read CPU usage
  void read_cpu_usage(bool is_start, bool is_gc);
  
  // Calculate iowait time of the process
  void calculate_iowait_time(double duration, double *iowait_time, bool is_gc);
};

#endif // SHARE_GC_TERAHEAP_TERACPUUSAGE_HPP
