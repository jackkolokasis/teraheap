/**************************************************
*
* file: test5.c
*
* @Author:   Iacovos G. Kolokasis
* @Version:  20-09-2021 
* @email:    kolokasis@ics.forth.gr
*
* Test to verify:
*	- explicit write using system call with fastmap
*	- object allocation using 256MB alignment
*	  regions in the correct positions
*	- read object using mmap
*	- this test check small sized objects to fill
*	  a region in TeraCache
*
* Enable ALIGN
* Scenario: Allocate large size objects (512MB)
* that entirely fit in the region
***************************************************/

#include <stdint.h>
#include <stdio.h>
#include "../include/sharedDefines.h"
#include "../include/regions.h"

#define CARD_SIZE ((uint64_t) (1 << 9))
#define PAGE_SIZE ((uint64_t) (1 << 12))

#define SIZE_510MB (510*1024LU*1024)
#define HEAPWORD (8)

#define SIZE_TO_WORD(SIZE) \
	((size_t) (SIZE / HEAPWORD))

int main() {
	char *obj1, *obj2;
	char *tmp;
	
	// Init allocator
	init(CARD_SIZE * PAGE_SIZE);
	
	// Check start and stop adddresses
	tmp = malloc(SIZE_510MB * sizeof(char));
	memset(tmp, '1', SIZE_510MB);
	tmp[SIZE_510MB - 1] = '\0';

	// This object will be allocated in the first region
	obj1 = allocate(SIZE_TO_WORD(SIZE_510MB));
	r_awrite(tmp, obj1, SIZE_TO_WORD(SIZE_510MB));
	free(tmp);
	assertf((obj1 - start_addr_mem_pool()) == 0, "Object start position");
	
	tmp = malloc(SIZE_510MB * sizeof(char));
	memset(tmp, '1', SIZE_510MB);
	tmp[SIZE_510MB - 1] = '\0';

#if ALIGN_ON
	// Check if there is available space to allocate the next object in the same
	// region
	if (r_is_obj_fits_in_region(SIZE_TO_WORD(SIZE_510MB)))
		obj2 = allocate(SIZE_TO_WORD(SIZE_510MB));
	else {
		char *cur = cur_alloc_ptr();
		char *top = r_region_top_addr();

		assertf((top - cur) == (2 * 1024 * 1024),
				"Error in available free space: %lu", (top - cur) / 1024 / 1024);

		// Fill the rest region area with a dummy object
		memset(cur, '1', top - cur);
		obj2 = allocate(SIZE_TO_WORD(SIZE_510MB));
	}
#else
	obj2 = allocate(SIZE_TO_WORD(SIZE_510MB));
#endif
	
	r_awrite(tmp, obj2, SIZE_TO_WORD(SIZE_510MB));
	free(tmp);

#if ALIGN_ON
	assertf((obj2 - obj1) == (512 * 1024 * 1024), "Object start position");
#else
	assertf((obj2 - obj1) == SIZE_510MB, "Object start position");
#endif

	// Wait all the asynchronous request to complete and then try to access the
	// data
	while (!r_areq_completed());

	assertf(strlen(obj1) == SIZE_510MB - 1, "Error in size %lu", strlen(obj1));
	assertf(strlen(obj2) == SIZE_510MB - 1, "Error in size %lu", strlen(obj2));

	printf("------------------------------\n");
	printf("Test5:\t\t\t\033[1;32m[PASS]\033[0m\n");
	printf("------------------------------\n");

	return 0;
}
