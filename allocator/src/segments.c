#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h> 
#include <errno.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <limits.h>
#include "../include/segments.h"
#include "../include/regions.h"
#include "../include/sharedDefines.h"
#include <stdbool.h>
/*
#if PR_BUFFER
struct pr_buffer *pr_buffers = NULL;
#endif
*/
#if defined(H2_COMPACT_STATISTICS)
static inline double _get_total_elapsed_time(double *elapsed_times);
static inline size_t _get_total_operations(size_t *operations);
#endif
static inline double _get_elapsed_time_ms(struct timespec start_time, struct timespec end_time) {
    return  (double)((end_time.tv_sec - start_time.tv_sec) * 1000000000L + (end_time.tv_nsec - start_time.tv_nsec)) / 1000000L;
}
static inline double _get_elapsed_time_sec(struct timespec start_time, struct timespec end_time) {
    return  (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
}
struct region *region_array = NULL;
struct region **id_array = NULL;
struct offset *offset_list;

int32_t		 region_enabled;
int32_t		 _next_region;

uint32_t _GC_THREADS;//perpap
static bool _ASYNC_IS_ENABLED = true;//perpap

//static void (*write_to_h2[])(char *data, char *offset, size_t size) = { r_write, r_awrite };
static void (*write_to_h2)(char *data, char *offset, size_t size, uint64_t worker_id) = NULL;

#if STATISTICS
uint32_t    total_deps = 0;
double	alloc_elapsedtime = 0.0;
double  free_elapsedtime = 0.0;		
#endif

#if defined(H2_COMPACT_STATISTICS)
static double *_buffer_insert_elapsed_times = NULL;
static double *_flush_buffer_elapsed_times  = NULL;
//static double *_flush_buffer_fragmentation_elapsed_times = NULL;
//static double *_flush_buffer_nofreespace_elapsed_times = NULL;
//static double *_async_request_elapsed_times  = NULL;
static size_t *_buffer_insert_operations = NULL; 
static size_t *_flush_buffer_operations = NULL; 
//static size_t *_flush_buffer_nofreespace_operations = NULL; 
//static size_t *_flush_buffer_fragmentation_operations = NULL; 
//static size_t *_async_request_operations = NULL; 
#endif

#if defined(H2_COMPACT_STATISTICS)
double _get_total_elapsed_time(double *elapsed_times){
    double total_elapsed_time = 0.0;
    for (uint8_t i = 0; i < _GC_THREADS; ++i) { 
    	total_elapsed_time += elapsed_times[i];
    }
    memset(elapsed_times, 0, _GC_THREADS * sizeof(double));
    return total_elapsed_time;
}

size_t _get_total_operations(size_t *operations) {
    size_t total_operations = 0;
    for (uint8_t i = 0; i < _GC_THREADS; ++i) { 
    	total_operations += operations[i];
    }
    memset(operations, 0, _GC_THREADS * sizeof(size_t));
    return total_operations;
}

double total_buffer_insert_elapsed_time() {
    return _get_total_elapsed_time(_buffer_insert_elapsed_times);
}

double total_flush_buffer_elapsed_time() {
    return _get_total_elapsed_time(_flush_buffer_elapsed_times);
}
/*
double total_flush_buffer_fragmentation_elapsed_time() {
    return _get_total_elapsed_time(_flush_buffer_fragmentation_elapsed_times);
}

double total_flush_buffer_nofreespace_elapsed_time() {
    return _get_total_elapsed_time(_flush_buffer_nofreespace_elapsed_times);
}

double total_async_request_elapsed_time() {
    return _get_total_elapsed_time(_async_request_elapsed_times);
}
*/
size_t total_buffer_insert_operations() {
    return _get_total_operations(_buffer_insert_operations);
}

size_t total_flush_buffer_operations() {
    return _get_total_operations(_flush_buffer_operations);
}
/*
size_t total_flush_buffer_fragmentation_operations() {
    return _get_total_operations(_flush_buffer_fragmentation_operations);
}

size_t total_flush_buffer_nofreespace_operations() {
    return _get_total_operations(_flush_buffer_nofreespace_operations);
}

size_t total_async_request_operations() {
    return _get_total_operations(_async_request_operations);
}*/
#endif

static inline void check_allocation_failure(void *ptr, const char *msg) {
    if (!ptr) {
        perror(msg);
        exit(EXIT_FAILURE);
    }
}

/*
 * Function to calculate the pr_buffer index
 */
static inline size_t get_pr_buffer_index(size_t region_index, size_t thread_id, size_t number_of_threads) {
    return (region_index * number_of_threads) + thread_id;
}
/*
static inline void measure_execution_time(void (*func)(void*, void*, size_t), void *obj, void *new_adr, size_t size, double *elapsed_time, uint32_t *total_operations_counter) {
    struct timeval request_start_time, request_end_time;
    gettimeofday(&request_start_time, NULL);
    
    func(obj, new_adr, size);
    
    gettimeofday(&request_end_time, NULL);
    
    *elapsed_time += (request_end_time.tv_sec - request_start_time.tv_sec) * 1000.0;
    *elapsed_time += (request_end_time.tv_usec - request_start_time.tv_usec) / 1000.0;
    
    ++total_operations_counter;    
}*/
/*
 * Initialize region array, tera_group array and their fields
 */
void init_regions(uint32_t gc_threads, const char *h2_write_policy){
  _GC_THREADS = gc_threads;
  #if DEBUG_PRINT 
  fprintf(allocator_log_fp, "[%s|%s|%d]_GC_THREADS:%" PRIu32 "\n", __FILE__, __func__, __LINE__, _GC_THREADS);
  #endif 
  #if defined(H2_COMPACT_STATISTICS)
  _buffer_insert_elapsed_times = calloc(0, _GC_THREADS * sizeof(double));
  check_allocation_failure(_buffer_insert_elapsed_times, "[Error] - Failed to allocate memory for _buffer_insert_elapsed_times\n");
  
  _flush_buffer_elapsed_times = calloc(0, _GC_THREADS * sizeof(double));
  check_allocation_failure(_flush_buffer_elapsed_times, "[Error] - Failed to allocate memory for _flush_buffer_elapsed_times\n");
  /*
  _flush_buffer_fragmentation_elapsed_times = calloc(0, _GC_THREADS * sizeof(double));
  check_allocation_failure(_flush_buffer_fragmentation_elapsed_times, "[Error] - Failed to allocate memory for _flush_buffer_fragmentation_elapsed_times\n");
  
  _flush_buffer_nofreespace_elapsed_times = calloc(0, _GC_THREADS * sizeof(double));
  check_allocation_failure(_flush_buffer_nofreespace_elapsed_times, "[Error] - Failed to allocate memory for _flush_buffer_nofreespace_elapsed_times\n");

  _async_request_elapsed_times = calloc(0, _GC_THREADS * sizeof(double));
  check_allocation_failure(_async_request_elapsed_times, "[Error] - Failed to allocate memory for _async_request_elapsed_times\n");
*/
  _buffer_insert_operations = calloc(0, _GC_THREADS * sizeof(size_t));
  check_allocation_failure(_buffer_insert_operations, "[Error] - Failed to allocate memory for _buffer_insert_operations\n");
/*
  _flush_buffer_fragmentation_operations = calloc(0, _GC_THREADS * sizeof(size_t));
  check_allocation_failure(_flush_buffer_fragmentation_operations, "[Error] - Failed to allocate memory for _flush_buffer_fragmentation_operations\n");

  _flush_buffer_nofreespace_operations = calloc(0, _GC_THREADS * sizeof(size_t));
  check_allocation_failure(_flush_buffer_nofreespace_operations, "[Error] - Failed to allocate memory for _flush_buffer_nofreespace_operations\n");
*/
  _flush_buffer_operations = calloc(0, _GC_THREADS * sizeof(size_t)); 
  check_allocation_failure(_flush_buffer_operations, "[Error] - Failed to allocate memory for _flush_buffer_operations\n");
/*
  _async_request_operations = calloc(0, _GC_THREADS * sizeof(size_t));  
  check_allocation_failure(_async_request_operations, "[Error] - Failed to allocate memory for _async_request_operations\n");
  */
  #endif


  if(strcmp(h2_write_policy, "AsyncWritePolicy") == 0){
      write_to_h2 = r_awrite;
  }else{
      write_to_h2 = r_write;
      _ASYNC_IS_ENABLED = false;
  }

  region_enabled = -1;
  offset_list = NULL;
  int32_t rdd_id_size = MAX_PARTITIONS * max_rdd_id;

  region_array=(struct region*) malloc(region_array_size * sizeof(struct region));
  //region_array=(struct region*) malloc(region_array_size * (sizeof(struct region) + _GC_THREADS * sizeof(struct pr_buffer *)));
  check_allocation_failure(region_array, "[Error] - Failed to allocate memory for region_array\n"); 

  id_array=(struct region**) malloc(( MAX_PARTITIONS * max_rdd_id) * sizeof(struct region*));
  check_allocation_failure(id_array, "[Error] - Failed to allocate memory for id_array\n"); 

#if DEBUG_PRINT 
  fprintf(allocator_log_fp, "[%s|%s|%d]Total num of regions:%" PRIu64 "\nrdd_id_size:%d\n", __FILE__, __func__, __LINE__, region_array_size, rdd_id_size);
#endif
/*
#if PR_BUFFER
  pr_buffers =(struct pr_buffer *) malloc(region_array_size * _GC_THREADS * sizeof(struct pr_buffer));
  check_allocation_failure(pr_buffers, "[Error] - Failed to allocate memory for pr_buffers\n"); 
  //struct pr_buffer *buf = pr_buffers;
#endif 
*/
#if DEBUG_PRINT 
  fprintf(stderr, "Total num of regions:%" PRUu64 "\n", region_array_size);
#endif
  for (size_t i = 0; i < region_array_size ; i++) {
    region_array[i].start_address             = (i == 0) ? start_addr_mem_pool() : (region_array[i - 1].start_address + (uint64_t) REGION_SIZE);
    region_array[i].used                      = 0;
    region_array[i].last_allocated_end        = region_array[i].start_address;
    region_array[i].last_allocated_start      = NULL;
    region_array[i].first_allocated_start     = NULL;
    region_array[i].dependency_list           = NULL;
    region_array[i].rdd_id                    = rdd_id_size;
    region_array[i].part_id                   = rdd_id_size;
    
#if PR_BUFFER
    
    //region_array[i].pr_buffer                 = malloc(sizeof(struct pr_buffer));
    region_array[i].pr_buffers                 = malloc(_GC_THREADS * sizeof(struct pr_buffer *));
    if (!region_array[i].pr_buffers) {
        fprintf(stderr, "[Error] - Failed to allocate contiguous pr_buffer block for region %zd\n", i);
        //free(region_array[i].pr_buffers);
        exit(EXIT_FAILURE);
    }
#if 1//perpap
    for(size_t tid = 0; tid < _GC_THREADS; ++tid){
	    //size_t _index = get_pr_buffer_index(i, tid, _GC_THREADS);
	    
	region_array[i].pr_buffers[tid]        = malloc(sizeof(struct pr_buffer));
	if(!region_array[i].pr_buffers[tid]){
	    fprintf(stderr, "pr_buffer is NULL for the region:%zd handled by gc_thread:%zd!\n", i, tid);
	    exit(EXIT_FAILURE);
	}
	region_array[i].pr_buffers[tid]->buffer         = NULL;
	region_array[i].pr_buffers[tid]->size           = 0;
	region_array[i].pr_buffers[tid]->alloc_ptr      = NULL;
	region_array[i].pr_buffers[tid]->first_obj_addr = NULL;
	/*
	pr_buffers[_index].buffer = NULL;
	pr_buffers[_index].size = 0;
	pr_buffers[_index].alloc_ptr = NULL;
	pr_buffers[_index].first_obj_addr = NULL;
	*/
    }
#endif
#if 0
=======
#if ANONYMOUS
    region_array[i].size_mapped               = 0;
    region_array[i].offset_list               = NULL;
#endif
    region_array[i].rdd_id                    = MAX_PARTITIONS * MAX_RDD_ID;
    region_array[i].part_id                   = MAX_PARTITIONS * MAX_RDD_ID;
#if PR_BUFFER
    region_array[i].pr_buffer                 = malloc(sizeof(struct pr_buffer));
>>>>>>> melidonis/java17_g1_teraheap
    region_array[i].pr_buffer->buffer         = NULL;
    region_array[i].pr_buffer->size           = 0;
    region_array[i].pr_buffer->alloc_ptr      = NULL;
    region_array[i].pr_buffer->first_obj_addr = NULL;
#endif
<<<<<<< HEAD

#endif //#if PR_BUFFER
  }
  for (int32_t i = 0; i < rdd_id_size; i++) {
    id_array[i]                               = NULL;
  }
/*
  for (i = 0; i < MAX_PARTITIONS * MAX_RDD_ID; i++) {
    id_array[i]                               = NULL;
  }
    

#if ANONYMOUS
  struct offset *prev = NULL;
  for (i = 0 ; i < DEV_SIZE / MMAP_SIZE ; i++){
    struct offset *ptr = malloc(sizeof(struct offset));
    ptr->offset = MMAP_SIZE * i;
    ptr->next = NULL;
    if (offset_list == NULL) {
      offset_list = ptr;
    } else {
      prev->next = ptr;
    }
    prev = ptr;
  }
#endif 
*/ 
}

/*
 * Returns the start of cont_regions empty regions
 */
int32_t get_cont_regions(int32_t cont_regions){
  static int32_t i = 0;
  int32_t j, index, end_index = i;

  for(; i < (region_array_size + end_index); i++) {
    for (j = i ; j < (i + cont_regions); j++) {
      if (region_array[j % (int32_t) region_array_size].last_allocated_end == 
          region_array[j % (int32_t) region_array_size].start_address) 
        continue;
      else
        break;
    }

    // Find region
    if (((j - 1) % (int32_t) region_array_size) == ((i % (int32_t) region_array_size) + cont_regions -1 )) {
      index = i;
      i = j % (int32_t) region_array_size;
      return index;
    }
    else
      i = j;
  }

  return -1;
}


/*
 * Finds an empty region and returns its starting address
 * Arguments: size is the size of the object we want to allocate (in
 * Bytes)

 * mark unused
 *  marking phase
        mark_used
   precompaction phase
      new address
 */
char* new_region(size_t size){
  int32_t i;
  int32_t cont_regions = (size % REGION_SIZE != 0) ? (size / REGION_SIZE) + 1 : (size / REGION_SIZE);
  int32_t cur_region = get_cont_regions(cont_regions);

  if (cur_region == -1)
    return NULL;

  for (i = cur_region ; i < cur_region + cont_regions ; i++){
    assertf(region_array[i].used == 0, "Error, write to an already used region");
    mark_used(region_array[i].start_address);
    references(region_array[cur_region].start_address, region_array[i].start_address);
    //references(region_array[i].start_address, region_array[cur_region].start_address);
    region_array[i].last_allocated_start = region_array[cur_region].start_address;
    region_array[i].first_allocated_start = region_array[cur_region].start_address;
    region_array[i].last_allocated_end = region_array[cur_region].start_address + size;
  }

  return region_array[cur_region].start_address;
}

uint64_t get_id(uint64_t rdd_id, uint64_t partition_id){
  assertf(partition_id < MAX_PARTITIONS, "Partition id out of bounds");
    return (rdd_id % max_rdd_id) * MAX_PARTITIONS + partition_id;
}

char* allocate_to_region(size_t size, uint64_t rdd_id, uint64_t partition_id) {
#if STATISTICS
  struct timeval start_time, end_time;
  gettimeofday(&start_time, NULL);
#endif

/*
#if ANONYMOUS
  assert(size <= REGION_SIZE);
#endif
*/
  int32_t id_index = get_id(rdd_id, partition_id);
  if (id_array[id_index] == NULL) {
    char* res = new_region(size);
  
    if (res == NULL) {
      perror("[Error] - H2 Allocator is full");
      exit(EXIT_FAILURE);
    }
    /* If object spans more than 1 region we don't want to allocate more objects with it*/
    if (size < (uint64_t) REGION_SIZE) {
      id_array[id_index] = &region_array[((res+size) - region_array[0].start_address) / ((uint64_t)REGION_SIZE)];
      id_array[id_index]->rdd_id = rdd_id;
      id_array[id_index]->part_id = partition_id;
    }
#if DEBUG_PRINT
    printf("Allocating from region %ld until region %ld\n",(res - region_array[0].start_address) / ((uint64_t)REGION_SIZE),(res+size - region_array[0].start_address) / ((uint64_t)REGION_SIZE));
#endif

    return res;
  }

  if (id_array[id_index]->last_allocated_end + size > id_array[id_index]->start_address + (uint64_t)REGION_SIZE) {
#if DEBUG_PRINT
    printf("Allocating from region %ld until region %ld\n",((res) - region_array[0].start_address) / ((uint64_t)REGION_SIZE),((res+size) - region_array[0].start_address) / ((uint64_t)REGION_SIZE));
#endif

    char* res = new_region(size);
    
    if (res == NULL) {
      perror("[Error] - H2 Allocator is full");
      exit(EXIT_FAILURE);
    }

    /* If object spans more than 1 region we don't want to allocate more objects with it*/
    if (size < (uint64_t) REGION_SIZE){
      id_array[id_index] = &region_array[(res+size - region_array[0].start_address) / (uint64_t)REGION_SIZE];
      id_array[id_index]->rdd_id = rdd_id;
      id_array[id_index]->part_id = partition_id;
    }

    assertf(res != NULL, "No empty region");

#if DEBUG_PRINT
    printf("Allocating from region %ld until region %ld\n",(res - region_array[0].start_address) / (uint64_t)REGION_SIZE,(res+size - region_array[0].start_address) / (uint64_t)REGION_SIZE);
#endif

    return res;
  }

  mark_used(id_array[id_index]->start_address);
  id_array[id_index]->last_allocated_start = id_array[id_index]->last_allocated_end;
  id_array[id_index]->last_allocated_end = id_array[id_index]->last_allocated_start + size;


#if STATISTICS
  gettimeofday(&end_time, NULL);
  alloc_elapsedtime += (end_time.tv_sec - start_time.tv_sec) * 1000.0;
  alloc_elapsedtime += (end_time.tv_usec - start_time.tv_usec) / 1000.0;
#endif

#if DEBUG_PRINT
  printf("Allocating from region %ld until region %ld\n",(id_array[id_index]->last_allocated_start - region_array[0].start_address) / (uint64_t)REGION_SIZE, (id_array[id_index]->last_allocated_start+size - region_array[0].start_address) / (uint64_t)REGION_SIZE);

#endif

  return id_array[id_index]->last_allocated_start;
}


/*
 * function that connects two regions in a tera_group
 * arguments: obj1: the object that references the other
 * obj2 the object that is referenced
 */

void references(char *obj1, char *obj2){
    int32_t seg1 = (obj1 - region_array[0].start_address) / (uint64_t)REGION_SIZE;
    int32_t seg2 = (obj2 - region_array[0].start_address) / (uint64_t)REGION_SIZE;
    if (seg1 >= region_array_size || seg2 >= region_array_size || seg1 < 0 || seg2 < 0)
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
 * function that connects two regions in a tera_group
 * arguments: obj: the object that must be checked to be groupped with the region_enabled
 */
void check_for_group(char *obj){
    int32_t seg1 = region_enabled;
    int32_t seg2 = (obj - region_array[0].start_address) / (uint64_t)REGION_SIZE;
    if (seg1 >= region_array_size || seg2 >= region_array_size || seg1 < 0 || seg2 < 0){ 

        return;
    }
    if (seg1 == seg2)
        return;
    struct tera_group *ptr = region_array[seg1].dependency_list;
    while (ptr != NULL){
        if (ptr->region == &region_array[seg2])
            return;
        ptr = ptr->next;
    }

    struct tera_group *new = malloc(sizeof(struct tera_group));
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
    int32_t i;
    fprintf(stderr, "Groups:\n");
    for (i = 0; i < region_array_size; i++){
        if (region_array[i].dependency_list != NULL){
            struct group *ptr = region_array[i].dependency_list;

            fprintf(stderr, "Region %d depends on regions:\n", i);
            while (ptr != NULL){
                fprintf(stderr, "\tRegion %lu\n", ptr->region-region_array);
                ptr = ptr->next;
            }
        }
    }
}

/*
 * Resets the used field of all regions and groups
 */
void reset_used(){
    int32_t i;
    for (i = 0 ; i < region_array_size ; i++)
        region_array[i].used = 0;
}

/*
 * Marks the region that contains this obj as used and increases tera_group
 * counter (if it belongs to a tera_group)
 * Arguments: obj: the object that is alive
 */
void mark_used(char *obj) {
    struct group *ptr = NULL;
    uint64_t seg = (obj - region_array[0].start_address) / (uint64_t)REGION_SIZE;


	assertf(seg >= 0 && seg < region_array_size,
			"Segment index is out of range %lu", seg); 
    if (region_array[seg].used == 1)
        return;

    region_array[seg].used = 1;
    ptr = region_array[seg].dependency_list;

    while (ptr) {
        mark_used(ptr->region->start_address);
        ptr = ptr->next;
    }
}

#if STATISTICS
void print_statistics(){
    uint64_t wasted_space = 0;
    uint64_t total_regions = 0;
    int32_t i;

    for(i = 0 ; i < region_array_size ; i++ ) {
        if (region_array[i % region_array_size].last_allocated_end != region_array[i % region_array_size].start_address ) {
            total_regions++;
            if (region_array[i].last_allocated_end <= region_array[i].start_address + REGION_SIZE){ 
                wasted_space += (region_array[i].start_address + (uint64_t) REGION_SIZE) - region_array[i].last_allocated_end; 
            }
        }
    }
    fprintf(stderr, "Total Wasted Space: %zu MBytes\n", wasted_space / (1024 * 1024));
    fprintf(stderr, "Total regions: %zu\n", total_regions);
    if (total_regions)
        fprintf(stderr, "Average wasted space: %zu KBytes\n", wasted_space / (1024 * total_regions));
    fprintf(stderr, "Total dependencies:%d\n", total_deps);
    fprintf(stderr, "Total time spent in allocate_to_region:%f ms\n", alloc_elapsedtime);
    fprintf(stderr, "Total time spent in free_regions:%f ms\n", free_elapsedtime);
}
#endif

/*
 * Frees all unused regions
 */
struct region_list* free_regions() {
#if STATISTICS
  struct timeval t1,t2;
  gettimeofday(&t1, NULL);
#endif
  int32_t i;
  struct region_list *head = NULL;
  for (i = 0; i < region_array_size; i++){
    if (region_array[i].used == 0 && region_array[i].last_allocated_end != region_array[i].start_address){
      struct group *ptr = region_array[i].dependency_list;
      struct group *next = NULL;

      while (ptr != NULL) {
        next = ptr->next;
        free(ptr);
#if STATISTICS
        total_deps--;
#endif
        ptr = next;
      }
      region_array[i].dependency_list = NULL;

      if (region_array[i].last_allocated_start >= region_array[i].start_address) {
        struct region_list *new_node = malloc(sizeof(struct region_list));
        new_node->start = region_array[i].start_address;
        new_node->end = region_array[i].last_allocated_start;
        new_node->next = head;
        head = new_node;
      }

      region_array[i].last_allocated_end = region_array[i].start_address;
      region_array[i].last_allocated_start = NULL;
      region_array[i].first_allocated_start = NULL;

      if (id_array[get_id(region_array[i].rdd_id, region_array[i].part_id)] == &region_array[i]) {
        id_array[get_id(region_array[i].rdd_id, region_array[i].part_id)] = NULL;
      }
      region_array[i].rdd_id = MAX_PARTITIONS * max_rdd_id;

#if STATISTICS
      fprintf(stderr, "Freeing region %d \n",i);
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
    int32_t i;
    fprintf(stderr, "Regions:\n");
    for (i = 0; i < region_array_size; i++){
        if (region_array[i].last_allocated_end != region_array[i].start_address)
            fprintf(stderr, "Region %d\n",i);
    }
}

/*
 * Prints all the used regions
 */
void print_used_regions(){
    int32_t i;
    fprintf(stderr, "Used Regions:\n");
    for (i = 0 ; i < region_array_size ; i++){
        if (region_array[i].used == 1)
            fprintf(stderr, "Region %d\n", i);
    }
}

/*
 * Checks if obj is before last object of region
 */
bool is_before_last_object(char *obj){
    uint64_t seg = (obj - region_array[0].start_address) / (uint64_t)REGION_SIZE;
	assertf(seg >= 0 && seg < region_array_size,
			"Segment index is out of range %lu", seg); 
    return (obj >= region_array[seg].last_allocated_end) ? false : true;
}

/*
 * Returns last object of region
 */
char* get_last_object(char *obj){
    uint64_t seg = (obj - region_array[0].start_address) / (uint64_t)REGION_SIZE;
	assertf(seg >= 0 && seg < region_array_size,
			"Segment index is out of range %lu", seg); 
    return region_array[seg].last_allocated_end;
}

// Returns true if object is first of its region false otherwise
bool is_region_start(char *obj){
    uint64_t seg = (obj - region_array[0].start_address) / (uint64_t)REGION_SIZE;
	assertf(seg >= 0 && seg < region_array_size,
			"Segment index is out of range %lu", seg); 

	return (region_array[seg].first_allocated_start == obj) ? true : false;
}

/*
 * Enables groupping with the region in which obj belongs to
 */
void enable_region_groups(char *obj){
  region_enabled = ((uint64_t)(obj - region_array[0].start_address)) / (uint64_t) REGION_SIZE;
  assertf(region_enabled >= 0 && region_enabled < INT32_MAX, "Sanity check for overflow");
}

/*
 * Disables groupping with the region previously enabled
 */
void disable_region_groups(void){
  region_enabled = region_array_size;
  assertf(region_enabled >= 0 && region_enabled < INT32_MAX, "Sanity check for overflow");
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
  while (_next_region < region_array_size &&
    (region_array[_next_region].used == 0 || region_array[_next_region].first_allocated_start != region_array[_next_region].start_address)) {
      _next_region++; 
  }

  if (_next_region >= region_array_size)
    return NULL;

  fprintf(stderr, "[PLACEMENT] Region: %d\n", _next_region);

  region_start_addr = region_array[_next_region].start_address;
  _next_region++;

  return region_start_addr;
}

char *get_first_object(char *addr) {
  uint64_t seg = (addr - region_array[0].start_address) / (uint64_t)REGION_SIZE;
  assertf(seg >= 0 && seg < region_array_size, "Segment index is out of range %lu", seg); 
  return region_array[seg].first_allocated_start;
}

int get_num_of_continuous_regions(char *addr){
  uint64_t seg = (addr - region_array[0].start_address) / (uint64_t)REGION_SIZE;
  #ifdef ASSERT
  static const char *border = "------------------------------------------------------------";
  //fprintf(stderr, "%s\n[%s|%s|%d]\nsegment = %" PRIu64 "\naddress = %p\n region_array[0].start_address = %p\n%s\n", border, __FILE__, __func__, __LINE__, seg, addr, region_array[0].start_address, border);
  assertf(addr >= region_array[0].start_address, "%s\n[%s|%s|%d]\nsegment = %" PRIu64 "\naddress = %p\n region_array[0].start_address = %p\n%s\n", border, __FILE__, __func__, __LINE__, seg, addr, region_array[0].start_address, border);
  #endif


  if (region_array[seg].last_allocated_end == region_array[seg].start_address)
    return 0;

  return (region_array[seg].last_allocated_end - region_array[seg].first_allocated_start) % (uint64_t)REGION_SIZE != 0 ? 
  (region_array[seg].last_allocated_end - region_array[seg].first_allocated_start) / (uint64_t)REGION_SIZE + 1 :
  (region_array[seg].last_allocated_end - region_array[seg].first_allocated_start) / (uint64_t)REGION_SIZE; 

}

/*
 * Get objects 'obj' region start address
 */
char* get_region_start_addr(char *obj, uint64_t rdd_id, uint64_t part_id) {
	uint64_t index = get_id(rdd_id, part_id);

	return id_array[index]->start_address;
}

/*
 * Get object 'groupId' (RDD Id). Each object is allocated based on a tera_group Id
 * and the partition Id that locates in teraflag.
 *
 * @obj: address of the object
 *
 * Return: the object rdd id
 */
uint64_t get_obj_group_id(char *obj) {
  uint64_t seg = (obj - region_array[0].start_address) / (uint64_t)REGION_SIZE;
  assertf(seg >= 0 && seg < region_array_size,

          "Segment index is out of range %lu", seg); 
  return region_array[seg].rdd_id;
}

/*
 * Get object 'groupId'. Each object is allocated based on a tera_group Id
 * and the partition Id that locates in teraflag.
 *
 * obj: address of the object
 *
 * returns: the object partition Id
 */
uint64_t get_obj_part_id(char *obj) {
  uint64_t seg = (obj - region_array[0].start_address) / (uint64_t)REGION_SIZE;
  assertf(seg >= 0 && seg < region_array_size,
          "Segment index %" PRIu64 " is out of range\n%-30s = %p\n%-30s = %p\nregion_array_size = %" PRIu64 "\n", seg, "object address", obj, "region_array[0].start_address", region_array[0].start_address, region_array_size); 

  return region_array[seg].part_id;
}

/*
 * Check if these two objects belong to the same tera_group
 *
 * obj1: address of the object
 * obj2: address of the object
 *
 * returns: 1 if objects are in the same tera_group, 0 otherwise
 */
int is_in_the_same_group(char *obj1, char *obj2) {
	uint64_t seg1 = (obj1 - region_array[0].start_address) / (uint64_t)REGION_SIZE;
	uint64_t seg2 = (obj2 - region_array[0].start_address) / (uint64_t)REGION_SIZE;
	struct group *ptr = NULL;
        
	assertf(seg1 < region_array_size && seg1 >= 0, "region_array_size:%lu\nseg1:%lu\nSegment index is out of range [0-%lu]:%s|%s|%d\n", region_array_size, seg1, region_array_size, __FILE__, __func__, __LINE__);
	assertf(seg2 < region_array_size && seg2 >= 0, "region_array_size:%lu\nseg2:%lu\nSegment index is out of range [0-%lu]:%s|%s|%d\n", region_array_size, seg2, region_array_size, __FILE__, __func__, __LINE__); 

	/* Objects belong to the same tera_group */
	if (seg1 == seg2)
		return 1;

	for (ptr = region_array[seg1].dependency_list; ptr != NULL; ptr = ptr->next) {
		if (ptr->region == &region_array[seg2])
			return 1;
	}

	return 0;
}

/*                                                                              
 * Get the total allocated regions                                              
 * Return the total number of allocated regions or zero, otherwise              
 */                                                                             
long total_allocated_regions() {                                                
	int32_t i;                                                                      
	long counter = 0;                                                           

	for (i = 0; i < region_array_size; i++){                                   
		if (region_array[i].last_allocated_end != region_array[i].start_address)
			counter++;                                                          
	}                                                                           

	return counter;                                                             
}

/*                                                                              
 * Get the total number of used regions                                         
 * Return the total number of used regions or zero, otherwise                   
 */                                                                             
long total_used_regions() {                                                     
	int32_t i;                                                                      
	long counter = 0;                                                           

	for (i = 0 ; i < region_array_size; i++) {                                 
		if (region_array[i].used == 1)                                          
			counter++;                                                          
	}                                                                           
	return counter;                                                             
}

#if PR_BUFFER

/*
 * Flush the promotion buffer of a certain region
 *
 * seg: Index of the region in region array
 *
 */
void flush_buffer(struct pr_buffer *buf, uint32_t worker_id) {
//void flush_buffer(uint64_t seg, uint32_t worker_id) {
    #if 1//perpap
    //struct pr_buffer *buf = region_array[seg].pr_buffers[worker_id];

    if (buf->size == 0){
	return;
    }

    assertf(buf->size <= PR_BUFFER_SIZE, "Sanity check");

    // Write the buffer to TeraCache
    write_to_h2(buf->buffer, buf->first_obj_addr, buf->size / HeapWordSize, worker_id); 
    buf->alloc_ptr = buf->buffer;
    buf->first_obj_addr = NULL;
    buf->size = 0;
    #endif
    #if 0
    struct pr_buffer *buf = region_array[seg].pr_buffer;
    if (buf->size == 0){
	return;
    }
    assertf(buf->size <= PR_BUFFER_SIZE, "Sanity check");
    // Write the buffer to TeraCache
    //r_awrite(buf->buffer, buf->first_obj_addr, buf->size / HeapWordSize);
    write_to_h2(buf->buffer, buf->first_obj_addr, buf->size / HeapWordSize);
    buf->alloc_ptr = buf->buffer;
    buf->first_obj_addr = NULL;
    buf->size = 0;
    #endif
}

/*
 * Add an obect to the promotion buffer. We use promotion buffer to avoid write
 * system calls for small sized objects.
 *
 * obj: Object that will be written in the promotion buffer
 * new_adr: Is used to know where the first object in the promotion buffer will
 *			be move to H2
 * size: Size of the object
 */
void buffer_insert(char* obj, char* new_adr, size_t size, uint32_t worker_id) {
    #if defined(H2_COMPACT_STATISTICS)
    struct timespec buffer_insert_start_time, buffer_insert_end_time;
    clock_gettime(CLOCK_MONOTONIC_RAW, &buffer_insert_start_time);    
    #endif
    uint64_t seg = (new_adr - region_array[0].start_address) / (uint64_t)REGION_SIZE;
    //uint64_t buf_index = (seg * _GC_THREADS + worker_id);
    //struct pr_buffer *buf = pr_buffers + buf_index;
    struct pr_buffer *buf = region_array[seg].pr_buffers[worker_id];
    //struct pr_buffer *buf = region_array[seg].pr_buffer;

    char*  start_adr  = buf->first_obj_addr;
    size_t cur_size   = buf->size;
    size_t free_space = PR_BUFFER_SIZE - cur_size;

    assertf(THRESHOLD < PR_BUFFER_SIZE, "Threshold should be less that promotion buffer size");

    if ((size * HeapWordSize) > THRESHOLD) {
        #if defined(H2_COMPACT_STATISTICS)
	    struct timespec flush_buffer_start_time, flush_buffer_end_time;
	    clock_gettime(CLOCK_MONOTONIC_RAW, &flush_buffer_start_time);     

	    write_to_h2(obj, new_adr, size, worker_id);

	    clock_gettime(CLOCK_MONOTONIC_RAW, &flush_buffer_end_time);     
	    
	    _flush_buffer_elapsed_times[worker_id] += _get_elapsed_time_sec(flush_buffer_start_time, flush_buffer_end_time); 
	    ++_flush_buffer_operations[worker_id];
        #else
            write_to_h2(obj, new_adr, size, worker_id);
        #endif
	
	return;
    }

    /* Allocate a buffer for the region and set buffer allocation ptr */
    if (buf->buffer == NULL) {
	buf->buffer = malloc(PR_BUFFER_SIZE * sizeof(char));
	buf->alloc_ptr = buf->buffer;
    }

    /* Case1: Buffer is empty */
    if (cur_size == 0) { 
        assertf(start_adr == NULL, "Sanity check");
	memcpy(buf->alloc_ptr, obj, size * HeapWordSize);
	buf->first_obj_addr = new_adr;
	buf->alloc_ptr += size * HeapWordSize;
	buf->size = size * HeapWordSize;

	#if defined(H2_COMPACT_STATISTICS)	
	clock_gettime(CLOCK_MONOTONIC_RAW, &buffer_insert_end_time);
        _buffer_insert_elapsed_times[worker_id] += _get_elapsed_time_sec(buffer_insert_start_time, buffer_insert_end_time);
        ++_buffer_insert_operations[worker_id];
        #endif	
	return;
    }
	
    /* 
     * Case2: Object size is grater than the available free space in buffer
     * Case3: Object's new address is not contignious with the other objects - 
     * object belongs to next addresses in the region
     * In both cases we flush the buffer and allocate the object in a new
     * buffer.
     */
    if (((size * HeapWordSize) > free_space) || ((start_adr + cur_size) != new_adr)) {
	#if defined(H2_COMPACT_STATISTICS)
	struct timespec flush_buffer_start_time, flush_buffer_end_time;
        clock_gettime(CLOCK_MONOTONIC_RAW, &flush_buffer_start_time);        
        #endif
        flush_buffer(buf, worker_id);
        #if defined(H2_COMPACT_STATISTICS)
        clock_gettime(CLOCK_MONOTONIC_RAW, &flush_buffer_end_time);        
	_flush_buffer_elapsed_times[worker_id] += _get_elapsed_time_sec(flush_buffer_start_time, flush_buffer_end_time);
	++_flush_buffer_operations[worker_id];
        #endif
	assertf(buf != NULL, "pr buffer is NULL");
	memcpy(buf->alloc_ptr, obj, size * HeapWordSize);
	buf->first_obj_addr = new_adr;
	buf->alloc_ptr += size * HeapWordSize;
	buf->size = size * HeapWordSize;
	return;
    }	
    memcpy(buf->alloc_ptr, obj, size * HeapWordSize);
    buf->alloc_ptr += size * HeapWordSize;
    buf->size += size * HeapWordSize;
    #if defined(H2_COMPACT_STATISTICS) 
    clock_gettime(CLOCK_MONOTONIC_RAW, &buffer_insert_end_time);   
    _buffer_insert_elapsed_times[worker_id] += _get_elapsed_time_sec(buffer_insert_start_time, buffer_insert_end_time);
    ++_buffer_insert_operations[worker_id];
    #endif
}

/*
 * Flush all active buffers and free each buffer memory. We need to free their
 * memory to limit waste space.
 */
void free_all_buffers() {
    struct pr_buffer *buf;
    //struct pr_buffer **buf;
    //struct pr_buffer *buf = pr_buffers;
#if 0
    for (uint64_t i = 0, tid = 0, total_buffers = region_array_size * _GC_THREADS; i < total_buffers; i++, tid++, buf++) {
    #ifdef ASSERT
    assertf(buf != NULL, "pr_buffer is NULL! buffer_index = %" PRIu64 " region_array_size = %" PRIu64 "\n", i, region_array_size);
    #endif
    if(tid == _GC_THREADS){
        tid = 0;
    }
        if(buf != NULL){
	    /* Buffer is not empty, so flush it*/
	    if (buf->size != 0) { 
	       #if defined(H2_COMPACT_STATISTICS)
	       struct timespec flush_buffer_start_time, flush_buffer_end_time;
	       clock_gettime(CLOCK_MONOTONIC_RAW, &flush_buffer_start_time);        
	       #endif
	       flush_buffer(buf, tid);
	       #if defined(H2_COMPACT_STATISTICS)
	       clock_gettime(CLOCK_MONOTONIC_RAW, &flush_buffer_end_time);        
	       _flush_buffer_elapsed_times[tid] += _get_elapsed_time_sec(flush_buffer_start_time, flush_buffer_end_time);
	       ++_flush_buffer_operations[tid];
	       #endif
	    }

	    /* If the buffer is already flushed, just free buffer's memory */
	    if (buf->buffer != NULL) {
	       free(buf->buffer);
	       buf->buffer = NULL;
	       buf->alloc_ptr = NULL;
	       buf->first_obj_addr = NULL;
	       buf->size = 0;
	    }
        }
    }
#endif
#if 1
    for (uint64_t i = 0; i < region_array_size; i++) {
	//buf = region_array[i].pr_buffers;
	for(uint32_t tid = 0; tid < _GC_THREADS; tid++) {
            buf = region_array[i].pr_buffers[tid];
            #ifdef ASSERT
            assertf(buf/*[tid]*/ != NULL, "pr_buffer is NULL! buffer_index = %" PRIu64 " region_array_size = %" PRIu64 "\n", i, region_array_size);
            #endif
	    if(buf/*[tid]*/ != NULL){
		    /* Buffer is not empty, so flush it*/
		    if (/*buf[tid]*/buf->size != 0) { 
                       #if defined(H2_COMPACT_STATISTICS)
                       struct timespec flush_buffer_start_time, flush_buffer_end_time;
                       clock_gettime(CLOCK_MONOTONIC_RAW, &flush_buffer_start_time);        
                       #endif
		       flush_buffer(buf/*[tid]*/, tid);
                       #if defined(H2_COMPACT_STATISTICS)
                       clock_gettime(CLOCK_MONOTONIC_RAW, &flush_buffer_end_time);        
		       _flush_buffer_elapsed_times[tid] += _get_elapsed_time_sec(flush_buffer_start_time, flush_buffer_end_time);
		       ++_flush_buffer_operations[tid];
                       #endif
		    }

		    /* If the buffer is already flushed, just free buffer's memory */
		    if (/*buf[tid]*/buf->buffer != NULL) {
		       free(/*buf[tid]*/buf->buffer);
		       /*buf[tid]*/buf->buffer = NULL;
		       /*buf[tid]*/buf->alloc_ptr = NULL;
		       /*buf[tid]*/buf->first_obj_addr = NULL;
		       /*buf[tid]*/buf->size = 0;
		    }
	    }
	}
    }
#endif
#if 0
    //uint64_t i;
    struct pr_buffer *buf;

    for (uint64_t i = 0; i < region_array_size; i++) {
        buf = region_array[i].pr_buffer;
        #ifdef ASSERT
        assertf(buf != NULL, "pr_buffer is NULL! buffer_index = %" PRIu64 " region_array_size = %" PRIu64 "\n", i, region_array_size);
        #endif
        /* Buffer is not empty, so flush it*/
        if (buf->size != 0)
           flush_buffer(i, 0/*dummy*/);

        /* If the buffer is already flushed, just free bufffer's memory */
        if (buf->buffer != NULL) {
            free(buf->buffer);
            buf->buffer = NULL;
            buf->alloc_ptr = NULL;
            buf->first_obj_addr = NULL;
            buf->size = 0;
        }
    }
#endif
}

void free_all_buffers_parallel(uint32_t worker_id) {
    struct pr_buffer *buf;
    for (uint64_t i = 0; i < region_array_size; i++) {
        buf = region_array[i].pr_buffers[worker_id];
        #ifdef ASSERT
        assertf(buf != NULL, "pr_buffer is NULL! buffer_index = %" PRIu64 " region_array_size = %" PRIu64 "\n", i, region_array_size);
        #endif
        if(buf != NULL){
	    /* Buffer is not empty, so flush it*/
	    if (buf->size != 0) { 
	       #if defined(H2_COMPACT_STATISTICS)
	       struct timespec flush_buffer_start_time, flush_buffer_end_time;
	       clock_gettime(CLOCK_MONOTONIC_RAW, &flush_buffer_start_time);        
	       #endif
	       flush_buffer(buf, worker_id);
	       #if defined(H2_COMPACT_STATISTICS)
	       clock_gettime(CLOCK_MONOTONIC_RAW, &flush_buffer_end_time);        
	       _flush_buffer_elapsed_times[worker_id] += _get_elapsed_time_sec(flush_buffer_start_time, flush_buffer_end_time);
	       ++_flush_buffer_operations[worker_id];
	       #endif
	    }

	    /* If the buffer is already flushed, just free buffer's memory */
	    if (buf->buffer != NULL) {
	       free(buf->buffer);
	       buf->buffer = NULL;
	       buf->alloc_ptr = NULL;
	       buf->first_obj_addr = NULL;
	       buf->size = 0;
	    }
        }
    }
}

bool object_starts_from_region(char *obj) {
  uint64_t seg = (obj - region_array[0].start_address) / (uint64_t)REGION_SIZE;
  assertf(seg >= 0 && seg < region_array_size,
          "Segment index %" PRIu64 " is out of range!\n%-30s = %p\n%-30s = %p\nregion_array_size = %" PRIu64 "\n", seg, "object address", obj, "region_array[0].start_address", region_array[0].start_address, region_array_size); 
  return (region_array[seg].first_allocated_start != region_array[seg].start_address) ? false : true;
}

bool object_starts_from_region(char *obj) {
  uint64_t seg = (obj - region_array[0].start_address) / ((uint64_t)REGION_SIZE);
  assertf(seg >= 0 && seg < REGION_ARRAY_SIZE,
          "Segment index is out of range %lu", seg); 
  return (region_array[seg].first_allocated_start != region_array[seg].start_address) ? false : true;
}
#endif
