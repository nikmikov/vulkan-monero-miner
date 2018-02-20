#include "monero/monero_solver.h"

#include "logging.h"
#include <assert.h>
#include <uv.h>

#include "monero.h"

#include "utils/opencl_inc.h"

#include "resources.h"
#include "utils/opencl_err.h"
#include "utils/port_sleep.h"

#include "crypto/blake.h"
#include "crypto/cryptonight/cryptonight.h"
#include "crypto/groestl.h"
#include "crypto/jh.h"
#include "crypto/skein.h"

#define INPUT_BUFFER_SIZE MONERO_INPUT_HASH_LEN
#define SCRATCHPAD_BUFFER_SIZE(threads)                                        \
  ((size_t)threads * MONERO_CRYPTONIGHT_MEMORY)
#define OUTPUT_BUFFER_SIZE(threads) ((size_t)threads * 200)

struct monero_solver_cl_context {
  /** Config options */
  size_t intensity;
  size_t worksize;

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
  cl_ulong device_max_memalloc_size;
  cl_ulong device_local_memsize;
};

struct monero_solver_cl_context *
monero_solver_cl_context_init(cl_uint platform_id, cl_uint device_id,
                              size_t intensity, size_t worksize);

bool monero_solver_cl_context_prepare_kernel(
    struct monero_solver_cl_context *cl);

bool monero_solver_cl_context_query_device(struct monero_solver_cl_context *cl);

void monero_solver_cl_context_release(struct monero_solver_cl_context *cl);

struct monero_solver_cl {
  struct monero_solver solver;

  /** CL context */
  struct monero_solver_cl_context *cl;

  /** output buffer */
  uint8_t *output_buffer;

  const uint8_t *input_hash;
  size_t input_hash_len;
  uint64_t target;
  uint8_t *output_hash;
  uint32_t *output_nonces;
  size_t *output_num;

  /** cryptonight context */
  struct cryptonight_ctx *cryptonight_ctx;
  struct cryptonight_hash cryptonight_output_hash;
};

bool monero_solver_cl_set_job(struct monero_solver *ptr,
                              const uint8_t *input_hash, size_t input_hash_len,
                              const uint64_t target, uint8_t *output_hash,
                              uint32_t *output_nonces, size_t *output_num)
{
  if (input_hash_len > 84) {
    return false;
  }
  struct monero_solver_cl *solver = (struct monero_solver_cl *)ptr;
  solver->input_hash = input_hash;
  solver->input_hash_len = input_hash_len;
  solver->target = target;
  solver->output_hash = output_hash;
  solver->output_nonces = output_nonces;
  solver->output_num = output_num;

  uint8_t input_buffer[INPUT_BUFFER_SIZE];
  memcpy(input_buffer, input_hash, input_hash_len);

  // padding
  if (input_hash_len < MONERO_INPUT_HASH_LEN) {
    const size_t zero_from = input_hash_len + 1;
    memset(input_buffer + zero_from, 0, MONERO_INPUT_HASH_LEN - zero_from);
    input_buffer[input_hash_len] = 0x01;
  }

  cl_uint ret;
  struct monero_solver_cl_context *ctx = solver->cl;
  assert(ctx != NULL);
  // INPUT BUFFER DATA
  ret = clEnqueueWriteBuffer(ctx->command_queue, ctx->input_buffer, CL_TRUE, 0,
                             INPUT_BUFFER_SIZE, input_buffer, 0, NULL, NULL);

  if (ret != CL_SUCCESS) {
    log_error("Error when calling clEnqueueWriteBuffer with input data: %s",
              cl_err_str(ret));
    return false;
  }

  // INPUT
  ret = clSetKernelArg(ctx->cryptonight_kernel, 0, sizeof(cl_mem),
                       &ctx->input_buffer);
  if (ret != CL_SUCCESS) {
    log_error("Error when calling clSetKernelArg for arg #0[input_buffer]: %s",
              cl_err_str(ret));
    return false;
  }

  // SCRATCHPAD
  ret = clSetKernelArg(ctx->cryptonight_kernel, 1, sizeof(cl_mem),
                       &ctx->scratchpad_buffer);
  if (ret != CL_SUCCESS) {
    log_error(
        "Error when calling clSetKernelArg for arg #1[scratchpad_buffer]: %s",
        cl_err_str(ret));
    return false;
    ;
  }

  // OUTPUT BUFFER
  ret = clSetKernelArg(ctx->cryptonight_kernel, 2, sizeof(cl_mem),
                       &ctx->output_buffer);
  if (ret != CL_SUCCESS) {
    log_error("Error when calling clSetKernelArg for arg #2[output_buffer]: %s",
              cl_err_str(ret));
    return false;
  }
  return true;
}

