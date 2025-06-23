#include <stdio.h>
#include <bpf/libbpf.h>
#include <stdlib.h>
#include <string.h>

int main() {
  struct bpf_object* obj = nullptr;
  struct bpf_program* prog = nullptr;
  struct bpf_link* link = nullptr;

  const char* bpf_obj_path = "../build/sched_switch.bpf.o";

  // Open the BPF object file
  obj = bpf_object__open_file(bpf_obj_path, nullptr);
  if (!obj) {
    fprintf(stderr, "Failed to open BPF object file: %s\n", bpf_obj_path);
    return 1;
  }

  // Load the BPF program into the kernel
  if (bpf_object__load(obj)) {
    fprintf(stderr, "Failed to load BPF object\n");
    bpf_object__close(obj);
    return 1;
  }

  // Find the tracepoint program by name
  prog = bpf_object__find_program_by_name(obj, "handle_sched_switch");
  if (!prog) {
    fprintf(stderr, "Could not find program 'handle_sched_switch'\n");
    bpf_object__close(obj);
    return 1;
  }

  // Attach the program to the sched:sched_switch tracepoint
  link = bpf_program__attach_tracepoint(prog, "sched", "sched_switch");
  if (!link) {
    fprintf(stderr, "Failed to attach tracepoint sched:sched_switch\n");
    bpf_object__close(obj);
    return 1;
  }

  // Pin the link
  if (bpf_link__pin(link, "/sys/fs/bpf/sched_switch_link") != 0) {
    fprintf(stderr, "Failed to pin BPF link\n");
  }

  // Pin relevant maps
  struct bpf_map* map;
  bpf_object__for_each_map(map, obj) {
    const char* name = bpf_map__name(map);
    if (strcmp(name, "mutators") == 0) {
      bpf_map__pin(map, "/sys/fs/bpf/mutators");
    } else if (strcmp(name, "enabled") == 0) {
      bpf_map__pin(map, "/sys/fs/bpf/enabled");
    } else if (strcmp(name, "iowait") == 0) {
      bpf_map__pin(map, "/sys/fs/bpf/iowait");
    }
  }

  printf("BPF program loaded and pinned successfully.\n");
  bpf_object__close(obj);
  return 0;
}
