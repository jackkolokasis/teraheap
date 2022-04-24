#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <limits.h>
#include "../include/segments.h"
#include "../include/regions.h"
#include "../include/sharedDefines.h"

struct region region_array[REGION_ARRAY_SIZE];
uint64_t cur_group;
uint64_t cur_region;
int region_enabled;
struct region *id_array[MAX_PARTITIONS*MAX_RDD_ID];
struct offset *offset_list;
#if STATISTICS
double alloc_elapsedtime = 0.0;
double free_elapsedtime = 0.0;
unsigned int total_deps = 0;
#endif

int _next_region;

/*
 * Initialize region array, group array and their fields
 */
void init_regions(){
    uint64_t i;
    cur_region = 0;
    region_enabled = -1;
    offset_list = NULL;
#if DEBUG_PRINT
    printf("Total num of regions:%lu\n",REGION_ARRAY_SIZE);
#endif
    for (i = 0; i < REGION_ARRAY_SIZE ; i++){
        if (i == 0)
            region_array[i].start_address = start_addr_mem_pool();
        else 
			      region_array[i].start_address = region_array[i-1].start_address + (uint64_t)REGION_SIZE; 
        region_array[i].used = 0;
        region_array[i].last_allocated_end = region_array[i].start_address;
        region_array[i].last_allocated_start = NULL;
        region_array[i].first_allocated_start = NULL;
        region_array[i].dependency_list = NULL;
        region_array[i].size_mapped = 0;
        region_array[i].offset_list = NULL;
        region_array[i].rdd_id = MAX_PARTITIONS*MAX_RDD_ID;
        region_array[i].part_id = MAX_PARTITIONS*MAX_RDD_ID;
    }
    for (i = 0 ; i < (MAX_PARTITIONS*MAX_RDD_ID) ; i++){
        id_array[i] = NULL;
    }
  
    struct offset *prev = NULL;
    for (i = 0 ; i < DEV_SIZE / MMAP_SIZE ; i++){
        struct offset *ptr = malloc(sizeof(struct offset));
        ptr->offset = MMAP_SIZE * i;
        ptr->next = NULL;
        if (offset_list == NULL){
            offset_list = ptr;
        } else {
            prev->next = ptr;
        }
        prev = ptr;
    }
}

/*
 * Returns the start of cont_regions empty regions
 */
unsigned int get_cont_regions(unsigned int cont_regions){
    unsigned int i;
    unsigned int j;
    for(i = 0 ; i < REGION_ARRAY_SIZE ; i++ ) {
        for (j = i ; j < i + cont_regions ; j++) {
            if (region_array[i % REGION_ARRAY_SIZE].last_allocated_end == region_array[i % REGION_ARRAY_SIZE].start_address) {
                continue;
            }
            else {
                break;
            }
        }
        if (j == i + cont_regions){
            return i;
        } else {
            i = j;
        }
    }
    return -1;
}
            


/*
 * Finds an empty region and returns its starting address
 * Arguments: size is the size of the object we want to allocate (in
 * Bytes)
 */
char* new_region(size_t size){
    unsigned int i;
    unsigned int cont_regions = (size / REGION_SIZE) + 1;
    cur_region = get_cont_regions(cont_regions) % REGION_ARRAY_SIZE;
    if (cur_region == -1 )
        return NULL;
    for (i = cur_region ; i < cur_region + cont_regions ; i++){
        references(region_array[cur_region].start_address,region_array[i].start_address);
        mark_used(region_array[i].start_address);
        region_array[i].last_allocated_start = region_array[cur_region].start_address;
        region_array[i].first_allocated_start = region_array[cur_region].start_address;
        region_array[i].last_allocated_end = region_array[cur_region].start_address + size;
    }
    return region_array[cur_region].start_address;
#if DEBUG_PRINT
    printf("No empty regions\n");
#endif
}

uint64_t get_id(uint64_t rdd_id, uint64_t partition_id){
    return (rdd_id % MAX_RDD_ID) * MAX_PARTITIONS + partition_id;
}

