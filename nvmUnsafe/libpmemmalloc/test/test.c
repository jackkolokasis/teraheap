#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../pmemalloc.h"
#include "../util.h"

#define MY_POOL_SIZE (1024 * 1024)
#define PATH "/mnt/pmemdir/file"
#define NPTRS 2

void
main(void)
{
    void *pmp;
    void *address_1;
    void *address_2;
    int i = 1234;
    int number;
    int number2;

    printf("Running test ....\n");

    /* Initialize Pool */
    pmp = pmemalloc_init(PATH, MY_POOL_SIZE);

    /* Malloc size for an integer */
    address_1 = pmemalloc_reserve(pmp, sizeof(int));
    memcpy(PMEM(pmp, address_1), &i, sizeof(int));

    /* Read this integer from memory */
    memcpy(&number, PMEM(pmp, address_1), sizeof(int));

    printf("The number is: %d\n", number);
    pmemalloc_activate(pmp, address_1);

    pmemalloc_check(PATH);

    //pmemalloc_free(pmp, address_1);
    
    i =5678;
    address_2 = pmemalloc_reserve(pmp, sizeof(int));
    memcpy(PMEM(pmp, address_2), &i, sizeof(int));
    pmemalloc_activate(pmp, address_2);

    memcpy(&number2, PMEM(pmp, address_2), sizeof(int));
    printf("The number is: %d\n", number2);

    pmemalloc_check(PATH);
    
    pmemalloc_check(PATH);
   return; 
}
