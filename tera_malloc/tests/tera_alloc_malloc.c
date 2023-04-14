#include <stdio.h>
#include <string.h>
#include "../include/tera_allocator.h"

int main (int argc, char *argv[])
{
  uint64_t num_entries = 250000000;
  char *addr = NULL;
  int i;

  init_arena(num_entries);

  for (i = 0; i < 200000000; i++) {
    addr = tera_malloc(24);
    strcpy(addr, "Iacovos Kolokasis");
  }

  free_arena();

  return 0;
}
