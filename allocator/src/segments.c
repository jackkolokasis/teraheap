#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include "../include/segments.h"
#include "../include/regions.h"
#include "../include/sharedDefines.h"

struct region region_array[REGION_ARRAY_SIZE];
struct group group_array[GROUP_ARRAY_SIZE];
uint64_t cur_group;
uint64_t cur_region;
uint64_t region_enabled;
uint64_t id_array[REGION_ARRAY_SIZE];

#if STATISTICS
double alloc_elapsedtime = 0.0;
double free_elapsedtime = 0.0;
#endif

int _next_region;						//< Get the next active region during 
										// region iteration

/*
 * Initialize region array, group array and their fields
 */
void init_regions(){
    int i;
    cur_region = 0;
    region_enabled = -1;
#if DEBUG_PRINT
    printf("Total regions:%lu\n",REGION_ARRAY_SIZE);
#endif
    for (i = 0; i < REGION_ARRAY_SIZE ; i++){
        if (i == 0)
            region_array[i].start_address = start_addr_mem_pool();
        else region_array[i].start_address = region_array[i-1].start_address + (uint64_t)REGION_SIZE; 
        region_array[i].used = 0;
        region_array[i].last_allocated_end = region_array[i].start_address;
        region_array[i].last_allocated_start = NULL;
        region_array[i].next_in_group = NULL;
        region_array[i].rdd_id = REGION_ARRAY_SIZE;
        id_array[i] = REGION_ARRAY_SIZE;
        region_array[i].group_id = -1;
    }
    for ( i = 0; i < GROUP_ARRAY_SIZE ; i++){
        group_array[i].region = NULL;
        group_array[i].num_of_references = 0;
    }
}

/*
 * Finds an empty region and returns its starting address
 * Arguments: size is the size of the object we want to allocate (in
 * Bytes)
 */
char* new_region(size_t size){
    int i;
    //for (i = cur_region ; i < cur_region+REGION_ARRAY_SIZE ; i++){
    for(i = 0 ; i < REGION_ARRAY_SIZE ; i++ ) {
        if (region_array[i % REGION_ARRAY_SIZE].last_allocated_end == region_array[i % REGION_ARRAY_SIZE].start_address) {
            cur_region = i % REGION_ARRAY_SIZE;
            mark_used(region_array[cur_region].start_address);
            region_array[cur_region].last_allocated_start = region_array[cur_region].last_allocated_end;
            region_array[cur_region].last_allocated_end = region_array[cur_region].start_address + size;
            return region_array[cur_region].start_address;
        }
    }
#if DEBUG_PRINT
    printf("No empty regions\n");
#endif
    return NULL;
}

#if SPARK_HINT
char* allocate_to_region(size_t size, uint64_t rdd_id){
#if STATISTICS
    struct timeval t1,t2;
    gettimeofday(&t1, NULL);
#endif
    assert(size <= (uint64_t)REGION_SIZE);
    if (id_array[rdd_id] == REGION_ARRAY_SIZE){
        char * res = new_region(size);
        id_array[rdd_id] = (res - region_array[0].start_address) / ((uint64_t)REGION_SIZE);
#if DEBUG_PRINT
        printf("RDD %zu in region %zu of tc\n",rdd_id, id_array[rdd_id]);
        fflush(stdout);
#endif
        region_array[id_array[rdd_id]].rdd_id = rdd_id;
#if STATISTICS
        gettimeofday(&t2, NULL);
        alloc_elapsedtime += (t2.tv_sec - t1.tv_sec) * 1000.0;
        alloc_elapsedtime += (t2.tv_usec - t1.tv_usec) / 1000.0;
#endif
        return res;
    }
    if (size > ((region_array[id_array[rdd_id]].start_address+(uint64_t)REGION_SIZE) - region_array[id_array[rdd_id]].last_allocated_end)){
#if STATISTICS
        printf("Wasting %luB in region %d, object is of size %zuB, last allocated is %p, start of next region is %p\n",((region_array[cur_region].start_address+(uint64_t)REGION_SIZE * 1024 * 1024) - region_array[cur_region].last_allocated_end), cur_region, size, region_array[cur_region].last_allocated_end, region_array[cur_region+1].start_address);
#endif
        char * res = new_region(size);
        id_array[rdd_id] = (res - region_array[0].start_address) / ((uint64_t)REGION_SIZE);
#if DEBUG_PRINTS
        printf("RDD %zu in region %zu of tc\n",rdd_id, id_array[rdd_id]);
        fflush(stdout);
#endif
        region_array[id_array[rdd_id]].rdd_id = rdd_id;
#if STATISTICS
        gettimeofday(&t2, NULL);
        alloc_elapsedtime += (t2.tv_sec - t1.tv_sec) * 1000.0;
        alloc_elapsedtime += (t2.tv_usec - t1.tv_usec) / 1000.0;
#endif
        return res;
    }
    mark_used(region_array[id_array[rdd_id]].start_address);
    region_array[id_array[rdd_id]].last_allocated_start = region_array[id_array[rdd_id]].last_allocated_end;
    region_array[id_array[rdd_id]].last_allocated_end = region_array[id_array[rdd_id]].last_allocated_start + size;
#if STATISTICS
    gettimeofday(&t2, NULL);
    alloc_elapsedtime += (t2.tv_sec - t1.tv_sec) * 1000.0;
    alloc_elapsedtime += (t2.tv_usec - t1.tv_usec) / 1000.0;
#endif
    return region_array[id_array[rdd_id]].last_allocated_start;

}
#else
/*
 * returns true if the object fits in the current region false
 * otherwise
 * arguments: size: the size of the object in Bytes
 */
