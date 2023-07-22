
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

//this test needs 256MB region size
int main() {
    char *obj1, *obj2, *obj3, *obj4, *obj5, *obj6, *obj7, *obj8, *obj9;

    // Init allocator
#ifndef H2_DYNAMIC_FILE_ALLOCATION
    init(CARD_SIZE * PAGE_SIZE);
#else
    init(CARD_SIZE * PAGE_SIZE,"/mnt/fmap/file.txt", 161061273600);
#endif

    //obj1 should be in region 0
    obj1 = allocate(1, 0, 0);
    fprintf(stderr, "Allocate: %p\n", obj1);
    assertf((obj1 - start_addr_mem_pool()) == 0, "Object start position");

    //obj2 should be in region 1 
    obj2 = allocate(200, 1, 0);
    fprintf(stderr, "Allocate: %p\n", obj2);
    assertf((obj2 - obj1)/8 == 33554432, "Object start position %zu", (obj2 - obj1) / 8); 

    //obj3 should be in region 0
    obj3 = allocate(12020, 0, 0);
    fprintf(stderr, "Allocate: %p\n", obj3);
    assertf((obj3 - obj1)/8 == 1, "Object start position");

    //obj4 should be in region 2 
    obj4 = allocate(262140, 2, 0);
    fprintf(stderr, "Allocate: %p\n", obj4);
    assertf((obj4 - obj2)/8 == 33554432, "Object start position %zu", (obj4 - obj2) / 8);

    //obj5 should be in region 1
    obj5 = allocate(4, 1, 0);
    fprintf(stderr, "Allocate: %p\n", obj5);
    assertf((obj5 - obj2)/8 == 200, "Object start position");

    //obj6 should be in region 0 
    obj6 = allocate(200, 0, 0);
    fprintf(stderr, "Allocate: %p\n", obj6);
    assertf((obj6 - obj3)/8 == 12020, "Object start position");

    //obj7 should be in region 3 
    obj7 = allocate(262140, 1, 0);
    fprintf(stderr, "Allocate: %p\n", obj7);
    assertf((obj7 - obj5)/8 == 4, "Object start position %zu", (obj7 - obj5) / 8);

    //obj8 should be in region 4 
    obj8 = allocate(500, 3, 0);
    fprintf(stderr, "Allocate: %p\n", obj8);
    assertf((obj8 - obj4)/8 == 33554432, "Object start position");

    //obj9 should be in region 5 
    obj9 = allocate(500, 2, 0);
    fprintf(stderr, "Allocate: %p\n", obj9);
    assertf((obj9 - obj4)/8 == 262140, "Object start position");
	
	printf("--------------------------------------\n");
	printf("TC_Allocate:\t\t\t\033[1;32m[PASS]\033[0m\n");
	printf("--------------------------------------\n");

	return 0;
}
