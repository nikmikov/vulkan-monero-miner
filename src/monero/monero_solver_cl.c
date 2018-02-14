#include "monero/monero_solver.h"

#include "logging.h"
#include <assert.h>
#include <stdatomic.h>
#include <uv.h>

#if defined(__APPLE__)
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#include "resources.h"

struct monero_solver_cl_context {
  cl_context cl_ctx;
  cl_device_id device_id;

  /** Device info and capabilities */
  char device_name[256];
  size_t device_work_group_size;
  cl_uint device_compute_units;
  cl_ulong device_total_memsize;
};

struct monero_solver_cl_context *
monero_solver_cl_context_init(cl_uint platform_id, cl_uint device_id);
bool monero_solver_cl_context_prepare_kernel(
    struct monero_solver_cl_context *cl);
bool monero_solver_cl_context_query_device(struct monero_solver_cl_context *cl);
void monero_solver_cl_context_release(struct monero_solver_cl_context *cl);

struct monero_solver_cl {
  struct monero_solver solver;

  /** set to false to terminate worker thread */
  atomic_bool is_alive;

  /** worker thread */
  uv_thread_t worker;

  /** current job */
  atomic_int job_id;

  /** GPU context */
  struct monero_solver_cl_context *cl;
};

void monero_solver_cl_get_metrics(struct monero_solver *solver,
                                  struct monero_solver_metrics *metrics)
{
  assert(solver != NULL);
  assert(metrics != NULL);
  struct monero_solver_cl *s = (struct monero_solver_cl *)solver;
}

void monero_solver_cl_work(struct monero_solver *ptr,
                           monero_solver_submit submit, void *submit_data,
                           int job_id, const uint8_t *input_hash,
                           size_t input_hash_len, uint64_t target,
                           uint32_t nonce_from, uint32_t nonce_to)
{
  assert(ptr != NULL);
  assert(submit != NULL);
  assert(submit_data != NULL);
  assert(input_hash != NULL);
}

void monero_solver_cl_free(struct monero_solver *ptr)
{
  struct monero_solver_cl *solver = (struct monero_solver_cl *)ptr;
  atomic_store(&solver->is_alive, false);
  uv_thread_join(&solver->worker);
  // uv_close((uv_handle_t *)&solver->solution_found_async, NULL);
  //  uv_mutex_destroy(&solver->solution_lock);
  // cryptonight_ctx_free(&solver->cryptonight_ctx);
  if (solver->cl != NULL) {
    monero_solver_cl_context_release(solver->cl);
  }
  free(solver);
}

struct monero_solver *
monero_solver_new_cl(const struct monero_config_solver_cl *cfg)
{
  assert(cfg != NULL);
  // init gpu
  assert(cfg->platform_id >= 0);
  assert(cfg->device_id >= 0);
  struct monero_solver_cl_context *cl = monero_solver_cl_context_init(
      (cl_uint)cfg->platform_id, (cl_uint)cfg->device_id);
  if (cl == NULL) {
    log_error("Error when initializing opencl device");
    return NULL;
  }

  struct monero_solver_cl *solver_cl =
      calloc(1, sizeof(struct monero_solver_cl));
  atomic_store(&solver_cl->job_id, 0);
  atomic_store(&solver_cl->is_alive, true);
  solver_cl->cl = cl;
  solver_cl->solver.work = monero_solver_cl_work;
  solver_cl->solver.free = monero_solver_cl_free;
  solver_cl->solver.get_metrics = monero_solver_cl_get_metrics;

  return &solver_cl->solver;
}