char* allocate_to_region(size_t size){
    //FOR DEBUGGING ONLY
    #if GROUP_DEBUG
    cur_region = 0;
    while (region_array[i % REGION_ARRAY_SIZE].last_allocated_end == region_array[i % REGION_ARRAY_SIZE].start_address)
        cur_region++;
#if DEBUG_PRINT
    printf("ALLOCATING IN REGION %d\n",cur_region);
    printf("Allocating at address %p, object of size %zu\n",region_array[cur_region].last_allocated_end, size);
#endif
    mark_used(region_array[cur_region].start_address);
    region_array[cur_region].last_allocated_start = region_array[cur_region].last_allocated_end;
    region_array[cur_region].last_allocated_end = region_array[cur_region].start_address + size;
    return region_array[cur_region].start_address;
    #endif
    assert(size <= (uint64_t)REGION_SIZE);
    if (size > ((region_array[cur_region].start_address+(uint64_t)REGION_SIZE) - region_array[cur_region].last_allocated_end)){
#if STATISTICS
        printf("Wasting %luB in region %d, object is of size %zuB, last allocated is %p, start of next region is %p\n",((region_array[cur_region].start_address+(uint64_t)REGION_SIZE * 1024 * 1024) - region_array[cur_region].last_allocated_end), cur_region, size, region_array[cur_region].last_allocated_end, region_array[cur_region+1].start_address);
#endif
        char * res = new_region(size);
        return res;
    }
    mark_used(region_array[cur_region].start_address);
    region_array[cur_region].last_allocated_start = region_array[cur_region].last_allocated_end;
    region_array[cur_region].last_allocated_end = region_array[cur_region].last_allocated_start + size;
    return region_array[cur_region].last_allocated_start;
}
#endif
/*
 * function that connects two regions in a group
 * arguments: obj1: the object that references the other
 * obj2 the object that is referenced (order does not matter)
 */
void references(char *obj1, char *obj2){
    
    uint64_t seg1 = (obj1 - region_array[0].start_address) / ((uint64_t)REGION_SIZE);
    uint64_t seg2 = (obj2 - region_array[0].start_address) / ((uint64_t)REGION_SIZE);
    if (seg1 >= REGION_ARRAY_SIZE || seg2 >= REGION_ARRAY_SIZE){ 
        return;
    }
    if (seg1 == seg2)
        return;
    if (region_array[seg1].group_id == -1 && region_array[seg2].group_id == -1){
        region_array[seg1].group_id = new_group();
        region_array[seg2].group_id = region_array[seg1].group_id;
        group_array[region_array[seg1].group_id].region = &region_array[seg1];
        region_array[seg1].next_in_group = &region_array[seg2];
        group_array[region_array[seg1].group_id].num_of_references += region_array[seg1].used;
        group_array[region_array[seg2].group_id].num_of_references += region_array[seg2].used;
    }
    else if (region_array[seg1].group_id == -1 || region_array[seg2].group_id == -1){
        if (region_array[seg1].group_id == -1){
            struct region *tmp = region_array[seg2].next_in_group;
            region_array[seg2].next_in_group = &region_array[seg1];
            region_array[seg1].next_in_group = tmp;
            region_array[seg1].group_id = region_array[seg2].group_id;
            group_array[region_array[seg1].group_id].num_of_references += region_array[seg1].used;
        } else {
            struct region *tmp = region_array[seg1].next_in_group;
            region_array[seg1].next_in_group = &region_array[seg2];
            region_array[seg2].next_in_group = tmp;
            region_array[seg2].group_id = region_array[seg1].group_id;
            group_array[region_array[seg1].group_id].num_of_references += region_array[seg2].used;
        }
    } else if (region_array[seg1].group_id == region_array[seg2].group_id ) {
        return;
    } else {
        merge_groups(region_array[seg1].group_id,region_array[seg2].group_id);
    }
}


