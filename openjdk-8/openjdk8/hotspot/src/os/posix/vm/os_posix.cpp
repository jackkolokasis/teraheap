/*
* Copyright (c) 1999, 2013, Oracle and/or its affiliates. All rights reserved.
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* This code is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License version 2 only, as
* published by the Free Software Foundation.
*
* This code is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* version 2 for more details (a copy is included in the LICENSE file that
* accompanied this code).
*
* You should have received a copy of the GNU General Public License version
* 2 along with this work; if not, write to the Free Software Foundation,
* Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
*
* Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
* or visit www.oracle.com if you need additional information or have any
* questions.
*
*/

#include "prims/jvm.h"
#include "runtime/frame.inline.hpp"
#include "runtime/os.hpp"
#include "utilities/vmError.hpp"
#include "utilities/align.hpp"
#include "services/memTracker.hpp"

#include <unistd.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <sys/mman.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>

#define check_with_errno(check_type, cond, msg)                               \
do {                                                                          \
    int err = errno;                                                          \
    check_type(cond, "%s; error='%s' (errno=%s)", msg, os::strerror(err),     \
                os::errno_name(err));                                         \
} while (false)

#define assert_with_errno(cond, msg) check_with_errno(assert, cond, msg)

// Check core dump limit and report possible place where core can be found
void os::check_or_create_dump(void* exceptionRecord, void* contextRecord, char* buffer, size_t bufferSize) {
  int n;
  struct rlimit rlim;
  bool success;

  n = get_core_path(buffer, bufferSize);

  if (getrlimit(RLIMIT_CORE, &rlim) != 0) {
    jio_snprintf(buffer + n, bufferSize - n, "/core or core.%d (may not exist)", current_process_id());
    success = true;
  } else {
    switch(rlim.rlim_cur) {
      case RLIM_INFINITY:
        jio_snprintf(buffer + n, bufferSize - n, "/core or core.%d", current_process_id());
        success = true;
        break;
      case 0:
        jio_snprintf(buffer, bufferSize, "Core dumps have been disabled. To enable core dumping, try \"ulimit -c unlimited\" before starting Java again");
        success = false;
        break;
      default:
        jio_snprintf(buffer + n, bufferSize - n, "/core or core.%d (max size %lu kB). To ensure a full core dump, try \"ulimit -c unlimited\" before starting Java again", current_process_id(), (unsigned long)(rlim.rlim_cur >> 10));
        success = true;
        break;
    }
  }
  VMError::report_coredump_status(buffer, success);
}

address os::get_caller_pc(int n) {
#ifdef _NMT_NOINLINE_
  n ++;
#endif
  frame fr = os::current_frame();
  while (n > 0 && fr.pc() &&
    !os::is_first_C_frame(&fr) && fr.sender_pc()) {
    fr = os::get_sender_for_C_frame(&fr);
    n --;
  }
  if (n == 0) {
    return fr.pc();
  } else {
    return NULL;
  }
}

int os::get_last_error() {
  return errno;
}

bool os::is_debugger_attached() {
  // not implemented
  return false;
}

void os::wait_for_keypress_at_exit(void) {
  // don't do anything on posix platforms
  return;
}

