#include <stdio.h>
#include "../include/tera_allocator.h"

int main (int argc, char *argv[])
{
  uint64_t num_entries = 250000000;

  init_arena(num_entries);
  free_arena();

  return 0;
}