static inline void print_debug(const char *s, uint8_t *mem, size_t N)
{
  printf("CPU: %s", s);
  for (size_t i = 0; i < N; ++i) {
    printf("%02hhx", mem[i]);
  }
  printf("\n");
}

// *output_hash: SOLUTIONS_BUFFER_SIZE * MONERO_OUTPUT_HASH_LEN
int monero_solver_cl_process(struct monero_solver *ptr, uint32_t nonce_from)
{
  cl_uint ret;
  struct monero_solver_cl *solver = (struct monero_solver_cl *)ptr;
  struct monero_solver_cl_context *ctx = solver->cl;
  assert(ctx != NULL);

  const size_t global_work_size = solver->cl->intensity;
  const size_t local_work_size = solver->cl->worksize;
  const size_t output_buffer_size = OUTPUT_BUFFER_SIZE(global_work_size);

  size_t global_offset = nonce_from;

  ret = clEnqueueNDRangeKernel(ctx->command_queue, ctx->cryptonight_kernel, 1,
                               &global_offset, &global_work_size,
                               &local_work_size, 0, NULL, NULL);
  if (ret != CL_SUCCESS) {
    log_error("Error when calling clEnqueueNDRangeKernel: %s", cl_err_str(ret));
    return -1;
  }

  // READ RESULTS
  ret = clEnqueueReadBuffer(ctx->command_queue, ctx->output_buffer, CL_TRUE, 0,
                            output_buffer_size, solver->output_buffer, 0, NULL,
                            NULL);
  if (ret != CL_SUCCESS) {
    log_error("Error when calling clEnqueueReadBuffer to fetch results: %s",
              cl_err_str(ret));
    return -1;
  }

  static void (*const extra_hashes[4])(const void *, size_t, uint8_t *) = {
      blake_256, groestl_256, jh_256, skein_512_256};

  static const size_t OUT_SZ = 200;
  uint8_t output[256];
  *solver->output_num = 0;

  for (size_t i = 0; i < global_work_size; ++i) {
    uint8_t *hash_state = solver->output_buffer + i * OUT_SZ;
    const int final_hash_idx = hash_state[0] & 3;
    extra_hashes[final_hash_idx](hash_state, OUT_SZ * 8, output);

    if (monero_solution_hash_val(output) < solver->target) {
      uint32_t nonce = nonce_from + (uint32_t)i;
      log_debug("Solution found: %x!", nonce);
      // solution found
      memcpy(solver->output_hash, output, MONERO_OUTPUT_HASH_LEN);
      (*solver->output_nonces++) = nonce;
      ++(*solver->output_num);
    }

    // compare against CPU version
    //#define  __VERIFY_CL_
#ifdef __VERIFY_CL_
    log_debug("Verifying results");
    printf("+++: %d\n", final_hash_idx);
    *(uint32_t *)&solver->input_hash[MONERO_NONCE_POSITION] = nonce_from + i;
    cryptonight_aesni(solver->input_hash, solver->input_hash_len,
                      &solver->cryptonight_output_hash,
                      solver->cryptonight_ctx);
    print_debug("FINAL HASH CL:: ", output, 32);
    print_debug("FINAL HASH CPU: ", &solver->cryptonight_output_hash, 32);
    if (memcmp(output, &solver->cryptonight_output_hash, 32) != 0) {
      log_error("Don't match: %lu", i);
      exit(1);
    } else {
      log_info("Match: %lu", i);
    }
#endif
  }

  return (int)global_work_size;
}

void monero_solver_cl_free(struct monero_solver *ptr)
{
  struct monero_solver_cl *solver = (struct monero_solver_cl *)ptr;

  free(solver->output_buffer);
  if (solver->cl != NULL) {
    monero_solver_cl_context_release(solver->cl);
  }

  free(ptr);
}

