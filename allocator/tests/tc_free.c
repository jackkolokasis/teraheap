/***************************************************
*
* file: tc_free.c
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

#include <stdint.h>
#include <stdio.h>
#include "../include/sharedDefines.h"
#include "../include/regions.h"
#include "../include/segments.h"

#define CARD_SIZE ((uint64_t) (1 << 9))
#define PAGE_SIZE ((uint64_t) (1 << 12))

//this test needs 2MB region size
int main() {
    char *obj1;
    char *obj2;
    char *obj3;
    char *obj4;
    char *obj5;
    char *obj6;
    char *obj7;
    char *obj8;
    char *obj9;
    // Init allocator
#ifndef H2_DYNAMIC_FILE_ALLOCATION
    init(CARD_SIZE * PAGE_SIZE);
#else
    init(CARD_SIZE * PAGE_SIZE,"/mnt/fmap/file.txt",161061273600);
#endif

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
    //region 2 added to group
    references(obj3, obj4);

    print_groups();

    //nothing should be done, obj4 and obj5 are in the same group
    references(obj4, obj5);
    print_groups();
    //region 3 added to group
    references(obj7, obj6);
    print_groups();
    //new group with region 4 and 5
    references(obj8, obj9);
    print_groups();
	
    reset_used();
    mark_used(obj1);
    mark_used(obj6);
    mark_used(obj8);

    //nothing should be freed because all regions belong to the same group 
    free_regions();
	assertf(total_allocated_regions() == 4, "Number of allocated regions is incorrect");

    print_regions();
    print_groups();
    
	reset_used();
    mark_used(obj1);
    mark_used(obj6);
	assertf(total_used_regions() == 3,
			"Number of used regions is incorrect %lu", total_used_regions());

    free_regions();
	assertf(total_allocated_regions() == 3,
			"Number of allocated regions is incorrect %lu", total_used_regions());
    print_regions();
    print_groups();
    
	reset_used();
	assertf(total_used_regions() == 0, "Number of used regions is incorrect");
    //all regions should be freed
    free_regions();
	assertf(total_allocated_regions() == 0,
			"Number of allocated regions is incorrect");
    print_regions();
    print_groups();

	printf("--------------------------------------\n");
	printf("TC_Free:\t\t\t\033[1;32m[PASS]\033[0m\n");
	printf("--------------------------------------\n");

    return 0;
}
