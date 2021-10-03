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
    init(CARD_SIZE * PAGE_SIZE);

    // Check start and stop adddresses
    printf("\n");
    printf("Start Address: %p\n", start_addr_mem_pool());
    printf("Stop Address: %p\n", stop_addr_mem_pool());

    //obj1 should be in region 0
    obj1 = allocate(1,0);
    printf("Allocate: %p\n", obj1);
    assertf((obj1 - start_addr_mem_pool()) == 0, "Object start position");

    //obj2 should be in region 1 
    obj2 = allocate(200,1);
    printf("Allocate: %p\n", obj2);
    assertf((obj2 - obj1)/8 == 262144, "Object start position");

    //obj3 should be in region 0
    obj3 = allocate(12020, 0);
    printf("Allocate: %p\n", obj3);
    assertf((obj3 - obj1)/8 == 1, "Object start position");

    //obj4 should be in region 2 
    obj4 = allocate(262140,2);
    printf("Allocate: %p\n", obj4);
    assertf((obj4 - obj2)/8 == 262144, "Object start position");

    //obj5 should be in region 1
    obj5 = allocate(4, 1);
    printf("Allocate: %p\n", obj5);
    assertf((obj5 - obj2)/8 == 200, "Object start position");

    //obj6 should be in region 0 
    obj6 = allocate(200, 0);
    printf("Allocate: %p\n", obj6);
    assertf((obj6 - obj3)/8 == 12020, "Object start position");

    //obj7 should be in region 3 
    obj7 = allocate(262140,1);
    printf("Allocate: %p\n", obj7);
    assertf((obj7 - obj4)/8 == 262144, "Object start position");

    //obj8 should be in region 4 
    obj8 = allocate(500,3);
    printf("Allocate: %p\n", obj8);
    assertf((obj8 - obj7)/8 == 262144, "Object start position");

    //obj9 should be in region 5 
    obj9 = allocate(500,2);
    printf("Allocate: %p\n", obj9);
    assertf((obj9 - obj8)/8 == 262144, "Object start position");

    //region 0 and region 1 grouped
    references(obj1,obj2);
    //region 2 added to group
    references(obj3,obj4);
    print_groups();
    //nothing should be done, obj4 and obj5 are in the same group
    references(obj4,obj5);
    print_groups();
    //region 3 added to group
    references(obj7,obj6);
    print_groups();
    //new group with region 4 and 5
    references(obj8,obj9);
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
    //region 4 and 5 should be freed
    free_regions();
    print_regions();
    print_groups();
    printf("\n");

    reset_used();
    mark_used(obj1);
    //no regions should be freed
    free_regions();
    print_regions();
    print_groups();
    printf("\n");

    reset_used();
    //all regions should be freed
    free_regions();
    print_regions();
    print_groups();
    printf("\n");
    return 0;
}
