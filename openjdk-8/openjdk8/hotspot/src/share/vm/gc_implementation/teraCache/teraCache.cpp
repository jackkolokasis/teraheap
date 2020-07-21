#include <iostream>
#include <string.h>
#include <errno.h>
#include "gc_implementation/teraCache/teraCache.hpp"

#define clean_errno() (errno == 0 ? "None" : strerror(errno))
#define log_error(M, ...) fprintf(stderr, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__) 
#define assertf(A, M, ...) if(!(A)) {log_error(M, ##__VA_ARGS__); assert(A);}

Stack<oop, mtGC>              TeraCache::_tera_root_stack;
char*        TeraCache::_start_addr = NULL; // Address shows where TeraCache start
char*        TeraCache::_stop_addr = NULL;  // Address shows where TeraCache ends
region_t     TeraCache::_region = NULL;
char*        TeraCache::_start_pos_region = NULL;

TeraCache::TeraCache()
{
  init();
  _start_addr = (char *)start_addr_mem_pool();
  _stop_addr =  (char*)((char *)start_addr_mem_pool() + mem_pool_size());
}

bool TeraCache::tc_check(char* ptr)
{
  if (ptr >= _start_addr && ptr < _stop_addr)
    return true;
  else 
    return false;
}

void TeraCache::tc_new_region(void)
{
  // Create a new region
  _region = new_region(NULL);
  // Initialize the size of the region
  _start_pos_region = (char *)rc_rstralloc0(_region, 5242880*sizeof(char));
  _next_pos_region = _start_pos_region;
}
    
char* TeraCache::tc_get_addr_region(void)
{
  //assertf(_start_pos_region == NULL, "[TERA CACHE] Region is not allocated");
  return (char *) _start_pos_region;
  
}
    
char* TeraCache::tc_region_top(oop obj, size_t size)
{
  char* tmp = _next_pos_region;
  // not really sure
  memcpy(tmp, obj, size);
  _next_pos_region = (char*) (_next_pos_region + size);

  return tmp;
}


void TeraCache::add_root_stack(oop obj) {
      std::cout << "TeraCache push stack" << std::endl;
      _tera_root_stack.push(obj);
}

oop TeraCache::get_root_stack() {
    std::cout << "TeraCache pop" << std::endl;
    return (oop) _tera_root_stack.pop();
}