/*
 * function that connects two regions in a group
 * arguments: obj: the object that must be checked to be groupped with the region_enabled
 */
void check_for_group(char *obj){
    uint64_t seg1 = region_enabled;
    uint64_t seg2 = (obj - region_array[0].start_address) / ((uint64_t)REGION_SIZE);
    if (seg1 >= REGION_ARRAY_SIZE || seg2 >= REGION_ARRAY_SIZE )
       return;
    if (seg1 == seg2)
        return;
    if (region_array[seg1].group_id == -1 && region_array[seg2].group_id == -1){
        region_array[seg1].group_id = new_group();
        region_array[seg2].group_id = region_array[seg1].group_id;
        group_array[region_array[seg1].group_id].region = &region_array[seg1];
        region_array[seg1].next_in_group = &region_array[seg2];
        group_array[region_array[seg1].group_id].num_of_references += region_array[seg1].used;
        group_array[region_array[seg2].group_id].num_of_references += region_array[seg2].used;
    }
    else if (region_array[seg1].group_id == -1 || region_array[seg2].group_id == -1){
        if (region_array[seg1].group_id == -1){
            struct region *tmp = region_array[seg2].next_in_group;
            region_array[seg2].next_in_group = &region_array[seg1];
            region_array[seg1].next_in_group = tmp;
            region_array[seg1].group_id = region_array[seg2].group_id;
            group_array[region_array[seg1].group_id].num_of_references += region_array[seg1].used;
        } else {
            struct region *tmp = region_array[seg1].next_in_group;
            region_array[seg1].next_in_group = &region_array[seg2];
            region_array[seg2].next_in_group = tmp;
            region_array[seg2].group_id = region_array[seg1].group_id;
            group_array[region_array[seg1].group_id].num_of_references += region_array[seg2].used;
        }
    } else if (region_array[seg1].group_id == region_array[seg2].group_id ) {
        return;
    } else {
        merge_groups(region_array[seg1].group_id,region_array[seg2].group_id);
    }
}


/*
 * returns an empty position of the group_array
 */
int new_group(){
    int i;
    for (i = cur_group ; i < cur_group+GROUP_ARRAY_SIZE ; i++){
        if (group_array[i % GROUP_ARRAY_SIZE].region == NULL){
            return i % GROUP_ARRAY_SIZE;
        }
    }
    return -1;
}

/* merges two groups of regions that already exist.
 * arguments: group1: the id of the first group
 * group2: the id of the second group
 */
void merge_groups(int group1, int group2){
    struct region *ptr = group_array[group1].region;
    int group_id = group_array[group1].region->group_id;
    group_array[group1].num_of_references += group_array[group2].num_of_references;
    while (ptr->next_in_group != NULL){
       ptr = ptr->next_in_group;
    }
    ptr->next_in_group = group_array[group2].region;
    group_array[group2].region = NULL;
    group_array[group2].num_of_references = 0;
    while (ptr != NULL){
        ptr->group_id = group_id;
        ptr = ptr->next_in_group;
    }
}


/**
 * Get total number of groups
 */
long get_total_groups() {
    int i;
    long counter = 0;

    for (i = 0; i < GROUP_ARRAY_SIZE; i++) {
		if (group_array[i].region != NULL)
			counter++;
    }

	return counter;
}

/*
 * prints all the region groups that contain something
 */
void print_groups() {
    int i;

    fprintf(stderr, "Groups:\n");

    for (i = 0; i < GROUP_ARRAY_SIZE ; i++){
        if (group_array[i].region != NULL){
            struct region *ptr = group_array[i].region;

            fprintf(stderr, "Group %d \n",i);

            while (ptr != NULL){
                fprintf(stderr, "Region %lu\n", ptr-region_array);
                ptr = ptr->next_in_group;
            }

            fprintf(stderr, "%d references\n", group_array[i].num_of_references);
        }
    }
}

/*
 * Resets the used field of all regions and groups
 */
void reset_used(){
    int i;
    for (i = 0 ; i < REGION_ARRAY_SIZE ; i++){
        region_array[i].used = 0;
    }
    for (i = 0 ; i < GROUP_ARRAY_SIZE ; i++){
        group_array[i].num_of_references = 0;
    }
}

