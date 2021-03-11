/**************************************************
*
* file: Makefile
*
* @Author:   Iacovos G. Kolokasis
* @Version:  09-03-2021 
* @email:    kolokasis@ics.forth.gr
*
***************************************************/

#include <stdio.h>
#include "../include/sharedDefines.h"
#include "../include/regions.h"

#define CARD_SIZE 512
#define PAGE_SIZE 4096

int main() {
	char *obj1;
	char *obj2;
	char *obj3;
	char *obj4;
	
	// Init allocator
	init(CARD_SIZE * PAGE_SIZE);

	// Check start and stop adddresses
	printf("\n");
	printf("Start Address: %p\n", start_addr_mem_pool());
	printf("Stop Address: %p\n", stop_addr_mem_pool());
	printf("Mem Pool Size: %lu\n", mem_pool_size());
	
	assertf((stop_addr_mem_pool() - start_addr_mem_pool()) == mem_pool_size(), 
			"Size is wrong");

	printf("\n");

	obj1 = allocate(1);
	printf("Allocate: %p\n", obj1);
	assertf((obj1 - start_addr_mem_pool()) == 0, "Object start position");

	obj2 = allocate(200);
	printf("Allocate: %p\n", obj2);
	assertf((obj2 - obj1)/8 == 1, "Object start position");

	obj3 = allocate(12020);
	printf("Allocate: %p\n", obj3);
	assertf((obj3 - obj2)/8 == 200, "Object start position");
	
	obj4 = allocate(10000000);
	printf("Allocate: %p\n", obj4);
	assertf((obj4 - obj3)/8 == 12020, "Object start position");

	printf("\n");

	return 0;
}
