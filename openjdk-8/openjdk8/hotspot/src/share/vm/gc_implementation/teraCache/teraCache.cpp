#include <iostream>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include "gc_implementation/teraCache/teraCache.hpp"
#include "memory/sharedDefines.h"

#define clean_errno() (errno == 0 ? "None" : strerror(errno))
#define log_error(M, ...) fprintf(stderr, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__) 
#define assertf(A, M, ...) if(!(A)) {log_error(M, ##__VA_ARGS__); assert(A);}

//Stack<oop, mtGC>              TeraCache::_tera_root_stack;
char*        TeraCache::_start_addr = NULL; // Address shows where TeraCache start
char*        TeraCache::_stop_addr = NULL;  // Address shows where TeraCache ends
region_t     TeraCache::_region = NULL;
char*        TeraCache::_start_pos_region = NULL;
char*    TeraCache::_next_pos_region = NULL;   // Next allocated region in region

TeraCache::TeraCache()
{
  init();
  _start_addr = (char *)start_addr_mem_pool();
  _stop_addr =  (char*)((char *)_start_addr + mem_pool_size());
}

bool TeraCache::tc_check(oop ptr)
{

#if DEBUG_TERA_CACHE
  printf("[TC_CHECK] | OOP(PTR) = %p | START_ADDR = %p | STOP_ADDR = %p | start = %p \n", ptr, _start_addr, _stop_addr, (char *) ptr);
#endif

  if ((char *)ptr >= _start_addr && (char*) ptr < _stop_addr)
  {
#if DEBUG_TERA_CACHE
    std::cerr << "[TC CHECK] = TRUE" << std::endl; 
#endif
    return true;
  }
  else 
  {
#if DEBUG_TERA_CACHE
    std::cerr << "[TC CHECK] = FALSE" << std::endl; 
#endif
    return false;
  }
}

void TeraCache::tc_new_region(void)
{
  // Create a new region
  _region = new_region(NULL);

  // Initialize the size of the region
  _start_pos_region = (char *)rc_rstralloc0(_region, 5242880*sizeof(char));
  _next_pos_region  = _start_pos_region;
}
    
char* TeraCache::tc_get_addr_region(void)
{
  assertf((char *)(_start_pos_region) != NULL, "[TERA CACHE] Region is not allocated");
  return (char *) _start_pos_region;
}
    
char* TeraCache::tc_region_top(oop obj, size_t size)
{
#if DEBUG_TERA_CACHE
  printf("[TC_REGION_TOP] | OOP(PTR) = %p | NEXT_POS = %p \n", obj, _next_pos_region);
  std::cout << "SIZE =" << " " << size << std::endl;
#endif
  char* tmp = _next_pos_region;
  // not really sure
  //memcpy(tmp, obj, size);
  //memmove(tmp, obj, size);
  _next_pos_region = (char*) (_next_pos_region + size*8);

  return tmp;
}

//void TeraCache::add_root_stack(oop obj) {
//      std::cout << "TeraCache push stack" << std::endl;
//      _tera_root_stack.push(obj);
//}
//
//oop TeraCache::get_root_stack() {
//    std::cout << "TeraCache pop" << std::endl;
//    return (oop) _tera_root_stack.pop();
//}
