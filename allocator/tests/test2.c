/**************************************************
*
* file: test2.c
*
* @Author:   Iacovos G. Kolokasis
* @Version:  09-03-2021 
* @email:    kolokasis@ics.forth.gr
*
* Test to verify:
*	- explicit write using system call
*	- object allocation in the correct positions
*	- read object using mmap
***************************************************/

#include <stdint.h>
#include <stdio.h>
#include "../include/sharedDefines.h"
#include "../include/regions.h"

#define CARD_SIZE ((uint64_t) (1 << 9))
#define PAGE_SIZE ((uint64_t) (1 << 12))

int main() {
	char *obj1, *obj2, *obj3, *obj4;
	char tmp[80]; 
	char tmp2[160]; 
	char tmp3[1048576]; 
	char tmp4[4194304]; 
	
	// Init allocator
	init(CARD_SIZE * PAGE_SIZE);

	// Check start and stop adddresses
	printf("Start Address: %p\n", start_addr_mem_pool());
	printf("Stop Address: %p\n", stop_addr_mem_pool());
	printf("Mem Pool Size: %lu\n", mem_pool_size());
	
	memset(tmp, '1', 80);
	tmp[79] = '\0';

	memset(tmp2, '2', 160);
	tmp2[159] = '\0';

	memset(tmp3, '3', 1048576);
	tmp3[1048575] = '\0';

	memset(tmp4, '4', 4194304);
	tmp4[4194303] = '\0';
	
	obj1 = allocate(10);
	r_write(tmp, obj1, 10);
	assertf(strlen(obj1) == 79, "Error in size %lu", strlen(obj1));
	
	obj2 = allocate(20);
	r_write(tmp2, obj2, 20);
	assertf(strlen(obj2) == 159, "Error in size");
	
	obj3 = allocate(131072);
	r_write(tmp3, obj3, 131072);
	assertf(strlen(obj3) == 1048575, "Error in size %lu", strlen(obj3));

	obj4 = allocate(524288);
	r_write(tmp4, obj4, 524288);
	assertf(strlen(obj4) == 4194303, "Error in size");

	return 0;
}
