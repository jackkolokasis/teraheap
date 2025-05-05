
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
#include "tc_heap.h"


#define SIZE_30MB (3932160)
#define SIZE_5MB (655360)
#define SIZE_1MB (131072)
#define SIZE_2MB (262144)

//this test needs 256MB region size
int main(int argc, char **argv) {
    if(argc != 4){
      fprintf(stderr,"Usage: ./tc_allocate_multi_regions.bin <mount_point> <h1_size> <h2_size>\n");
      exit(EXIT_FAILURE);
    }
    unsigned long long h1_size = convert_string_to_number(argv[2]);
    ERRNO_CHECK
    unsigned long long h2_size = convert_string_to_number(argv[3]);
    ERRNO_CHECK

  char *obj1, *obj2, *obj3, *obj4, *obj5, *obj6;

  initialize_h1(H1_ALIGNMENT, NULL, h1_size * GB, 0);
  initialize_h2(16, H2_ALIGNMENT, argv[1], h2_size * GB, (void *)(h1.start_address + h1_size * GB));
  print_heap_statistics();

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
  printf("--------------------------------------\n");
	printf("TC_Allocate_Multi_Regions:\t\t\t\033[1;32m[PASS]\033[0m\n");
	printf("--------------------------------------\n");

  return 0;
}