// JK: Create file for heap
int os::create_file_for_heap(const char* dir) {

    const char name_template[] = "/jvmheap.XXXXXX";
    size_t fullname_len;
    char *fullname;
    int n;
    bool is_fmap = false;

    if (strcmp(dir, "/dev/nvme0n1") == 0 || 
        strcmp(dir, "/dev/nvme1n1") == 0 || 
        strcmp(dir, "/dev/dmap/dmap1") == 0 || 
        strcmp(dir, "/dev/sdb") == 0 || 
        strcmp(dir, "/dev/sdd") == 0 || 
        strcmp(dir, "/dev/zero") == 0)
    {
        is_fmap = true;
    }

    if (is_fmap)
    {
        fullname_len = strlen(dir);
        fullname = (char*)os::malloc(fullname_len + 1, mtInternal);
    
        if (fullname == NULL) {
            vm_exit_during_initialization(err_msg("Malloc failed during creation of backing file for heap (%s)", os::strerror(errno)));
            return -1;
        }
        n = snprintf(fullname, fullname_len + 1, "%s", dir);
        assert((size_t)n == fullname_len, "Unexpected number of characters in string");
    }
    else {
        fullname_len = strlen(dir) + strlen(name_template);
        fullname = (char*)os::malloc(fullname_len + 1, mtInternal);

        if (fullname == NULL) {
            vm_exit_during_initialization(err_msg("Malloc failed during creation of backing file for heap (%s)", os::strerror(errno)));
            return -1;
        }
        snprintf(fullname, fullname_len + 1, "%s%s", dir, name_template);
        assert((size_t)n == fullname_len, "Unexpected number of characters in string");
    }

    os::native_path(fullname);

    // set the file creation mask.
    mode_t file_mode = S_IRUSR | S_IWUSR;

    // create new file
    int fd;
    if (is_fmap)
    {
        fd = open(dir, O_RDWR, file_mode);
    } else {
        fd = mkstemp(fullname);
    }

    if (fd < 0) {
        warning("Could not create file for heap with template %s", fullname);
        os::free(fullname);
    }

    // delete the name from the filesystem. When 'fd' is closed, the
    // file (and space) will be deleted.
    int ret = unlink(fullname);
    assert_with_errno(ret == 0, "unlink returned error");

    os::free(fullname);
    return fd;
}

// JK: Reserved mmaped memory
static char* reserve_mmaped_memory(size_t bytes, char* requested_addr) {
    char *addr;

    int flags = MAP_PRIVATE | MAP_NORESERVE | MAP_ANONYMOUS;

    if (requested_addr != NULL) {
        assert((uintptr_i) requested_addr % os::vm_page_size() == 0, "Requested address should be aligned to OS page size");
        flags |= MAP_FIXED;
    }

    // Map reserved/uncomitted pages PROT_NONE so we fail early if we
    // touch an uncommited page. Otherwise, the read/write might
    // succeed if we have enough swap space to bacj the physical page.
    addr = (char*)::mmap(requested_addr, bytes, PROT_NONE, flags, -1, 0);

    if (addr != MAP_FAILED) {
        MemTracker::record_virtual_memory_reserve((address)addr, bytes, mtNone, CALLER_PC);
        return addr;
    }
    return NULL;
}

// JK: Util
// static int util_posix_fallocate(int fd, off_t offset, off_t len) {
// #ifdef __APPLE__
//     fstore_t store = { F_ALLOCATECONTIG, F_PEOFPOSMODE, 0, len };
//     // First we try to get a continuous chunk of disk space
//     int ret = fcntl(fd, F_PREALLOCATE, &store);
//     if (ret == -1) {
//         // Maybe we are too fragmented, try to allocate
//         // non-continuousa range
//         store.fst_flags = F_ALLOCATEALL;
//         ret = fcntl(fd, F_PREALLOCATE, &store);
//     }
//     if (ret != -1) {
//         return ftruncate(fd, len);
//     }
//     return -1;
// #else
//     return posix_fallocate(fd, offset, len);
// #endif
// }


// JK: Map the given address range to the provided file descriptor
char* os::map_memory_to_file(char* base, size_t size, int fd) {
    assert(fd != -1, "File descriptor is not valid");

    // Allocate space for the file
    // JK: Comment only for fmap
    //int ret = util_posix_fallocate(fd, 0, (off_t)size);

    //if (ret != 0) {
    //    vm_exit_during_initialization(err_msg("Error in mapping Java Heap at the given filesystem directory. error(%d)", ret));
    //    return NULL;
    //}

    int prot = PROT_READ | PROT_WRITE;
    int flags = MAP_SHARED;
    if (base != NULL) {
        flags |= MAP_FIXED;
    }
    // JK: Mmap reserve space for the file
    // Papagiannis fmap api must replace this line of code
    char* addr = (char*)mmap(base, size, prot, flags, fd, 0);

    if (addr == MAP_FAILED) {
        warning("Failed mmap to file. (%s)", os::strerror(errno));
        return NULL;
    }
    if (base != NULL && addr != base) {
        if (!os::release_memory(addr, size)) {
            warning("Could not release memory on unsuccessful file mapping");
        }
        return NULL;
    }

    // JK: Comment only for fmap
    // madvise(addr, size, MADV_SEQUENTIAL);

    return addr;
}

