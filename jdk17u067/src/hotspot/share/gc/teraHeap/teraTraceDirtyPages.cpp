#include "gc/teraHeap/teraTraceDirtyPages.hpp"
#include "runtime/os.hpp"
#include <sys/ioctl.h>
#include <cstdlib>

#define TRACE_DIRTY_PAGES_MAGIC 'T'
#define TRACE_DIRTY_PAGES _IOW(TRACE_DIRTY_PAGES_MAGIC, 1, struct ioctl_data)

#define BUFFER_SIZE 512

TeraTraceDirtyPages::TeraTraceDirtyPages(unsigned long start_address, unsigned long end_address) {
  data.start_address = start_address;
  data.end_address = end_address;
  data.page_array = NULL;
  data.page_array_size = 0;
  data.num_dirty_pages = NEW_C_HEAP_ARRAY(unsigned long, 1, mtInternal);;
  *data.num_dirty_pages = 0;
  trace_fd = open("/dev/trace_dirty_pages", O_RDWR);

  if (trace_fd < 0) {
    perror("Failed to open /dev/trace_dirty_pages");
    exit(EXIT_FAILURE);
  }
}

void TeraTraceDirtyPages::find_dirty_pages() {
  size_t num_pages = get_page_cache_size() / os::vm_page_size();
  data.page_array_size = ((num_pages + 512 - 1) / 512) * 512;
  //data.page_array_size = 512;
  data.page_array = NEW_C_HEAP_ARRAY(unsigned long, data.page_array_size, mtInternal);

  if (data.page_array == NULL) {
    perror("Failed to allocate memory for page array");
    close(trace_fd);
    exit(EXIT_FAILURE);
  }
  
  for (size_t i = 0; i < data.page_array_size; i++) {
    data.page_array[i] = 0;
  }
  
  /* Request virtual addresses from the kernel */
  if (ioctl(trace_fd, TRACE_DIRTY_PAGES, &data) < 0) {
    perror("IOCTL VIRT_ADDR_GET failed");
    close(trace_fd);
    exit(EXIT_FAILURE);
  }

  // print filled arrray
  for (size_t i = 0; i < data.page_array_size && data.page_array[i] != 0; i++)
    thlog_or_tty->print_cr("Dirty page = 0x%lx\n", data.page_array[i]);
  
  thlog_or_tty->print_cr("Dirty pages = %lu | Page Cache = %lu | Ratio = %lf\n",
                         *data.num_dirty_pages, num_pages, ((double) *data.num_dirty_pages) / num_pages * 100.0);

  FREE_C_HEAP_ARRAY(unsigned long, data.page_array);
  data.page_array = NULL;
  data.page_array_size = 0;
  *data.num_dirty_pages = 0;
}

unsigned long TeraTraceDirtyPages::get_page_cache_size(void) {
  unsigned long page_cache_size = 0;
  int memory_fd;

  memory_fd = open("/sys/fs/cgroup/memory/memlim/memory.stat", O_RDONLY);

  if (memory_fd == -1) {
    fprintf(stderr, "Failed to open memory.stat\n");
    return 0;
  }

  // Use lseek to reset the file pointer to the beginning of the file
  if (lseek(memory_fd, 0, SEEK_SET) == -1) {
    fprintf(stderr, "Failed to reset file pointer\n");
    return 0;
  }

  // Allocate buffer with O_DIRECT alignment
  char buffer[BUFFER_SIZE + 512]; // Adding extra space for alignment
  char *aligned_buffer = (char *)(((unsigned long)buffer + 512) & ~511);

  ssize_t n_bytes;
  // the line is in this form "cache 1185214464" and we want to extract the
  // second word, which is the cache size in bytes (as a string)
  n_bytes = read(memory_fd, aligned_buffer, BUFFER_SIZE);

  if (n_bytes > 0) {
    // process the data in the buffer (assuming it contains text)
    char *word = strtok(aligned_buffer, " \t\n");
    int count_words = 1;

    while (word != NULL) {
      // The value of the page cache is the second word in the file
      if (count_words == 2) {
        char *p = NULL;
        page_cache_size = strtoul(word, &p, 10);
        break;
      }
      word = strtok(NULL, " \t\n");
      count_words++;
    }
  }

  close(memory_fd);

  return page_cache_size;
}

TeraTraceDirtyPages::~TeraTraceDirtyPages() {
  close(trace_fd); /* Close file */
  FREE_C_HEAP_ARRAY(unsigned long, data.num_dirty_pages);
}