char* allocate_to_region(size_t size, uint64_t rdd_id, uint64_t partition_id){
#if STATISTICS
    struct timeval t1,t2;
    gettimeofday(&t1, NULL);
#endif
#if ANONYMOUS
    assert(size <= (uint64_t)REGION_SIZE);
#endif
    uint64_t id_index = get_id(rdd_id, partition_id);
    if (id_array[id_index] == NULL){
        char * res = new_region(size);
        id_array[id_index] = &region_array[((res+size) - region_array[0].start_address) / ((uint64_t)REGION_SIZE)];
        id_array[id_index]->rdd_id = rdd_id;
        region_array[i].part_id = partition_id;
#if STATISTICS
        gettimeofday(&t2, NULL);
        alloc_elapsedtime += (t2.tv_sec - t1.tv_sec) * 1000.0;
        alloc_elapsedtime += (t2.tv_usec - t1.tv_usec) / 1000.0;
#endif
#if ANONYMOUS
        uint64_t i = 0;
        struct offset *mmap_offset = offset_list;
        for (i = 0; i < (size/MMAP_SIZE)+1 ; i++){
            struct offset *tmp = offset_list;
            assert(tmp != NULL);
            offset_list = offset_list->next;
            tmp->next = id_array[id_index]->offset_list;
            id_array[id_index]->offset_list = tmp;
        }
        char *address_mmapped = mmap(res, MMAP_SIZE * ((size/MMAP_SIZE)+1), PROT_READ|PROT_WRITE, MAP_SHARED | MAP_FIXED, fd, mmap_offset->offset);
        id_array[id_index]->size_mapped += MMAP_SIZE * ((size/MMAP_SIZE)+1);
        if (address_mmapped == MAP_FAILED){
            fprintf(stderr, "mmap to file failed 1\n");
        }
#endif
        return res;
    }
  
    if ((id_array[id_index]->start_address+(uint64_t)REGION_SIZE) < id_array[id_index]->last_allocated_end 
        || size > ((id_array[id_index]->start_address+(uint64_t)REGION_SIZE) - id_array[id_index]->last_allocated_end)) {
        char * res = new_region(size);
        id_array[id_index] = &region_array[((res+size) - region_array[0].start_address) / ((uint64_t)REGION_SIZE)];
        id_array[id_index]->rdd_id = rdd_id;
        id_array[id_index]->part_id = partition_id;
        assertf(res != NULL, "No empty region");
      
#if STATISTICS
        gettimeofday(&t2, NULL);
        alloc_elapsedtime += (t2.tv_sec - t1.tv_sec) * 1000.0;
        alloc_elapsedtime += (t2.tv_usec - t1.tv_usec) / 1000.0;
#endif
#if ANONYMOUS
        uint64_t i = 0;
        for (i = 0; i < (size/MMAP_SIZE)+1 ; i++){
            struct offset *tmp = offset_list;
            assert(tmp != NULL);
            offset_list = offset_list->next;
            tmp->next = id_array[id_index]->offset_list;
            id_array[id_index]->offset_list = tmp;
            char *address_mmapped = mmap(res + id_array[id_index]->size_mapped, MMAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED | MAP_FIXED, fd, tmp->offset);
            id_array[id_index]->size_mapped += MMAP_SIZE;
            if (address_mmapped == MAP_FAILED){
                fprintf(stderr, "mmap to file failed 2\n");
            }
        }
#endif
        return res;
    }
    mark_used(id_array[id_index]->start_address);
    id_array[id_index]->last_allocated_start = id_array[id_index]->last_allocated_end;
    id_array[id_index]->last_allocated_end = id_array[id_index]->last_allocated_start + size;
#if ANONYMOUS
    if (size > MMAP_SIZE || id_array[id_index]->last_allocated_end > id_array[id_index]->start_address+id_array[id_index]->size_mapped){
        size_t missing_size =  id_array[id_index]->last_allocated_end - (id_array[id_index]->start_address + id_array[id_index]->size_mapped); 
        uint64_t i = 0;
        for (i = 0; i < (missing_size/MMAP_SIZE)+1 ; i++){
            struct offset *tmp = offset_list;
            assert(tmp != NULL);
            offset_list = offset_list->next;
            tmp->next = id_array[id_index]->offset_list;
            id_array[id_index]->offset_list = tmp;
            void *address_mmapped = mmap(id_array[id_index]->start_address + id_array[id_index]->size_mapped, MMAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_FIXED, fd, tmp->offset);
            id_array[id_index]->size_mapped += MMAP_SIZE;
            if (address_mmapped == MAP_FAILED){
                fprintf(stderr, "mmap to file failed 3\n");
                fprintf(stderr, "Start of region:%p\n",id_array[id_index]->start_address);
                fprintf(stderr, "Last object ends at:%p\n",id_array[id_index]->last_allocated_start );
                fprintf(stderr, "MMAPS needed:%zu\n",((missing_size/MMAP_SIZE)+1));
                fprintf(stderr, "size:%zu\n",size);
                fprintf(stderr, "missing size:%zu\n",missing_size);
                return NULL;
            }
        }
    }
#endif
#if STATISTICS
    gettimeofday(&t2, NULL);
    alloc_elapsedtime += (t2.tv_sec - t1.tv_sec) * 1000.0;
    alloc_elapsedtime += (t2.tv_usec - t1.tv_usec) / 1000.0;
#endif
    return id_array[id_index]->last_allocated_start;
}

