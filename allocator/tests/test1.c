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

#include "../include/regions.h"
#include "../include/sharedDefines.h"

#include <stdint.h>
#include <stdio.h>
#define CARD_SIZE ((uint64_t) (1 << 9))
#define PAGE_SIZE ((uint64_t) (1 << 12))

#define SIZE_8B   (8)
#define SIZE_8K   (8*1024LU)
#define SIZE_100M (100*1024LU*1024)
#define SIZE_300M (300*1024LU*1024)

#define HEAPWORD (8)

#define SIZE_TO_WORD(SIZE) \
	((size_t) (SIZE / HEAPWORD))

int main() {
	char *obj1, *obj2, *obj3, *obj4;
	
	// Init allocator
	init(CARD_SIZE * PAGE_SIZE);

	obj1 = allocate(SIZE_TO_WORD(SIZE_8B));
	assertf((obj1 - start_addr_mem_pool()) == 0, "Object start position");

	obj2 = allocate(SIZE_TO_WORD(SIZE_8K));
	assertf((obj2 - obj1) == SIZE_8B, "Object start position");

	obj3 = allocate(SIZE_TO_WORD(SIZE_100M));
	assertf((obj3 - obj2) == SIZE_8K, "Object start position");
	
	obj4 = allocate(SIZE_TO_WORD(SIZE_300M));
	assertf((obj4 - obj3) == SIZE_100M, "Object start position");

	printf("------------------------------\n");
	printf("Test1:\t\t\t\033[1;32m[PASS]\033[0m\n");
	printf("------------------------------\n");

	return 0;
}
