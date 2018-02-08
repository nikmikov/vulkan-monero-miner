#pragma once

#include <stddef.h>
#include <stdlib.h>

#include "logging.h"

#if defined(__APPLE__)
#include <mach/vm_statistics.h>
#endif

#ifdef _WIN32
#include <ntsecapi.h>
#include <windows.h>
#else
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#endif // _WIN32

static inline void *hugepages_alloc(size_t memsize)
{
  void *mem;
#if defined(__APPLE__)
  mem = mmap(0, memsize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON,
             VM_FLAGS_SUPERPAGE_SIZE_2MB, 0);
#elif defined(__FreeBSD__)
  mem =
      mmap(0, memsize, PROT_READ | PROT_WRITE,
           MAP_PRIVATE | MAP_ANONYMOUS | MAP_ALIGNED_SUPER | MAP_PREFAULT_READ,
           -1, 0);
#else
  mem = mmap(0, memsize, PROT_READ | PROT_WRITE,
             MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB | MAP_POPULATE, 0, 0);
#endif
  if (mem == MAP_FAILED) {
    return NULL;
  }
  return mem;
}

static inline void hugepages_free(void *mem, size_t memsize)
{
  int res = munmap(mem, memsize);
  if (res != 0) {
    log_error("Hugepages memory unmapping failed");
  }
}
