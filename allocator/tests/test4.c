/**************************************************
*
* file: test3.c
*
* @Author:   Iacovos G. Kolokasis
* @Version:  20-03-2021 
* @email:    kolokasis@ics.forth.gr
*
* Test to verify:
*	- allocator initialization
*	- object allocation in the correct positions
*	using asynchronous I/O
***************************************************/

#include "../include/sharedDefines.h"
#include "../include/regions.h"

#include <stdint.h>
#include <stdio.h>

#define CARD_SIZE ((uint64_t) (1 << 9))
#define PAGE_SIZE ((uint64_t) (1 << 12))

#define SIZE_80B   (80)
#define SIZE_160B  (160)
#define SIZE_1M (1*1024LU*1024)
#define SIZE_4M (4*1024LU*1024)

#define HEAPWORD (8)

#define SIZE_TO_WORD(SIZE) \
	((size_t) (SIZE / HEAPWORD))

int main() {
	char *obj1, *obj2, *obj3, *obj4;
	char *tmp, *tmp2, *tmp3, *tmp4;
	
	// Init allocator
	init(CARD_SIZE * PAGE_SIZE);

	tmp = malloc(SIZE_80B * sizeof(char));
	memset(tmp, '1', SIZE_80B);
	tmp[SIZE_80B - 1] = '\0';

	tmp2 = malloc(SIZE_160B * sizeof(char));
	memset(tmp2, '2', SIZE_160B);
	tmp2[SIZE_160B - 1] = '\0';

	tmp3 = malloc(SIZE_1M * sizeof(char));
	memset(tmp3, '3', SIZE_1M);
	tmp3[SIZE_1M - 1] = '\0';

	tmp4 = malloc(SIZE_4M * sizeof(char));
	memset(tmp4, '4', SIZE_4M);
	tmp4[SIZE_4M - 1] = '\0';
	
	obj1 = allocate(SIZE_TO_WORD(SIZE_80B));
	r_write(tmp, obj1, SIZE_TO_WORD(SIZE_80B));
	
	obj2 = allocate(SIZE_TO_WORD(SIZE_160B));
	r_write(tmp2, obj2, SIZE_TO_WORD(SIZE_160B));
	
	obj3 = allocate(SIZE_TO_WORD(SIZE_1M));
	r_write(tmp3, obj3, SIZE_TO_WORD(SIZE_1M));

	obj4 = allocate(SIZE_TO_WORD(SIZE_4M));
	r_write(tmp4, obj4, SIZE_TO_WORD(SIZE_4M));

	while (!r_areq_completed());

	assertf(strlen(obj1) == SIZE_80B - 1, "Error in size %lu", strlen(obj1));
	assertf(strlen(obj2) == SIZE_160B - 1, "Error in size %lu", strlen(obj2));
	assertf(strlen(obj3) == SIZE_1M - 1, "Error in size %lu", strlen(obj3));
	assertf(strlen(obj4) == SIZE_4M - 1, "Error in size %lu", strlen(obj4));
	
	printf("------------------------------\n");
	printf("Test4:\t\t\t\033[1;32m[PASS]\033[0m\n");
	printf("------------------------------\n");
	
	return 0;
}