struct monero_solver_cl_context *
monero_solver_cl_context_init(cl_uint platform_id, cl_uint device_id)
{
  // init cl context
  cl_int ret;
  cl_uint num_platforms = 0;
  ret = clGetPlatformIDs(0, NULL, &num_platforms);
  if (ret != CL_SUCCESS) {
    log_error("Error when querying number of OpenCL platforms");
    return NULL;
  }

  if (num_platforms == 0) {
    log_error("No OpenCL platform found");
    return NULL;
  }

  if (platform_id >= num_platforms) {
    log_error(
        "Platform index(%d) is greater than number available platforms(%u)",
        platform_id, num_platforms);
    return NULL;
  }

  cl_platform_id platforms[num_platforms];
  ret = clGetPlatformIDs(num_platforms, platforms, NULL);
  if (ret != CL_SUCCESS) {
    log_error("Error when querying available OpenCL platforms");
    return NULL;
  }

  cl_uint num_devices = 0;
  ret = clGetDeviceIDs(platforms[platform_id], CL_DEVICE_TYPE_ALL, 0, NULL,
                       &num_devices);
  if (ret != CL_SUCCESS) {
    log_error("Error when querying available OpenCL devices for platform #%d",
              platform_id);
    return NULL;
  }

  if (num_devices == 0) {
    log_error("No devices found for platform #%d", platform_id);
    return NULL;
  }

  if (device_id >= num_devices) {
    log_error(
        "Device index(%d) is greater than number available OpenCL devices(%u)",
        platform_id, num_platforms);
    return NULL;
  }

  cl_device_id devices[num_devices];
  ret = clGetDeviceIDs(platforms[platform_id], CL_DEVICE_TYPE_ALL, num_devices,
                       devices, NULL);
  if (ret != CL_SUCCESS) {
    log_error("Error when querying available OpenCL devices for platform #%d",
              platform_id);
    return NULL;
  }

  cl_context cl_ctx =
      clCreateContext(NULL, 1, &devices[device_id], NULL, NULL, &ret);
  if (!(ret == CL_SUCCESS && cl_ctx != NULL)) {
    log_error("Error when calling clCreateContext: Platform #%d, device: %d",
              platform_id, device_id);
    return NULL;
  }

  struct monero_solver_cl_context *gpu =
      calloc(1, sizeof(struct monero_solver_cl_context));
  gpu->cl_ctx = cl_ctx;
  gpu->device_id = devices[device_id];

  if (!monero_solver_cl_context_query_device(gpu)) {
    log_error("Failed to query OpenCL device capabilities");
    monero_solver_cl_context_release(gpu);
    return NULL;
  }

  if (!monero_solver_cl_context_prepare_kernel(gpu)) {
    log_error("Failed initialize OpenCL solver kernel");
    monero_solver_cl_context_release(gpu);
    return NULL;
  }

  return gpu;
}

void monero_solver_cl_context_release(struct monero_solver_cl_context *gpu)
{
  if (gpu->cl_ctx != NULL) {
    clReleaseContext(gpu->cl_ctx);
  }
  free(gpu);
}

bool monero_solver_cl_context_query_device(struct monero_solver_cl_context *gpu)
{
  cl_int ret;
  ret = clGetDeviceInfo(gpu->device_id, CL_DEVICE_NAME, 255, gpu->device_name,
                        NULL);
  if (ret != CL_SUCCESS) {
    log_error("Error when querying CL_DEVICE_NAME");
    return false;
  }
  log_debug("Device query success: CL_DEVICE_NAME: %s", gpu->device_name);

  ret = clGetDeviceInfo(gpu->device_id, CL_DEVICE_MAX_WORK_GROUP_SIZE,
                        sizeof(size_t), &gpu->device_work_group_size, NULL);
  if (ret != CL_SUCCESS) {
    log_error("Error when querying CL_DEVICE_MAX_WORK_GROUP_SIZE");
    return false;
  }
  log_debug("Device query success: CL_DEVICE_MAX_WORK_GROUP_SIZE: %d",
            (int)gpu->device_work_group_size);

  ret = clGetDeviceInfo(gpu->device_id, CL_DEVICE_MAX_COMPUTE_UNITS,
                        sizeof(cl_uint), &gpu->device_compute_units, NULL);
  if (ret != CL_SUCCESS) {
    log_error("Error when querying CL_DEVICE_MAX_COMPUTE_UNITS");
    return false;
  }
  log_debug("Device query success: CL_DEVICE_MAX_COMPUTE_UNITS: %u",
            gpu->device_compute_units);

  ret = clGetDeviceInfo(gpu->device_id, CL_DEVICE_GLOBAL_MEM_SIZE,
                        sizeof(cl_ulong), &gpu->device_total_memsize, NULL);
  if (ret != CL_SUCCESS) {
    log_error("Error when querying CL_DEVICE_GLOBAL_MEM_SIZE");
    return false;
  }
  log_debug("Device query success: CL_DEVICE_GLOBAL_MEM_SIZE: %lu",
            gpu->device_total_memsize);

  return true;
}

bool monero_solver_cl_context_prepare_kernel(
    struct monero_solver_cl_context *gpu)
{
  printf("%s\n", RC_CL_SOURCE_CRYPTONIGHT);
  return false;
}
