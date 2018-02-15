#include "monero/monero_solver.h"

#include "logging.h"
#include <assert.h>
#include <stdatomic.h>
#include <uv.h>

#include "monero.h"

#include "utils/affinity.h"
#include "utils/opencl_inc.h"
#include "utils/port_sleep.h"

#include "resources.h"
#include "utils/opencl_err.h"

#define SOLUTIONS_BUFFER_SIZE 32
#define CRYPTONIGHT_MEMORY 2097152

struct monero_solver_cl_context {
  cl_context cl_ctx;
  cl_device_id device_id;
  cl_command_queue command_queue;
  cl_program cryptonight_program;
  cl_kernel cryptonight_kernel;
  cl_mem input_buffer;
  cl_mem scratchpad_buffer;
  cl_mem output_buffer;

  /** Device info and capabilities */
  char device_name[256];
  size_t device_work_group_size;
  cl_uint device_compute_units;
  cl_ulong device_total_memsize;
  cl_ulong device_local_memsize;
};

struct monero_solver_cl_context *
monero_solver_cl_context_init(cl_uint platform_id, cl_uint device_id);

bool monero_solver_cl_context_prepare_kernel(
    struct monero_solver_cl_context *cl);

bool monero_solver_cl_context_query_device(struct monero_solver_cl_context *cl);

void monero_solver_cl_context_release(struct monero_solver_cl_context *cl);

struct monero_solver_cl {
  struct monero_solver solver;

  /** GPU context */
  struct monero_solver_cl_context *cl;

  /** set to false to terminate worker thread */
  atomic_bool is_alive;

  /** worker thread */
  uv_thread_t worker;

  /** current job */
  atomic_int job_id;
  uint8_t input_hash[MONERO_INPUT_HASH_MAX_LEN];
  size_t input_hash_len;
  uint64_t target;
  uint32_t nonce_from;
  uint32_t nonce_to;

  // submit callback
  monero_solver_submit submit;
  void *submit_data;

  /** solution data */
  uv_async_t solution_found_async; // async handle on solution found
  uv_mutex_t solution_lock;
  struct monero_solution solutions[SOLUTIONS_BUFFER_SIZE];
  size_t num_solutions;

  // quick metrics
  struct monero_solver_metrics metrics;
  atomic_int hashes_counter;
};

const size_t NTHREADS = 32;
const size_t INPUT_BUFFER_SIZE = 88;
const size_t OUTPUT_BUFFER_SIZE = NTHREADS * 1600;
const size_t SCRATCHPAD_BUFFER_SIZE = NTHREADS * CRYPTONIGHT_MEMORY;

void monero_solver_cl_get_metrics(struct monero_solver *solver,
                                  struct monero_solver_metrics *metrics)
{
  assert(solver != NULL);
  assert(metrics != NULL);
  struct monero_solver_cl *s = (struct monero_solver_cl *)solver;

  s->metrics.hashes_processed_total += atomic_exchange(&s->hashes_counter, 0);
  *metrics = s->metrics;
}

static inline void metrics_add_solution(struct monero_solver_metrics *m,
                                        uint64_t sol)
{
  ++m->solutions_found;
  for (size_t i = 0; i < m->solutions_found && i < 10; ++i) {
    if (m->top_10_solutions[i] > sol) {
      uint64_t tmp = m->top_10_solutions[i];
      m->top_10_solutions[i] = sol;
      sol = tmp;
    }
  }
}

/** Called from worker thread on main loop when solution found */
void monero_solver_cl_solution_found(uv_async_t *handle)
{
  assert(handle->data);
  struct monero_solver_cl *solver = (struct monero_solver_cl *)handle->data;
  struct monero_solution solutions[solver->num_solutions];
  size_t num_solutions = 0;
  uv_mutex_lock(&solver->solution_lock);
  assert(solver->num_solutions > 0);
  // copy solutions buffer
  num_solutions = solver->num_solutions;
  memcpy(solutions, solver->solutions,
         sizeof(struct monero_solution) * num_solutions);
  solver->num_solutions = 0;
  uv_mutex_unlock(&solver->solution_lock);

  // submit solution
  assert(solver->submit != NULL);
  for (size_t i = 0; i < num_solutions; ++i) {
    solver->submit(solver->solver.solver_id, &solutions[i],
                   solver->submit_data);
    metrics_add_solution(&solver->metrics,
                         monero_solution_hash_val(solutions[i].hash));
  }
}