/*
 * function that connects two regions in a group
 * arguments: obj1: the object that references the other
 * obj2 the object that is referenced (order does not matter)
 */

void references(char *obj1, char *obj2){
    int seg1 = (obj1 - region_array[0].start_address) / ((uint64_t)REGION_SIZE);
    int seg2 = (obj2 - region_array[0].start_address) / ((uint64_t)REGION_SIZE);
    if (seg1 >= REGION_ARRAY_SIZE || seg2 >= REGION_ARRAY_SIZE || seg1 < 0 || seg2 < 0)
        return;
  
    if (seg1 == seg2)
        return;
  
    struct group *ptr = region_array[seg1].dependency_list;
    while (ptr != NULL){
        if (ptr->region == &region_array[seg2])
            break;
        ptr = ptr->next;
    }
    if (ptr)
        return;
    struct group *new = malloc(sizeof(struct group));
#if STATISTICS
    total_deps++;
#endif
    new->next = region_array[seg1].dependency_list;
    new->region = &region_array[seg2];
    region_array[seg1].dependency_list = new;
    if (region_array[seg1].used)
        mark_used(region_array[seg2].start_address);
}

/*
 * function that connects two regions in a group
 * arguments: obj: the object that must be checked to be groupped with the region_enabled
 */
void check_for_group(char *obj){
    int seg1 = region_enabled;
    int seg2 = (obj - region_array[0].start_address) / ((uint64_t)REGION_SIZE);
    if (seg1 >= REGION_ARRAY_SIZE || seg2 >= REGION_ARRAY_SIZE || seg1 < 0 || seg2 < 0){ 
        return;
    }
    if (seg1 == seg2)
        return;
    struct group *ptr = region_array[seg1].dependency_list;
    while (ptr != NULL){
        if (ptr->region == &region_array[seg2])
            break;
        ptr = ptr->next;
    }
    if (ptr)
        return;
    struct group *new = malloc(sizeof(struct group));
#if STATISTICS
    total_deps++;
#endif
    new->next = region_array[seg1].dependency_list;
    new->region = &region_array[seg2];
    region_array[seg1].dependency_list = new;
    if (region_array[seg1].used)
        mark_used(region_array[seg2].start_address);
}

/*
 * prints all the region groups that contain something
 */
void print_groups(){
    int i;
    fprintf(stderr, "Groups:\n");
    for (i = 0; i < REGION_ARRAY_SIZE ; i++){
        if (region_array[i].dependency_list != NULL){
            struct group *ptr = region_array[i].dependency_list;
            printf("Region %d depends on regions: \n",i);
            while (ptr != NULL){
                fprintf(stderr, "Region %lu\n", ptr->region-region_array);
                ptr = ptr->next;
            }
        }
    }
}

/*
 * Resets the used field of all regions and groups
 */
void reset_used(){
    int i;
    for (i = 0 ; i < REGION_ARRAY_SIZE ; i++)
        region_array[i].used = 0;
}

/*
 * Marks the region that contains this obj as used and increases group
 * counter (if it belongs to a group)
 * Arguments: obj: the object that is alive
 */
void mark_used(char *obj) {
    uint64_t seg = (obj - region_array[0].start_address) / ((uint64_t)REGION_SIZE);
    if (region_array[seg].used == 1)
        return;
    region_array[seg].used = 1;
    struct group *ptr = region_array[seg].dependency_list;
    while (ptr){
        mark_used(ptr->region->start_address);
        ptr = ptr->next;
    }
}

