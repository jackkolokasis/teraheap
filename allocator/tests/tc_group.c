/***************************************************
*
* file: tc_group.c
*
* @Author:   Iacovos G. Kolokasis
* @Author:   Giannos Evdorou
* @Version:  29-11-2021
* @email:    kolokasis@ics.forth.gr
*
* Test to verify:
*       - allocator initialization
*       - object allocation in the correct positions
***************************************************/
#include "tc_heap.h"

//this test needs 2MB region size
int main(int argc, char **argv) {
    if(argc != 4){
      fprintf(stderr,"Usage: ./tc_allocate.bin <mount_point> <h1_size> <h2_size>\n");
      exit(EXIT_FAILURE);
    }
    unsigned long long h1_size = convert_string_to_number(argv[2]);
    ERRNO_CHECK
    unsigned long long h2_size = convert_string_to_number(argv[3]);
    ERRNO_CHECK

    char *obj1, *obj2, *obj3; char *obj4; char *obj5; char *obj6;
    char *obj7;
    char *obj8;
    char *obj9;

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
    obj3 = allocate(12020, 0, 0);
    fprintf(stderr, "Allocate: %p\n", obj3);

    //obj4 should be in region 2 
    obj4 = allocate(262140, 2, 0);
    fprintf(stderr, "Allocate: %p\n", obj4);

    //obj5 should be in region 1
    obj5 = allocate(4, 1, 0);
    fprintf(stderr, "Allocate: %p\n", obj5);

    //obj6 should be in region 0 
    obj6 = allocate(200, 0, 0);
    fprintf(stderr, "Allocate: %p\n", obj6);

    //obj7 should be in region 3 
    obj7 = allocate(262140, 1, 0);
    fprintf(stderr, "Allocate: %p\n", obj7);

    //obj8 should be in region 4 
    obj8 = allocate(500, 3, 0);
    fprintf(stderr, "Allocate: %p\n", obj8);

    //obj9 should be in region 5 
    obj9 = allocate(500, 2, 0);
    fprintf(stderr, "Allocate: %p\n", obj9);

    //region 0 and region 1 grouped
    references(obj1, obj2);
    //region 2 added to tera_group
    references(obj3, obj4);

    print_groups();

    //nothing should be done, obj4 and obj5 are in the same tera_group
    references(obj4, obj5);
    print_groups();
    //region 3 added to tera_group
    references(obj7, obj6);
    print_groups();
    //new tera_group with region 4 and 5
    references(obj8, obj9);
    print_groups();
	
	printf("--------------------------------------\n");
	printf("TC_Group:\t\t\t\033[1;32m[PASS]\033[0m\n");
	printf("--------------------------------------\n");

    return 0;
}