// JK: Replace mappings with file mapping
char* os::replace_existing_mapping_with_file_mapping(char* base, size_t size, int fd) {
    assert(fd != -1, "File descriptor is not valid");
    assert(base != NULL, "Base cannot be NULL");

    return map_memory_to_file(base, size, fd);
}

// Multiple threads can race in this code, and can remap over each other with MAP_FIXED,
// so on posix, unmap the section at the start and at the end of the chunk that we mapped
// rather than unmapping and remapping the whole chunk to get requested alignment.
char* os::reserve_memory_aligned(size_t size, size_t alignment, int file_desc) {
  assert((alignment & (os::vm_allocation_granularity() - 1)) == 0,
      "Alignment must be a multiple of allocation granularity (page size)");
  assert((size & (alignment -1)) == 0, "size must be 'alignment' aligned");

  size_t extra_size = size + alignment;
  assert(extra_size >= size, "overflow, size is too large to allow alignment");

  char* extra_base;
  if (file_desc != -1) {
      // For file mapping, we do not call
      // os::reserve_memory(extra_size, NULL, alignment, file_desc)
      // because we need to deal with shrinking of the file space
      // later when we release extra memory after alignment. We also
      // cannot called os::reserve_memory() with file_desc set to -1
      // because on AIX we might get SHM memory. So here to call a
      // helper function while reserve memory for us. After we have an
      // aligned base, we will replace anonymous mapping with file
      // mapping.
      extra_base = reserve_mmaped_memory(extra_size, NULL);
      if (extra_base != NULL) {
          MemTracker::record_virtual_memory_reserve((address)extra_base, extra_size, mtNone, CALLER_PC);
      } 
  } else {
      extra_base = os::reserve_memory(extra_size, NULL, alignment);
  }
      
  if (extra_base == NULL) {
      return NULL;
  }

  // Do manual alignment
  char* aligned_base = align_up(extra_base, alignment);

  // [  |                                       |  ]
  // ^ extra_base
  //    ^ extra_base + begin_offset == aligned_base
  //     extra_base + begin_offset + size       ^
  //                       extra_base + extra_size ^
  // |<>| == begin_offset
  size_t begin_offset = aligned_base - extra_base;
  size_t end_offset = (extra_base + extra_size) - (aligned_base + size);

  if (begin_offset > 0) {
      os::release_memory(extra_base, begin_offset);
  }

  if (end_offset > 0) {
      os::release_memory(extra_base + begin_offset + size, end_offset);
  }

  if (file_desc != -1) {
      // After we have an aligned address, we can replace
      // anonymous mapping with file mapping
      if (replace_existing_mapping_with_file_mapping(aligned_base, size, file_desc) == NULL) {
          vm_exit_during_initialization(err_msg("Error in mapping Java heap at the given filesystem directory"));
      }
      MemTracker::record_virtual_memory_commit((address)aligned_base, size, CALLER_PC);
  }
  return aligned_base;
}
          
//   }
// 
//   char* extra_base = os::reserve_memory(extra_size, NULL, alignment);
// 
//   if (extra_base == NULL) {
//     return NULL;
//   }
// 
//   // Do manual alignment
//   char* aligned_base = (char*) align_size_up((uintptr_t) extra_base, alignment);
// 
//   // [  |                                       |  ]
//   // ^ extra_base
//   //    ^ extra_base + begin_offset == aligned_base
//   //     extra_base + begin_offset + size       ^
//   //                       extra_base + extra_size ^
//   // |<>| == begin_offset
//   //                              end_offset == |<>|
//   size_t begin_offset = aligned_base - extra_base;
//   size_t end_offset = (extra_base + extra_size) - (aligned_base + size);
// 
//   if (begin_offset > 0) {
//       os::release_memory(extra_base, begin_offset);
//   }
// 
//   if (end_offset > 0) {
//       os::release_memory(extra_base + begin_offset + size, end_offset);
//   }
// 
//   return aligned_base;
// }

void os::Posix::print_load_average(outputStream* st) {
  st->print("load average:");
  double loadavg[3];
  os::loadavg(loadavg, 3);
  st->print("%0.02f %0.02f %0.02f", loadavg[0], loadavg[1], loadavg[2]);
  st->cr();
}

