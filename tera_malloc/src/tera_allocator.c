#include "../include/tera_allocator.h"

#include <sys/mman.h>
#include <stdio.h>
#include "../include/tera_assert.h"

#define SIZE_PER_ENTRY 24UL

typedef struct arena {
  char *start_addr;         /* < Current allocatio */
  char *top;
  uint64_t remain_size;
  uint64_t total_size;
} arena_t;

arena_t *arena;

static uint64_t nearest_power_of_2(uint64_t entries) {
  uint64_t power_of_2 = entries * SIZE_PER_ENTRY;
  power_of_2--;
  power_of_2 |= power_of_2 >> 1;
  power_of_2 |= power_of_2 >> 2;
  power_of_2 |= power_of_2 >> 4;
  power_of_2 |= power_of_2 >> 8;
  power_of_2 |= power_of_2 >> 16;
  power_of_2++;

  return power_of_2;
}

void init_arena(uint64_t entries) {
  if (entries == 0) {
    return;
  }

  arena = malloc(sizeof(arena_t));

  arena->total_size = nearest_power_of_2(entries);
  arena->remain_size = arena->total_size;

  arena->start_addr = mmap(NULL, arena->total_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON | MAP_NORESERVE, -1, 0);
  if (arena->start_addr == NULL) {
    perror("[TERA_ALLOCATOR] Mmap failed!");
    exit(EXIT_FAILURE);
  }

  arena->top = arena->start_addr;
}

char* tera_malloc(uint64_t size) {
  assert(size > 0 && size == (size_t)SIZE_PER_ENTRY, "Error size %lu", size);
  assert(size < arena->remain_size, "Requested size is grater than remaining size");

  if (size > arena->remain_size) {
    perror("[TERA_ALLOCATOR] Arena run out of memory!");
    exit(EXIT_FAILURE);
  }

  char* addr = arena->top;
  arena->top += size;
  assert(arena->top >= arena->start_addr && arena->top < (arena->start_addr + arena->total_size), "Top pointer out of bound");

  arena->remain_size -= size;

  return addr;
}

void free_arena() {
  int res = munmap(arena->start_addr, arena->total_size);

  if (res == -1) {
    perror("[TERA_ALLOCATOR] Munmap failed!");
    exit(EXIT_FAILURE);
  }
  
  arena->start_addr = NULL;
  arena->top = NULL;
  arena->total_size = 0;
  arena->remain_size = 0;
}