struct monero_solver *
monero_solver_new_cl(const struct monero_config_solver_cl *cfg)
{
  assert(cfg != NULL);
  // init gpu
  assert(cfg->platform_id >= 0);
  assert(cfg->device_id >= 0);
  assert(cfg->worksize >= 0);
  assert(cfg->intensity >= 0);

  struct monero_solver_cl_context *cl = monero_solver_cl_context_init(
      (cl_uint)cfg->platform_id, (cl_uint)cfg->device_id,
      (size_t)cfg->intensity, (size_t)cfg->worksize);

  if (cl == NULL) {
    log_error("Error when initializing opencl device");
    return NULL;
  }

  struct monero_solver_cl *solver_cl =
      calloc(1, sizeof(struct monero_solver_cl));

  solver_cl->cryptonight_ctx = cryptonight_ctx_new();

  solver_cl->output_buffer = calloc(1, OUTPUT_BUFFER_SIZE(cfg->intensity));

  solver_cl->cl = cl;

  solver_cl->solver.set_job = monero_solver_cl_set_job;
  solver_cl->solver.process = monero_solver_cl_process;
  solver_cl->solver.free = monero_solver_cl_free;

  if (monero_solver_init(&cfg->solver, &solver_cl->solver)) {
    return &solver_cl->solver;
  } else {
    monero_solver_cl_context_release(solver_cl->cl);
    free(solver_cl);
    return NULL;
  }
}

struct monero_solver_cl_context *
monero_solver_cl_context_init(cl_uint platform_id, cl_uint device_id,
                              size_t intensity, size_t worksize)
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

  struct monero_solver_cl_context *ctx =
      calloc(1, sizeof(struct monero_solver_cl_context));
  ctx->intensity = intensity;
  ctx->worksize = worksize;

  ctx->cl_ctx = cl_ctx;
  ctx->device_id = devices[device_id];

  if (!monero_solver_cl_context_query_device(ctx)) {
    log_error("Failed to query OpenCL device capabilities");
    monero_solver_cl_context_release(ctx);
    return NULL;
  }

  if (!monero_solver_cl_context_prepare_kernel(ctx)) {
    log_error("Failed initialize OpenCL solver kernel");
    monero_solver_cl_context_release(ctx);
    return NULL;
  }

  return ctx;
}

void monero_solver_cl_context_release(struct monero_solver_cl_context *ctx)
{
  if (ctx->scratchpad_buffer != NULL) {
    clReleaseMemObject(ctx->scratchpad_buffer);
  }
  if (ctx->output_buffer != NULL) {
    clReleaseMemObject(ctx->output_buffer);
  }
  if (ctx->input_buffer != NULL) {
    clReleaseMemObject(ctx->input_buffer);
  }
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

  ret = clGetDeviceInfo(gpu->device_id, CL_DEVICE_MAX_MEM_ALLOC_SIZE,
                        sizeof(cl_ulong), &gpu->device_max_memalloc_size, NULL);
  if (ret != CL_SUCCESS) {
    log_error("Error when querying CL_DEVICE_MAX_MEM_ALLOC_SIZE");
    return false;
  }
  log_debug("Device query success: CL_DEVICE_MAX_MEM_ALLOC_SIZE: %lu",
            gpu->device_max_memalloc_size);

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

  const size_t scratchpad_buffer_size = SCRATCHPAD_BUFFER_SIZE(ctx->intensity);
  log_debug("Allocating scratchpad buffer of %lu bytes",
            scratchpad_buffer_size);
  ctx->scratchpad_buffer =
      clCreateBuffer(ctx->cl_ctx, CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS,
                     scratchpad_buffer_size, NULL, &ret);
  if (ret != CL_SUCCESS) {
    log_error("Error when calling clCreateBuffer for scratchpad buffer of "
              "size(%lu): %s",
              scratchpad_buffer_size, cl_err_str(ret));
    return false;
  }

  ctx->output_buffer =
      clCreateBuffer(ctx->cl_ctx, CL_MEM_WRITE_ONLY,
                     OUTPUT_BUFFER_SIZE(ctx->intensity), NULL, &ret);
  if (ret != CL_SUCCESS) {
    log_error("Error when calling clCreateBuffer for output buffer: %s",
              cl_err_str(ret));
    return false;
  }

  log_debug("Successfully created cryptonight kernel");
  return true;
}
