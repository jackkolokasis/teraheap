
/***************************************************
*
* file: tc_allocate.c
*
* @Author:   Iacovos G. Kolokasis
* @Author:   Giannos Evdorou
* @Version:  09-03-2021
* @email:    kolokasis@ics.forth.gr
*
* Test to verify:
*       - allocator initialization
*       - object allocation in the correct positions
***************************************************/

#include <stdint.h>
#include <stdio.h>
#include "../include/sharedDefines.h"
#include "../include/regions.h"
#include "../include/segments.h"

#define CARD_SIZE ((uint64_t) (1 << 9))
#define PAGE_SIZE ((uint64_t) (1 << 12))

#define SIZE_30MB (3932160)
#define SIZE_5MB (655360)
#define SIZE_1MB (131072)
#define SIZE_2MB (262144)

//this test needs 256MB region size
int main() {
  char *obj1, *obj2, *obj3, *obj4, *obj5, *obj6;

  // Init allocator
  init(CARD_SIZE * PAGE_SIZE);

  //obj1 should be in region 0
  obj1 = allocate(1, 0, 0);
  fprintf(stderr, "Allocate: %p\n", obj1);

  //obj2 should be in region 1 
  obj2 = allocate(200, 1, 0);
  fprintf(stderr, "Allocate: %p\n", obj2);

  //obj3 should be in region 0
  obj3 = allocate(SIZE_5MB, 0, 0);
  fprintf(stderr, "Allocate: %p\n", obj3);

  obj4 = allocate(SIZE_1MB, 2, 0);
  fprintf(stderr, "Allocate: %p\n", obj4);

  obj5 = allocate(10, 2, 0);
  fprintf(stderr, "Allocate: %p\n", obj5);

  obj6 = allocate(10, 0, 0);
  fprintf(stderr, "Allocate: %p\n", obj6);
  

  reset_used();
  mark_used(obj3);
  mark_used(obj4);
  mark_used(obj6);
  
  //obj2 should be in region 1 
  obj2 = allocate(64000, 10, 0);
  fprintf(stderr, "Allocate: %p\n", obj2);

  free_regions();

  obj3 = allocate(SIZE_2MB, 4, 0);
  fprintf(stderr, "Allocate: %p\n", obj3);
  
  free_regions();
  
  obj3 = allocate(SIZE_1MB, 4, 0);
  fprintf(stderr, "Allocate: %p\n", obj3);

  //free_regions();

  return 0;
}
