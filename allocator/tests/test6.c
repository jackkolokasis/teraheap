
/**************************************************
*
* file: test6.c
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
* Scenario: Allocate 4 small size objects (100MB)
* and one object that need to be allocated in the next region
***************************************************/

#include <stdint.h>
#include <stdio.h>
#include "../include/sharedDefines.h"
#include "../include/regions.h"

#define CARD_SIZE ((uint64_t) (1 << 9))
#define PAGE_SIZE ((uint64_t) (1 << 12))

#define SIZE_100MB (100*1024LU*1024)
#define SIZE_200MB (200*1024LU*1024)
#define HEAPWORD (8)

#define SIZE_TO_WORD(SIZE) \
	((size_t) (SIZE / HEAPWORD))

int main() {
	char *obj1, *obj2, *obj3, *obj4, *obj5, *obj6;
	char *tmp;
	
	// Init allocator
	init(CARD_SIZE * PAGE_SIZE);
	
	tmp = malloc(SIZE_100MB * sizeof(char));
	memset(tmp, '1', SIZE_100MB);
	tmp[SIZE_100MB - 1] = '\0';

	obj1 = allocate(SIZE_TO_WORD(SIZE_100MB));
	r_awrite(tmp, obj1, SIZE_TO_WORD(SIZE_100MB));
	free(tmp);
	assertf((obj1 - start_addr_mem_pool()) == 0, "Object start position");
	
	tmp = malloc(SIZE_100MB * sizeof(char));
	memset(tmp, '1', SIZE_100MB);
	tmp[SIZE_100MB - 1] = '\0';
	
	obj2 = allocate(SIZE_TO_WORD(SIZE_100MB));
	r_awrite(tmp, obj2, SIZE_TO_WORD(SIZE_100MB));
	free(tmp);
	assertf((obj2 - obj1) == SIZE_100MB, "Object start position");
	
	tmp = malloc(SIZE_100MB * sizeof(char));
	memset(tmp, '1', SIZE_100MB);
	tmp[SIZE_100MB - 1] = '\0';
	
	obj3 = allocate(SIZE_TO_WORD(SIZE_100MB));
	r_awrite(tmp, obj3, SIZE_TO_WORD(SIZE_100MB));
	free(tmp);
	assertf((obj3 - obj2) == SIZE_100MB, "Object start position");
	
	tmp = malloc(SIZE_100MB * sizeof(char));
	memset(tmp, '1', SIZE_100MB);
	tmp[SIZE_100MB - 1] = '\0';
	
	obj4 = allocate(SIZE_TO_WORD(SIZE_100MB));
	r_awrite(tmp, obj4, SIZE_TO_WORD(SIZE_100MB));
	free(tmp);
	assertf((obj4 - obj3) == SIZE_100MB, "Object start position");

	tmp = malloc(SIZE_200MB * sizeof(char));
	memset(tmp, '1', SIZE_200MB);
	tmp[SIZE_200MB - 1] = '\0';

#if ALIGN_ON
	if (r_is_obj_fits_in_region(SIZE_TO_WORD(SIZE_200MB))) {
		obj5 = allocate(SIZE_TO_WORD(SIZE_200MB));
	}
	else {
		char *cur = cur_alloc_ptr();
		char *top = r_region_top_addr();

		assertf((top - cur) == (112 * 1024 * 1024),
				"Error in available free space: %lu", (top - cur) / 1024 / 1024);

		memset(cur, '1', top - cur);
		obj5 = allocate(SIZE_TO_WORD(SIZE_200MB));
	}
#else
	obj5 = allocate(SIZE_TO_WORD(SIZE_200MB));
#endif
		
	r_awrite(tmp, obj5, SIZE_TO_WORD(SIZE_200MB));
	free(tmp);
#if ALIGN_ON
	assertf((obj5 - obj4) == (212*1024*1024), "Object start position %p | %p",
			obj5, obj4);
#else
	assertf((obj5 - obj4) == SIZE_100MB, "Object start position %p | %p",
			obj5, obj4);
#endif
	
	tmp = malloc(SIZE_200MB * sizeof(char));
	memset(tmp, '1', SIZE_200MB);
	tmp[SIZE_200MB - 1] = '\0';
	
#if ALIGN_ON
	if (r_is_obj_fits_in_region(SIZE_TO_WORD(SIZE_200MB))) {
		obj6 = allocate(SIZE_TO_WORD(SIZE_200MB));
	}
	else {
		char *cur = cur_alloc_ptr();
		char *top = r_region_top_addr();

		assertf((top - cur) == (112 * 1024 * 1024),
				"Error in available free space: %lu", (top - cur) / 1024 / 1024);

		// Fill the rest region area with a dummy
		// object
		memset(cur, '1', top - cur);
		obj6 = allocate(SIZE_TO_WORD(SIZE_200MB));
	}
#else
	obj6 = allocate(SIZE_TO_WORD(SIZE_200MB));
#endif
	
	r_awrite(tmp, obj6, SIZE_TO_WORD(SIZE_200MB));
	free(tmp);
	assertf((obj6 - obj5) == SIZE_200MB, "Object start position %p | %p",
			obj6, obj5);
	
    // Wait all the asynchronous request to
	// complete and then try to access the data
	while (!r_areq_completed());

	assertf(strlen(obj1) == SIZE_100MB - 1, "Error in size %lu", strlen(obj1));
	assertf(strlen(obj2) == SIZE_100MB - 1, "Error in size %lu", strlen(obj2));
	assertf(strlen(obj3) == SIZE_100MB - 1, "Error in size %lu", strlen(obj3));
	assertf(strlen(obj4) == SIZE_100MB - 1, "Error in size %lu", strlen(obj4));
	assertf(strlen(obj5) == SIZE_200MB - 1, "Error in size %lu", strlen(obj5));
	assertf(strlen(obj6) == SIZE_200MB - 1, "Error in size %lu", strlen(obj6));
	
	printf("------------------------------\n");
	printf("Test6:\t\t\t\033[1;32m[PASS]\033[0m\n");
	printf("------------------------------\n");

	return 0;
}
