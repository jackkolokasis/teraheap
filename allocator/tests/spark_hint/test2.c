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

#include <stdint.h>
#include <stdio.h>
#include "../include/sharedDefines.h"
#include "../include/regions.h"
#include "../include/segments.h"

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
    obj1 = allocate(262140,0);
    printf("Allocate: %p\n", obj1);
    assertf((obj1 - start_addr_mem_pool()) == 0, "Object start position");

    //obj2 should be in segment 1 
    obj2 = allocate(200,1);
    printf("Allocate: %p\n", obj2);
    assertf((obj2 - obj1)/8 == 262144, "Object start position");

    //obj3 should be in segment 2
    obj3 = allocate(12020, 0);
    printf("Allocate: %p\n", obj3);
    assertf((obj3 - obj2)/8 == 262144, "Object start position");

    //obj4 should be in segment 3
    obj4 = allocate(262140,2);
    printf("Allocate: %p\n", obj4);
    assertf((obj4 - obj3)/8 == 262144, "Object start position");

    //obj5 should be in segment 1
    obj5 = allocate(4, 1);
    printf("Allocate: %p\n", obj5);
    assertf((obj5 - obj2)/8 == 200, "Object start position");

    //obj6 should be in segment 2
    obj6 = allocate(200, 0);
    printf("Allocate: %p\n", obj6);
    assertf((obj6 - obj3)/8 == 12020, "Object start position");

    //obj7 should be in segment 4
    obj7 = allocate(262140,1);
    printf("Allocate: %p\n", obj7);
    assertf((obj7 - obj4)/8 == 262144, "Object start position");

    //obj8 should be in segment 5
    obj8 = allocate(500,3);
    printf("Allocate: %p\n", obj8);
    assertf((obj8 - obj7)/8 == 262144, "Object start position");

    //obj9 should be in segment 6
    obj9 = allocate(500,2);
    printf("Allocate: %p\n", obj9);
    assertf((obj9 - obj8)/8 == 262144, "Object start position");

    //region 0 and region 1 are grouped
    references(obj1,obj2);
    //region 2 and region 3 are grouped
    references(obj3,obj4);
    print_groups();
    //the two previous groups are merged
    references(obj4,obj5);
    print_groups();
    //region 4 is added to the group
    references(obj7,obj6);
    print_groups();
    printf("\n");

    reset_used();
    mark_used(obj1);
    mark_used(obj6);
    mark_used(obj8);
    //region 6 is freed
    free_regions();
    print_regions();
    print_groups();
    printf("\n");

    reset_used();
    mark_used(obj1);
    mark_used(obj6);
    //region 5 is freed
    free_regions();
    print_regions();
    print_groups();
    printf("\n");

    reset_used();
    mark_used(obj1);
    //nothing is freed
    free_regions();
    print_regions();
    print_groups();
    printf("\n");

    return 0;
}