void os::Posix::print_rlimit_info(outputStream* st) {
  st->print("rlimit:");
  struct rlimit rlim;

  st->print(" STACK ");
  getrlimit(RLIMIT_STACK, &rlim);
  if (rlim.rlim_cur == RLIM_INFINITY) st->print("infinity");
  else st->print("%uk", rlim.rlim_cur >> 10);

  st->print(", CORE ");
  getrlimit(RLIMIT_CORE, &rlim);
  if (rlim.rlim_cur == RLIM_INFINITY) st->print("infinity");
  else st->print("%uk", rlim.rlim_cur >> 10);

  //Isn't there on solaris
#ifndef TARGET_OS_FAMILY_solaris
  st->print(", NPROC ");
  getrlimit(RLIMIT_NPROC, &rlim);
  if (rlim.rlim_cur == RLIM_INFINITY) st->print("infinity");
  else st->print("%d", rlim.rlim_cur);
#endif

  st->print(", NOFILE ");
  getrlimit(RLIMIT_NOFILE, &rlim);
  if (rlim.rlim_cur == RLIM_INFINITY) st->print("infinity");
  else st->print("%d", rlim.rlim_cur);

  st->print(", AS ");
  getrlimit(RLIMIT_AS, &rlim);
  if (rlim.rlim_cur == RLIM_INFINITY) st->print("infinity");
  else st->print("%uk", rlim.rlim_cur >> 10);
  st->cr();
}

void os::Posix::print_uname_info(outputStream* st) {
  // kernel
  st->print("uname:");
  struct utsname name;
  uname(&name);
  st->print(name.sysname); st->print(" ");
  st->print(name.release); st->print(" ");
  st->print(name.version); st->print(" ");
  st->print(name.machine);
  st->cr();
}

bool os::has_allocatable_memory_limit(julong* limit) {
  struct rlimit rlim;
  int getrlimit_res = getrlimit(RLIMIT_AS, &rlim);
  // if there was an error when calling getrlimit, assume that there is no limitation
  // on virtual memory.
  bool result;
  if ((getrlimit_res != 0) || (rlim.rlim_cur == RLIM_INFINITY)) {
    result = false;
  } else {
    *limit = (julong)rlim.rlim_cur;
    result = true;
  }
#ifdef _LP64
  return result;
#else
  // arbitrary virtual space limit for 32 bit Unices found by testing. If
  // getrlimit above returned a limit, bound it with this limit. Otherwise
  // directly use it.
  const julong max_virtual_limit = (julong)3800*M;
  if (result) {
    *limit = MIN2(*limit, max_virtual_limit);
  } else {
    *limit = max_virtual_limit;
  }

  // bound by actually allocatable memory. The algorithm uses two bounds, an
  // upper and a lower limit. The upper limit is the current highest amount of
  // memory that could not be allocated, the lower limit is the current highest
  // amount of memory that could be allocated.
  // The algorithm iteratively refines the result by halving the difference
  // between these limits, updating either the upper limit (if that value could
  // not be allocated) or the lower limit (if the that value could be allocated)
  // until the difference between these limits is "small".

  // the minimum amount of memory we care about allocating.
  const julong min_allocation_size = M;

  julong upper_limit = *limit;

  // first check a few trivial cases
  if (is_allocatable(upper_limit) || (upper_limit <= min_allocation_size)) {
    *limit = upper_limit;
  } else if (!is_allocatable(min_allocation_size)) {
    // we found that not even min_allocation_size is allocatable. Return it
    // anyway. There is no point to search for a better value any more.
    *limit = min_allocation_size;
  } else {
    // perform the binary search.
    julong lower_limit = min_allocation_size;
    while ((upper_limit - lower_limit) > min_allocation_size) {
      julong temp_limit = ((upper_limit - lower_limit) / 2) + lower_limit;
      temp_limit = align_size_down_(temp_limit, min_allocation_size);
      if (is_allocatable(temp_limit)) {
        lower_limit = temp_limit;
      } else {
        upper_limit = temp_limit;
      }
    }
    *limit = lower_limit;
  }
  return true;
#endif
}

