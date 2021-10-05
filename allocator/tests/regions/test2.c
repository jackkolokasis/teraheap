/***************************************************
*
* file: test2.c
*
* @Author:   Iacovos G. Kolokasis
* @Version:  09-03-2021
* @email:    kolokasis@ics.forth.gr
*
* Test to verify:
*       - allocator initialization
*       - object allocation in the correct positions
***************************************************/

#include "../include/sharedDefines.h"
#include "../include/regions.h"
#include "../include/segments.h"

#include <stdint.h>
#include <stdio.h>

#define CARD_SIZE ((uint64_t) (1 << 9))
#define PAGE_SIZE ((uint64_t) (1 << 12))

//this test needs 2MB segment size
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
    init(CARD_SIZE * PAGE_SIZE);

    // Check start and stop adddresses
    printf("\n");
    printf("Start Address: %p\n", start_addr_mem_pool());
    printf("Stop Address: %p\n", stop_addr_mem_pool());

    //obj1 should be in segment 0
    obj1 = allocate(1);
    printf("Allocate: %p\n", obj1);
    assertf((obj1 - start_addr_mem_pool()) == 0, "Object start position");

    //obj2 should be in segment 0
    obj2 = allocate(200);
    printf("Allocate: %p\n", obj2);
    assertf((obj2 - obj1)/8 == 1, "Object start position");

    //obj3 should be in segment 0
    obj3 = allocate(12020);
    printf("Allocate: %p\n", obj3);
    assertf((obj3 - obj2)/8 == 200, "Object start position");

    //obj4 should be in segment 1
    obj4 = allocate(262140);
    printf("Allocate: %p\n", obj4);
    assertf((obj4 - obj1)/8 == 262144, "Object start position");

    //obj5 should be in segment 1
    obj5 = allocate(4);
    printf("Allocate: %p\n", obj5);
    assertf((obj5 - obj4)/8 == 262140, "Object start position");

    //obj6 should be in segment 2
    obj6 = allocate(200);
    printf("Allocate: %p\n", obj6);

    //obj7 should be in segment 3
    obj7 = allocate(262140);
    printf("Allocate: %p\n", obj7);

    //obj8 should be in segment 4
    obj8 = allocate(500);
    printf("Allocate: %p\n", obj8);

    //obj9 should be in segment 4
    obj9 = allocate(500);
    printf("Allocate: %p\n", obj9);

    //nothing should be done, obj1 and obj2 are in the same segment
    references(obj1,obj2);
    //new group created with segments 0 and 1
    references(obj3,obj4);
    print_groups();
    //nothing should be done, obj4 and obj5 are in the same segment
    references(obj4,obj5);
    print_groups();
    //new group created with segments 2 and 3
    references(obj7,obj6);
    print_groups();
    printf("\n");

    reset_used();
    mark_used(obj1);
    mark_used(obj6);
    mark_used(obj8);
    //nothing should be freed because there is one region in each
    //group that is used
    free_regions();
    print_regions();
    print_groups();
    printf("\n");

    reset_used();
    mark_used(obj1);
    mark_used(obj6);
    //region 4 should be freed
    free_regions();
    print_regions();
    print_groups();
    printf("\n");

    reset_used();
    mark_used(obj1);
    //regions 2 and 3 should be freed
    free_regions();
    print_regions();
    print_groups();
    printf("\n");

    return 0;
}
