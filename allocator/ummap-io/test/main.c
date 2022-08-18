#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include "ummap.h"

int main(void) {
  mode_t mode     = (S_IRUSR   | S_IWUSR);
  int fd = open("./example.data", O_CREAT | O_RDWR, mode);
  int8_t *baseptr = NULL;       // Base pointer
  size_t size     = 1073741824; // 1GB allocation

  if (fd == -1) {
    perror("File open");
    exit(EXIT_FAILURE);
  }
    
  ummap(size, PROT_READ | PROT_WRITE , fd, 0, (void **)&baseptr);
    
  close(fd);
   
  // Set some random value on the allocation
  for (off_t i = 0; i < 20; i++) {
        baseptr[i] = 21;
        printf("Num = %d\n", baseptr[i]);
  }
}
