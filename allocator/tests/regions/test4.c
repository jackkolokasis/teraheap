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
	int i;
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
	
	obj1 = allocate(10);
	r_awrite(tmp, obj1, 10);
	
	obj2 = allocate(20);
	r_awrite(tmp2, obj2, 20);
	
	obj3 = allocate(131072);
	r_awrite(tmp3, obj3, 131072);
	
	obj4 = allocate(524288);
	r_awrite(tmp4, obj4, 524288);

	while (!r_areq_completed());

	assertf(strlen(obj1) == 79, "Error in size %lu", strlen(obj1));
	assertf(strlen(obj2) == 159, "Error in size");
	assertf(strlen(obj3) == 1048575, "Error in size");
	assertf(strlen(obj4) == 4194303, "Error in size");
	
	for (i = 0; i < 4096; i++) {
		obj1 = allocate(10);
		r_awrite(tmp, obj1, 10);

		obj2 = allocate(20);
		r_awrite(tmp2, obj2, 20);

		obj3 = allocate(131072);
		r_awrite(tmp3, obj3, 131072);

		obj4 = allocate(524288);
		r_awrite(tmp4, obj4, 524288);
	}

	while (!r_areq_completed());

	assertf(strlen(obj1) == 79, "Error in size %lu", strlen(obj1));
	assertf(strlen(obj2) == 159, "Error in size");
	assertf(strlen(obj3) == 1048575, "Error in size");
	assertf(strlen(obj4) == 4194303, "Error in size");
	return 0;
}