void monero_solver_cl_work_thread(void *arg)
{
  log_debug("CL Worker thread started");
  struct monero_solver_cl *solver = arg;
  struct monero_solver_cl_context *ctx = solver->cl;

  int current_job_id = 0;

  // bytes 24..31 is what we are looking for
  uint64_t target = 0;
  uint8_t input_hash[MONERO_INPUT_HASH_MAX_LEN];
  uint32_t nonce = 0, nonce_to = 0;
  size_t input_hash_len = 0;

  bool opencl_new_data = false;
  bool opencl_kernel_ready = false;

  char *hash_output = calloc(1, OUTPUT_BUFFER_SIZE);

  cl_int ret;
  while (atomic_load(&solver->is_alive)) {
    int j = atomic_load(&solver->job_id);
    if (j != current_job_id) {
      log_warn("Worker thread received job");
      // load new job into local buffer
      input_hash_len = solver->input_hash_len;
      assert(input_hash_len <= MONERO_INPUT_HASH_MAX_LEN);
      if (input_hash_len < MONERO_NONCE_POSITION + 4 ||
          input_hash_len > INPUT_BUFFER_SIZE) {
        log_error("Work #%d: Invalid input hash len: %lu", input_hash_len);
        continue;
      }
      // copy hash
      memcpy(input_hash, solver->input_hash, input_hash_len);
      // nonces
      nonce = solver->nonce_from;
      nonce_to = solver->nonce_to;
      // target
      target = solver->target;
      current_job_id = j;
      opencl_new_data = true;
      opencl_kernel_ready = false;
    } else if (opencl_new_data) { // send new input to GPU
      opencl_new_data = false;
      assert(ctx != NULL);
      // INPUT BUFFER DATA
      ret =
          clEnqueueWriteBuffer(ctx->command_queue, ctx->input_buffer, CL_TRUE,
                               0, INPUT_BUFFER_SIZE, input_hash, 0, NULL, NULL);
      if (ret != CL_SUCCESS) {
        log_error("Error when calling clEnqueueWriteBuffer with input data: %s",
                  cl_err_str(ret));
        continue;
      }

      // INPUT
      ret = clSetKernelArg(ctx->cryptonight_kernel, 0, sizeof(cl_mem),
                           &ctx->input_buffer);
      if (ret != CL_SUCCESS) {
        log_error(
            "Error when calling clSetKernelArg for arg #0[input_buffer]: %s",
            cl_err_str(ret));
        continue;
        ;
      }

      // SCRATCHPAD
      ret = clSetKernelArg(ctx->cryptonight_kernel, 1, sizeof(cl_mem),
                           &ctx->scratchpad_buffer);
      if (ret != CL_SUCCESS) {
        log_error("Error when calling clSetKernelArg for arg "
                  "#1[scratchpad_buffer]: %s",
                  cl_err_str(ret));
        continue;
        ;
      }

      // OUTPUT BUFFER
      ret = clSetKernelArg(ctx->cryptonight_kernel, 2, sizeof(cl_mem),
                           &ctx->output_buffer);
      if (ret != CL_SUCCESS) {
        log_error(
            "Error when calling clSetKernelArg for arg #2[output_buffer]: %s",
            cl_err_str(ret));
        continue;
        ;
      }
      opencl_kernel_ready = true;
    } else if (opencl_kernel_ready && nonce < nonce_to) { // work job
      // run cryptonight

      // EXEC KERNEL
      size_t global_offset = nonce;
      size_t global_threads = NTHREADS;
      size_t local_threads = 1;

      ret = clEnqueueNDRangeKernel(ctx->command_queue, ctx->cryptonight_kernel,
                                   1, &global_offset, &global_threads,
                                   &local_threads, 0, NULL, NULL);
      if (ret != CL_SUCCESS) {
        log_error("Error when calling clEnqueueNDRangeKernel: %s",
                  cl_err_str(ret));
        opencl_kernel_ready = false;
        continue;
      }

      // READ RESULTS
      ret = clEnqueueReadBuffer(ctx->command_queue, ctx->output_buffer, CL_TRUE,
                                0, OUTPUT_BUFFER_SIZE, hash_output, 0, NULL,
                                NULL);
      if (ret != CL_SUCCESS) {
        log_error("Error when calling clEnqueueReadBuffer to fetch results: %s",
                  cl_err_str(ret));
        continue;
      }
      atomic_fetch_add(&solver->hashes_counter, NTHREADS);

      nonce += NTHREADS;
      // break;
    } else {
      log_debug("No work available. Z-z-z-z...");
      port_sleep(1);
    }
  }
  free(hash_output);
  log_debug("CL Worker thread quit");
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

  struct monero_solver_cl *solver = (struct monero_solver_cl *)ptr;
  assert(input_hash_len <= MONERO_INPUT_HASH_MAX_LEN);
  log_debug("New work: %lu, target: %lx, nonce: %x - %x, %u hashes to go",
            job_id, target, nonce_from, nonce_to, (nonce_to - nonce_from));
  solver->submit = submit;
  solver->submit_data = submit_data;
  memcpy(solver->input_hash, input_hash, input_hash_len);
  solver->input_hash_len = input_hash_len;
  solver->target = target;
  solver->nonce_from = nonce_from;
  solver->nonce_to = nonce_to;

  atomic_store(&solver->job_id, job_id); // signal worker of job change
}