#if STATISTICS
void print_statistics(){
    uint64_t wasted_space = 0;
    uint64_t total_regions = 0;
    int i;
    int flag = 0;
    for(i = 0 ; i < REGION_ARRAY_SIZE ; i++ ) {
        if (region_array[i % REGION_ARRAY_SIZE].last_allocated_end != region_array[i % REGION_ARRAY_SIZE].start_address ) {
            total_regions++;
            if (region_array[i].last_allocated_end <= region_array[i].start_address + REGION_SIZE){ 
                wasted_space += (region_array[i].start_address + (uint64_t) REGION_SIZE) - region_array[i].last_allocated_end; 
                if (flag == 0){
                    flag++;
                }
            }
        }
    }
    fprintf(stderr, "Total Wasted Space: %zu GBytes\n",wasted_space/(1024*1024*1024));
    fprintf(stderr, "Total regions: %zu\n",total_regions);
    if (total_regions)
        fprintf(stderr, "Average wasted space: %zu MBytes\n",wasted_space/(1024*1024*total_regions));
    fprintf(stderr, "Total dependencies:%d\n",total_deps);
    fprintf(stderr, "Total time spent in allocate_to_region:%f ms\n",alloc_elapsedtime);
    fprintf(stderr, "Total time spent in free_regions:%f ms\n",free_elapsedtime);
}
#endif

/*
 * Frees all unused regions
 */
struct region_list* free_regions(){
#if STATISTICS
    struct timeval t1,t2;
    gettimeofday(&t1, NULL);
#endif
    int i;
    struct region_list *head = NULL;
    for (i = 0; i < REGION_ARRAY_SIZE ; i++){
        if (region_array[i].used == 0 && region_array[i].last_allocated_end != region_array[i].start_address){
            struct group *ptr = region_array[i].dependency_list;
            struct group *next = NULL;
            while (ptr != NULL) {
                next = ptr->next;
                free(ptr);
                ptr = next;
            }
            region_array[i].dependency_list = NULL;
            struct region_list *new_node = malloc(sizeof(struct region_list));
            new_node->start = region_array[i].start_address;
            new_node->end = region_array[i].last_allocated_start;
            new_node->next = head;
            head = new_node;
            region_array[i].last_allocated_end = region_array[i].start_address;
            //memset(region_array[i].start_address, 0, REGION_SIZE);
            region_array[i].last_allocated_start = NULL;
            region_array[i].first_allocated_start = NULL;
            
#if ANONYMOUS 
            region_array[i].size_mapped = 0;
            struct offset *offset_ptr = region_array[i].offset_list;
            struct offset *temp = offset_ptr;
            while (offset_ptr != NULL){
               temp = offset_ptr->next;
               offset_ptr->next = offset_list;
               offset_list = offset_ptr;
               offset_ptr = temp;
            }
            region_array[i].offset_list = NULL;
#endif
            if (id_array[region_array[i].rdd_id] == &region_array[i]){
                id_array[region_array[i].rdd_id] = NULL;
            }
            region_array[i].rdd_id = MAX_PARTITIONS * MAX_RDD_ID;
#if STATISTICS
            printf("Freeing region %d \n",i);
            fflush(stdout);
#endif
        }
    }
#if STATISTICS
    print_statistics();
    gettimeofday(&t2, NULL);
    free_elapsedtime += (t2.tv_sec - t1.tv_sec) * 1000.0;
    free_elapsedtime += (t2.tv_usec - t1.tv_usec) / 1000.0;
#endif
    return head;
}

/*
 * Prints all the allocated regions
 */
void print_regions(){
    int i;
    printf("Regions:\n");
    for (i = 0; i < REGION_ARRAY_SIZE ; i++){
        if (region_array[i].last_allocated_end != region_array[i].start_address)
            printf("Region %d\n",i);
    }
}

/*
 * Prints all the used regions
 */
void print_used_regions(){
    int i;
    printf("Used Regions:\n");
    for (i = 0 ; i < REGION_ARRAY_SIZE ; i++){
        if (region_array[i].used == 1)
            printf("Region %d\n",i);
    }
    fflush(stdout);
}