/*
 * Marks the region that contains this obj as used and increases group
 * counter (if it belongs to a group)
 * Arguments: obj: the object that is alive
 */
void mark_used(char *obj){
    uint64_t seg = (obj - region_array[0].start_address) / ((uint64_t)REGION_SIZE);
    if (region_array[seg].group_id != -1){
        group_array[region_array[seg].group_id].num_of_references -= region_array[seg].used;
        region_array[seg].used = 1;
        group_array[region_array[seg].group_id].num_of_references += 1;
    } else {
        region_array[seg].used = 1;
    }
}

#if STATISTICS
void print_statistics(){
    uint64_t wasted_space = 0;
    uint64_t total_regions = 0;
    int i;
    int flag = 0;
    for(i = 0 ; i < REGION_ARRAY_SIZE ; i++ ) {
        if (region_array[i % REGION_ARRAY_SIZE].last_allocated_end != region_array[i % REGION_ARRAY_SIZE].start_address) {
            total_regions++;
            wasted_space += (region_array[i].start_address + (uint64_t) REGION_SIZE) - region_array[i].last_allocated_end; 
            if (flag == 0){
                flag++;
            }
        }
    }
    printf("Total Wasted Space: %zu Bytes\n",wasted_space);
    printf("Total regions: %zu\n",total_regions);
    printf("Average wasted space: %zu Bytes\n",total_regions);
    printf("Total time spent in allocate_to_region:%f ms\n",alloc_elapsedtime);
    printf("Total time spent in free_regions:%f ms\n",free_elapsedtime);
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
        if (region_array[i].used == 0 && region_array[i].last_allocated_end != region_array[i].start_address && region_array[i].group_id == -1){
            struct region_list *new_node = malloc(sizeof(struct region_list));
            new_node->start = region_array[i].start_address;
            new_node->end = region_array[i].last_allocated_start;
            new_node->next = head;
            head = new_node;
            region_array[i].last_allocated_end = region_array[i].start_address;
            region_array[i].last_allocated_start = NULL;
            fprintf(stderr, "Freeing region %d \n", i);
        }
    }
    for (i = 0; i < GROUP_ARRAY_SIZE ; i++){
        if (group_array[i].region != NULL && group_array[i].num_of_references == 0){
            struct region *ptr = group_array[i].region;
            struct region *prev = NULL;
            while (ptr != NULL){
                struct region_list *new_node = malloc(sizeof(struct region_list));
                new_node->start = ptr->start_address;
                new_node->end = ptr->last_allocated_start;
                new_node->next = head;
                head = new_node;
                ptr->group_id = -1;
                ptr->last_allocated_end = ptr->start_address;
                ptr->last_allocated_start = NULL;
                prev = ptr;
                fprintf(stderr, "Freeing region from group: %lu \n", ptr-region_array);
                ptr = ptr->next_in_group;
                prev->next_in_group = NULL;
            }
            group_array[i].region = NULL;
            group_array[i].num_of_references = 0;
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
 * Get the total allocated regions
 * Return the total number of allocated regions or zero, otherwise
 */
long total_allocated_regions() {
	int i;
	long counter = 0;

	for (i = 0; i < REGION_ARRAY_SIZE ; i++){
		if (region_array[i].last_allocated_end != region_array[i].start_address)
			counter++;
	}

	return counter;
}

/*
 * Prints all the allocated regions
 */
void print_regions(){
    int i;
    fprintf(stderr, "Regions:\n");
    for (i = 0; i < REGION_ARRAY_SIZE ; i++){
        if (region_array[i].last_allocated_end != region_array[i].start_address)
            fprintf(stderr, "Region %d\n", i);
    }
}

/*
 * Get the total number of used regions
 * Return the total number of used regions or zero, otherwise
 */
long total_used_regions() {
    int i;
	long counter = 0;

	for (i = 0 ; i < REGION_ARRAY_SIZE ; i++) {
		if (region_array[i].used == 1)
			counter++;
	}

	return counter;
}

/*
 * Prints all the used regions
 */
void print_used_regions(){
    int i;
    printf("Used Regions:\n");
    for (i = 0 ; i < REGION_ARRAY_SIZE ; i++){
        if (region_array[i].used == 1)
            printf("Region %d\n", i);
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
    if (obj >= region_array[seg].last_allocated_end){
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
    uint64_t seg = (obj - region_array[0].start_address) / ((uint64_t)REGION_SIZE);
    if (region_array[seg].rdd_id == 29 || region_array[seg].rdd_id == 42){
        printf("Object name: %s\n",string);
    }
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