void monero_solver_cl_free(struct monero_solver *ptr)
{
  struct monero_solver_cl *solver = (struct monero_solver_cl *)ptr;
  atomic_store(&solver->is_alive, false);
  uv_thread_join(&solver->worker);

  uv_close((uv_handle_t *)&solver->solution_found_async, NULL);
  uv_mutex_destroy(&solver->solution_lock);

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

  solver_cl->cl = cl;
  atomic_store(&solver_cl->job_id, 0);
  atomic_store(&solver_cl->is_alive, true);
  solver_cl->solver.work = monero_solver_cl_work;
  solver_cl->solver.free = monero_solver_cl_free;
  solver_cl->solver.get_metrics = monero_solver_cl_get_metrics;

  uv_mutex_init(&solver_cl->solution_lock);
  uv_async_init(uv_default_loop(), &solver_cl->solution_found_async,
                monero_solver_cl_solution_found);
  solver_cl->solution_found_async.data = solver_cl;
  uv_thread_create(&solver_cl->worker, monero_solver_cl_work_thread, solver_cl);

  int affinity = cfg->solver.affine_to_cpu;
  if (affinity >= 0) {
    bool r = uv_thread_set_affinity(solver_cl->worker, (uint64_t)affinity);
    if (!r) {
      log_warn("CPU affinity not set");
    } else {
      log_info("Set CPU affinity: %d", affinity);
    }
  }

  atomic_store(&solver_cl->hashes_counter, 0);

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

void monero_solver_cl_context_release(struct monero_solver_cl_context *ctx)
{
  if (ctx->cryptonight_kernel != NULL) {
    clReleaseKernel(ctx->cryptonight_kernel);
  }
  if (ctx->cryptonight_program != NULL) {
    clReleaseProgram(ctx->cryptonight_program);
  }
  if (ctx->command_queue != NULL) {
    clReleaseCommandQueue(ctx->command_queue);
  }
  if (ctx->cl_ctx != NULL) {
    clReleaseContext(ctx->cl_ctx);
  }
  free(ctx);
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

  ret = clGetDeviceInfo(gpu->device_id, CL_DEVICE_LOCAL_MEM_SIZE,
                        sizeof(cl_ulong), &gpu->device_local_memsize, NULL);
  if (ret != CL_SUCCESS) {
    log_error("Error when querying CL_DEVICE_LOCAL_MEM_SIZE");
    return false;
  }
  log_debug("Device query success: CL_DEVICE_LOCAL_MEM_SIZE: %lu",
            gpu->device_local_memsize);

  return true;
}

bool monero_solver_cl_context_prepare_kernel(
    struct monero_solver_cl_context *ctx)
{
  cl_int ret;

  log_debug("Initializing command queue");
#ifdef CL_VERSION_2_0
  const cl_queue_properties queue_prop[] = {0, 0, 0};
  ctx->command_queue = clCreateCommandQueueWithProperties(
      ctx->cl_ctx, ctx->device_id, queue_prop, &ret);
#else
  const cl_command_queue_properties queue_prop = {0};
  ctx->command_queue =
      clCreateCommandQueue(ctx->cl_ctx, ctx->device_id, queue_prop, &ret);
#endif

  if (ret != CL_SUCCESS) {
    log_error("Error when calling clCreateCommandQueueWithProperties: %s",
              cl_err_str(ret));
    return false;
  }

  log_debug("Creating CL kernel from source");
  ctx->cryptonight_program = clCreateProgramWithSource(
      ctx->cl_ctx, 1, &RC_CL_SOURCE_CRYPTONIGHT, NULL, &ret);
  if (ret != CL_SUCCESS) {
    log_error("Error when calling clCreateProgramWithSource: ",
              cl_err_str(ret));
    return false;
  }

  char build_options[256] = {""};
  log_debug("Compiling CL kernel with options: %s", build_options);
  ret = clBuildProgram(ctx->cryptonight_program, 1, &ctx->device_id,
                       build_options, NULL, NULL);
  if (ret != CL_SUCCESS) {
    log_error("Error when calling  clBuildProgram: %s", cl_err_str(ret));
    size_t build_log_len;
    ret = clGetProgramBuildInfo(ctx->cryptonight_program, ctx->device_id,
                                CL_PROGRAM_BUILD_LOG, 0, NULL, &build_log_len);
    if (ret != CL_SUCCESS) {
      log_error("Error when calling clGetProgramBuildInfo: %s",
                cl_err_str(ret));
      return false;
    }
    char *build_log = calloc(1, build_log_len + 1);
    ret = clGetProgramBuildInfo(ctx->cryptonight_program, ctx->device_id,
                                CL_PROGRAM_BUILD_LOG, build_log_len, build_log,
                                NULL);
    if (ret != CL_SUCCESS) {
      log_error("Error when calling clGetProgramBuildInfo: %s.",
                cl_err_str(ret));
    } else {
      log_error("Build log:\n%s", build_log);
    }

    free(build_log);
    return false;
  }

  cl_build_status status;
  do {
    ret = clGetProgramBuildInfo(ctx->cryptonight_program, ctx->device_id,
                                CL_PROGRAM_BUILD_STATUS,
                                sizeof(cl_build_status), &status, NULL);
    if (ret != CL_SUCCESS) {
      log_error("Error when calling clGetProgramBuildInfo: %s",
                cl_err_str(ret));
      return false;
    }
    port_sleep(1);

  } while (status == CL_BUILD_IN_PROGRESS);

  ctx->cryptonight_kernel =
      clCreateKernel(ctx->cryptonight_program, "cryptonight", &ret);
  if (ret != CL_SUCCESS) {
    log_error("Error when calling clCreateKernel for kernel cryptonigth: %s",
              cl_err_str(ret));
    return false;
  }

  // buffers
  ctx->input_buffer = clCreateBuffer(ctx->cl_ctx, CL_MEM_READ_ONLY,
                                     INPUT_BUFFER_SIZE, NULL, &ret);
  if (ret != CL_SUCCESS) {
    log_error("Error when calling clCreateBuffer for input buffer: %s",
              cl_err_str(ret));
    return false;
  }

  ctx->scratchpad_buffer = clCreateBuffer(ctx->cl_ctx, CL_MEM_READ_WRITE,
                                          SCRATCHPAD_BUFFER_SIZE, NULL, &ret);
  if (ret != CL_SUCCESS) {
    log_error("Error when calling clCreateBuffer for scratchpad buffer: %s",
              cl_err_str(ret));
    return false;
  }

  ctx->output_buffer = clCreateBuffer(ctx->cl_ctx, CL_MEM_WRITE_ONLY,
                                      OUTPUT_BUFFER_SIZE, NULL, &ret);
  if (ret != CL_SUCCESS) {
    log_error("Error when calling clCreateBuffer for output buffer: %s",
              cl_err_str(ret));
    return false;
  }

  log_debug("Successfully created cryptonight kernel");
  return true;
}