const char* os::get_current_directory(char *buf, size_t buflen) {
  return getcwd(buf, buflen);
}

FILE* os::open(int fd, const char* mode) {
  return ::fdopen(fd, mode);
}

void* os::get_default_process_handle() {
  return (void*)::dlopen(NULL, RTLD_LAZY);
}

// Builds a platform dependent Agent_OnLoad_<lib_name> function name
// which is used to find statically linked in agents.
// Parameters:
//            sym_name: Symbol in library we are looking for
//            lib_name: Name of library to look in, NULL for shared libs.
//            is_absolute_path == true if lib_name is absolute path to agent
//                                     such as "/a/b/libL.so"
//            == false if only the base name of the library is passed in
//               such as "L"
char* os::build_agent_function_name(const char *sym_name, const char *lib_name,
                                    bool is_absolute_path) {
  char *agent_entry_name;
  size_t len;
  size_t name_len;
  size_t prefix_len = strlen(JNI_LIB_PREFIX);
  size_t suffix_len = strlen(JNI_LIB_SUFFIX);
  const char *start;

  if (lib_name != NULL) {
    len = name_len = strlen(lib_name);
    if (is_absolute_path) {
      // Need to strip path, prefix and suffix
      if ((start = strrchr(lib_name, *os::file_separator())) != NULL) {
        lib_name = ++start;
      }
      if (len <= (prefix_len + suffix_len)) {
        return NULL;
      }
      lib_name += prefix_len;
      name_len = strlen(lib_name) - suffix_len;
    }
  }
  len = (lib_name != NULL ? name_len : 0) + strlen(sym_name) + 2;
  agent_entry_name = NEW_C_HEAP_ARRAY_RETURN_NULL(char, len, mtThread);
  if (agent_entry_name == NULL) {
    return NULL;
  }
  strcpy(agent_entry_name, sym_name);
  if (lib_name != NULL) {
    strcat(agent_entry_name, "_");
    strncat(agent_entry_name, lib_name, name_len);
  }
  return agent_entry_name;
}

os::WatcherThreadCrashProtection::WatcherThreadCrashProtection() {
  assert(Thread::current()->is_Watcher_thread(), "Must be WatcherThread");
}

/*
 * See the caveats for this class in os_posix.hpp
 * Protects the callback call so that SIGSEGV / SIGBUS jumps back into this
 * method and returns false. If none of the signals are raised, returns true.
 * The callback is supposed to provide the method that should be protected.
 */
bool os::WatcherThreadCrashProtection::call(os::CrashProtectionCallback& cb) {
  sigset_t saved_sig_mask;

  assert(Thread::current()->is_Watcher_thread(), "Only for WatcherThread");
  assert(!WatcherThread::watcher_thread()->has_crash_protection(),
      "crash_protection already set?");

  // we cannot rely on sigsetjmp/siglongjmp to save/restore the signal mask
  // since on at least some systems (OS X) siglongjmp will restore the mask
  // for the process, not the thread
  pthread_sigmask(0, NULL, &saved_sig_mask);
  if (sigsetjmp(_jmpbuf, 0) == 0) {
    // make sure we can see in the signal handler that we have crash protection
    // installed
    WatcherThread::watcher_thread()->set_crash_protection(this);
    cb.call();
    // and clear the crash protection
    WatcherThread::watcher_thread()->set_crash_protection(NULL);
    return true;
  }
  // this happens when we siglongjmp() back
  pthread_sigmask(SIG_SETMASK, &saved_sig_mask, NULL);
  WatcherThread::watcher_thread()->set_crash_protection(NULL);
  return false;
}

void os::WatcherThreadCrashProtection::restore() {
  assert(WatcherThread::watcher_thread()->has_crash_protection(),
      "must have crash protection");

  siglongjmp(_jmpbuf, 1);
}

void os::WatcherThreadCrashProtection::check_crash_protection(int sig,
    Thread* thread) {

  if (thread != NULL &&
      thread->is_Watcher_thread() &&
      WatcherThread::watcher_thread()->has_crash_protection()) {

    if (sig == SIGSEGV || sig == SIGBUS) {
      WatcherThread::watcher_thread()->crash_protection()->restore();
    }
  }
}
