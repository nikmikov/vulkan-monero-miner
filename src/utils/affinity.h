#pragma once

#include <stdbool.h>
#include <uv.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>

#if defined(__APPLE__)
#include <mach/thread_act.h>
#include <mach/thread_policy.h>
#define SYSCTL_CORE_COUNT "machdep.cpu.core_count"
#elif defined(__FreeBSD__)
#include <pthread_np.h>
#endif //__APPLE__

#endif //_WIN32

static inline bool uv_thread_set_affinity(uv_thread_t h, uint64_t cpu_id)
{
#if defined(_WIN32)
  return SetThreadAffinityMask(h, 1ULL << cpu_id) != 0;
#elif defined(__APPLE__)
  thread_port_t mach_thread;
  thread_affinity_policy_data_t policy = {(integer_t)cpu_id};
  mach_thread = pthread_mach_thread_np(h);
  return thread_policy_set(mach_thread, THREAD_AFFINITY_POLICY,
                           (thread_policy_t)&policy, 1) == KERN_SUCCESS;
#elif defined(__FreeBSD__)
  cpuset_t mn;
  CPU_ZERO(&mn);
  CPU_SET(cpu_id, &mn);
  return pthread_setaffinity_np(h, sizeof(cpuset_t), &mn) == 0;
#else
  cpu_set_t mn;
  CPU_ZERO(&mn);
  CPU_SET(cpu_id, &mn);
  return pthread_setaffinity_np(h, sizeof(cpu_set_t), &mn) == 0;
#endif
}