/*
 * Checks if obj is before last object of region
 */
bool is_before_last_object(char *obj){
    #if !REGIONS
    return true;
    #endif
    uint64_t seg = (obj - region_array[0].start_address) / ((uint64_t)REGION_SIZE);
    if (obj >= region_array[seg].last_allocated_end || region_array[seg].last_allocated_end >= region_array[seg].start_address + REGION_SIZE){
        return false;
    } else {
        return true;
    }
}

/*
 * Returns last object of region
 */
char* get_last_object(char *obj){
    uint64_t seg = (obj - region_array[0].start_address) / ((uint64_t)REGION_SIZE);
    return region_array[seg].last_allocated_end;
}

// Returns true if object is first of its region false otherwise
bool is_region_start(char *obj){
    #if !REGIONS
        return false;
    #endif
    uint64_t seg = (obj - region_array[0].start_address) / ((uint64_t)REGION_SIZE);
    if (region_array[seg].start_address == obj)
        return true;
    return false;
}


/*
 * Enables groupping with the region in which obj belongs to
 */
void enable_region_groups(char *obj){
    region_enabled = (obj - region_array[0].start_address) / ((uint64_t)REGION_SIZE);
}

/*
 * Disables groupping with the region previously enabled
 */
void disable_region_groups(void){
    region_enabled = REGION_ARRAY_SIZE;
}


void print_objects_temporary_function(char *obj,const char *string){
    printf("Object name: %s\n",string);
}

/*
 * Start iteration over all active regions to print their object state.
 */
void start_iterate_regions() {
	_next_region = 0;
}

/*
 * Return the next active region or NULL if we reached the end of the region
 * array.
 */
char* get_next_region() {
	char *region_start_addr;

	// Find the next active region
	while (_next_region < REGION_ARRAY_SIZE &&
			region_array[_next_region].used == 0) {
		_next_region++; 
	}

	if (_next_region >= REGION_ARRAY_SIZE)
		return NULL;

	fprintf(stderr, "[PLACEMENT] Region: %d\n", _next_region);

	region_start_addr = region_array[_next_region].start_address;
	_next_region++;

	return region_start_addr;
}

char *get_first_object(char *addr){
    uint64_t seg = (addr - region_array[0].start_address) / ((uint64_t)REGION_SIZE);
    return region_array[seg].first_allocated_start;

/*
 * Get objects 'obj' region start address
 */
char* get_region_start_addr(char *obj, long rdd_id, long part_id) {
    #if !REGIONS
        return false;
    #endif
	int index;

	index = ((rdd_id % TOTAL_RDDS) * TOTAL_PARTITIONS) + part_id;

	return region_array[id_array[index]].start_address;
}

/*
 * Get object 'groupId' (RDD Id). Each object is allocated based on a group Id
 * and the partition Id that locates in teraflag.
 *
 * @obj: address of the object
 *
 * Return: the object rdd id
 */
uint64_t get_obj_group_id(char *obj) {
    uint64_t seg = (obj - region_array[0].start_address) / ((uint64_t)REGION_SIZE);
	return region_array[seg].rdd_id;

}

/*
 * Get object 'groupId'. Each object is allocated based on a group Id
 * and the partition Id that locates in teraflag.
 *
 * obj: address of the object
 *
 * returns: the object partition Id
 */
uint64_t get_obj_part_id(char *obj) {
    uint64_t seg = (obj - region_array[0].start_address) / ((uint64_t)REGION_SIZE);

	return region_array[seg].part_id;
}

/*
 * Check if these two objects belong to the same group
 *
 * obj1: address of the object
 * obj2: address of the object
 *
 * returns: 1 if objects are in the same group, 0 otherwise
 */
uint64_t is_in_the_same_group(char *obj1, char *obj2) {
	uint64_t seg1 = (obj1 - region_array[0].start_address) / ((uint64_t)REGION_SIZE);
	uint64_t seg2 = (obj2 - region_array[0].start_address) / ((uint64_t)REGION_SIZE);

	assertf(seg1 < REGION_ARRAY_SIZE && seg2 < REGION_ARRAY_SIZE, "Out of range"); 

    return (seg1 == seg2 || region_array[seg1].group_id == region_array[seg2].group_id);
}
