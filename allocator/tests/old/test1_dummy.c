/**************************************************
*
* file: test1.c
*
* @Author:   Iacovos G. Kolokasis
* @Version:  09-03-2021 
* @email:    kolokasis@ics.forth.gr
*
* Test to verify:
*	- allocator initialization
*	- object allocation in the correct positions
***************************************************/

#include <stdint.h>
#include <stdio.h>
#include "../include/sharedDefines.h"
#include "../include/regions.h"

#define CARD_SIZE ((uint64_t) (1 << 9))
#define PAGE_SIZE ((uint64_t) (1 << 12))

int main() {
	char *obj1;
	char *obj2;
	char *obj3;
	char *obj4;
    char *dum1;
    char *dum2;
    char *dum3;
    char *dum4;
    size_t dummy_size = 0;
    size_t tmp_dummy_size = 0;
	// Init allocator
	init(CARD_SIZE * PAGE_SIZE);

	// Check start and stop adddresses
	printf("\n");
	printf("Start Address: %p\n", start_addr_mem_pool());
	printf("Stop Address: %p\n", stop_addr_mem_pool());
	printf("Mem Pool Size: %lu\n", mem_pool_size());
	
	printf("\n");

	obj1 = allocate(1, &dummy_size);
    if (dummy_size){
        dum1 = allocate(dummy_size, &dummy_size);
	    printf("Allocate: dum1 %p\n", dum1);
	    assertf((dum1 - start_addr_mem_pool()) == 0, "Object start position");
        tmp_dummy_size = dummy_size; 
        dummy_size = 0;
        obj1 = allocate(1, &dummy_size);
        printf("Allocate: obj1  %p\n", obj1);
	    assertf((obj1 - dum1)/8 == tmp_dummy_size, "Object start position");
    } else {
        printf("Allocate: obj1 %p\n", obj1);
        assertf((obj1 - start_addr_mem_pool()) == 0, "Object start position");
    }

	obj2 = allocate(200, &dummy_size);
    if (dummy_size){
        dum2 = allocate(dummy_size, &dummy_size);
	    printf("Allocate: dum2 %p\n", dum2);
	    assertf((dum2 - obj1)/8 == 1, "Object start position");
        tmp_dummy_size = dummy_size;
        dummy_size = 0;
        obj2 = allocate(200, &dummy_size);
        printf("Allocate: obj2 %p\n", obj2);
	    assertf((obj2 - dum2)/8 == tmp_dummy_size, "Object start position");
    } else {
        printf("Allocate: obj2 %p\n", obj2);
        assertf((obj2 - obj1)/8 == 1, "Object start position");
    }
	
    obj3 = allocate(12020, &dummy_size);
    if (dummy_size){
        dum3 = allocate(dummy_size, &dummy_size);
	    printf("Allocate: dum3 %p\n", dum3);
	    assertf((dum3 - obj2)/8 == 200, "Object start position");
        tmp_dummy_size = dummy_size;
        dummy_size = 0;
        obj3 = allocate(12020, &dummy_size);
        printf("Allocate: obj3 %p\n", obj3);
        assertf((obj3 - dum3)/8 == tmp_dummy_size, "Object start position");
    } else {
        printf("Allocate: obj3 %p\n", obj3);
        assertf((obj3 - obj2)/8 == 200, "Object start position");
    }
	
	obj4 = allocate(10000000, &dummy_size);
    if (dummy_size){
        dum4 = allocate(dummy_size, &dummy_size);
	    printf("Allocate: dum4 %p\n", dum4);
	    assertf((dum4 - obj3)/8 == 12020, "Object start position");
        tmp_dummy_size = dummy_size;
        dummy_size = 0;
        obj4 = allocate(10000000, &dummy_size);
        printf("Allocate: obj4 %p\n", obj4);
        assertf((obj4 - dum4)/8 == tmp_dummy_size, "Object start position");
    } else {
        printf("Allocate: obj4 %p\n", obj4);
        assertf((obj4 - obj3)/8 == 12020, "Object start position");
    }
	printf("\n");

	return 0;
}
