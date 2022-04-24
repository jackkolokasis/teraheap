#ifndef __SEGMENTS_H__
#define __SEGMENTS_H__

#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>

#define SPARK_HINT 1
#define ONE_WAY 1

struct offset{
    uint64_t offset;
    struct offset *next;
};

/*
 * the struct for regions
 */
#if ONE_WAY
struct region{
    char *start_address;
    uint64_t used;
    char *last_allocated_end;
    char *last_allocated_start;
    char *first_allocated_start;
    size_t size_mapped;
    struct group *dependency_list;
    uint64_t rdd_id;
    struct offset *offset_list;
    uint64_t part_id;
};
#else
struct region{
    char *start_address;
    uint64_t used;
    char *last_allocated_end;
    char *last_allocated_start;
    char *first_allocated_start;
    struct region *next_in_group;
    int group_id;
    uint64_t rdd_id;
    uint64_t part_id;
};
#endif

/*
 * the struct for group array
 */
#if ONE_WAY
struct group{
    struct region *region;
    struct group *next;
};
#else
struct group{
    struct region *region;
    int num_of_references;
};
#endif

/*
 * Initialize region array, group array and their fields
 */
void init_regions();

/*
 * Finds an empty regions and returns its starting address
 * Arguments: size: the size of the object we want to allocate in
 * Bytes
 */
char* new_region(size_t size);

#if SPARK_HINT
/*
 * Returns the address of the allocated object
 * Arguments: size: the size of the object in Bytes
 * rdd_id: The id of the rdd which the object belongs
 * part_id: The id of the partition that the object belongs
 */
char* allocate_to_region(size_t size, uint64_t rdd_id, uint64_t partition_id);

uint64_t get_id(uint64_t rdd_id, uint64_t partition_id);
#else
/*
 * Returns the address of the allocated object 
 * Arguments: size: the size of the object in Bytes
 */
char* allocate_to_region(size_t size);
#endif

/*
 * Returns an empty position of the group_array
 */
int new_group();

/*
 * Merges two groups of regions that already exist
 * Arguments: group1: the id of the first group
 * group2:the id of the second group
 */
void merge_groups(int group1, int group2);

/*
 * Connects two regions in a group
 * Arguments: obj1: the object that references the other
 * obj2: the object that is referenced (order does not matter)
 */
void references(char *obj1, char *obj2);

/**
 * Get the total number of groups
 * Return number of groups that exist, or zero otherwise
 */
long get_total_groups();

/*
 * Prints all the region groups that contain something
 */
void print_groups();

/*
 * Resets the used field of all regions, and groups
 */
void reset_used();

/*
 * Marks the region that contains this obj as used and increases group
 * counter (if it belongs to a group)
 * Arguments: obj:the object that is alive
 */
void mark_used(char *obj);

/*
 * Frees all unused regions
 */
struct region_list* free_regions();

/**
 * Get the total number of allocated regions
 * Return the total number of allocated regions or zero, otherwise
 */
long total_allocated_regions();

/*
 * Prints all the allocated regions
 */
void print_regions();

/*
 * Get the total number of used regions
 * Return the number of used regions or zero, otherwise
 */
long total_used_regions();

/*
 * Prints all the used regions
 */
void print_used_regions();

/*
 * Checks if address of obj is before last object
 */
bool is_before_last_object(char *obj);

/*
 * Returns last object of region
 */
char* get_last_object(char *obj);

/*
 * Returns true if object is first of its region, false otherwise
 */
bool is_region_start(char *obj);

/*
 * Enables groupping with the region in which obj belongs to
 */
void enable_region_groups(char *obj);

/*
 * Disables groupping with the region previously enabled
 */
void disable_region_groups(void);

/*
 * function that connects two regions in a group
 * arguments: obj: the object that must be checked to be groupped with the region_enabled
 */
void check_for_group(char *obj);

void print_objects_temporary_function(char *obj,const char *string);

/*
 * Start iteration over all active regions to print their object state
 */
void start_iterate_regions(void);

/*
 * Get the next active region
 */
char* get_next_region(void);

char *get_first_object(char *addr);

/*
 * Get objects 'obj' region start address
 */
char* get_region_start_addr(char *obj, long rdd_id, long part_id);

/*
 * Get object 'groupId' (RDD Id). Each object is allocated based on a group Id
 * and the partition Id that locates in teraflag.
 *
 * @obj: address of the object
 *
 * Return: the object partition Id
 */
uint64_t get_obj_group_id(char *obj);

/*
 * Get object 'groupId'. Each object is allocated based on a group Id
 * and the partition Id that locates in teraflag.
 *
 * obj: address of the object
 *
 * returns: the object partition Id
 */
uint64_t get_obj_part_id(char *obj);

/*
 * Check if these two objects belong to the same group
 *
 * obj1: address of the object
 * obj2: address of the object
 *
 * returns: 1 if objects are in the same group, 0 otherwise
 */
uint64_t is_in_the_same_group(char *obj1, char *obj2);

#endif