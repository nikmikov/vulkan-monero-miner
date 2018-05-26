#include "monero/monero_solver.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

#include "crypto/cryptonight_spv.h"
#include "crypto/cryptonight/cryptonight.h"
#include "logging.h"

#define CRYPTONIGHT_STATE_SIZE 200

enum BUFFERS { INPUT_BUFFER = 0, STATE_BUFFER, SCRATCHPAD_BUFFER, NUM_BUFFERS };

enum PIPELINES {
  PIPELINE_INIT = 0,
  PIPELINE_KECCAK,
  PIPELINE_EXPLODE,
  PIPELINE_MEMLOOP,
  NUM_COMPUTE_PIPELINES
};

struct monero_solver_vk_context {
  uint32_t device_idx;
  VkInstance instance;
  VkPhysicalDevice physical_device;
  VkPhysicalDeviceProperties physical_device_properties;
  VkDevice device;
  VkQueue queue;
  VkCommandPool cmd_pool;
  VkDescriptorPool descriptor_pool;

  // memory and buffers
  VkDeviceMemory memory[NUM_BUFFERS];
  VkBuffer buffer[NUM_BUFFERS];
  void *input_mmapped;
  void *output_mmapped;
  void *scratchpad_mmapped;

  // shaders
  VkCommandBuffer cmd_buffer;
  VkShaderModule compute_shader[NUM_COMPUTE_PIPELINES];
  VkDescriptorSetLayout descriptor_set_layout[NUM_COMPUTE_PIPELINES];
  VkDescriptorSet descriptor_set[NUM_COMPUTE_PIPELINES];
  VkPipelineLayout pipeline_layout[NUM_COMPUTE_PIPELINES];
  VkPipeline pipeline[NUM_COMPUTE_PIPELINES];
};

struct monero_solver_vk {
  struct monero_solver solver;

  /** Vulkan Context */
  struct monero_solver_vk_context *vk;

  /** parallelizm */
  size_t parallelism;

  /** Job params */
  const uint8_t *input_hash;
  size_t input_hash_len;
  uint64_t target;
  uint8_t *output_hash;
  uint32_t *output_nonces;
  size_t *output_num;
};

struct monero_solver_vk_context *
monero_solver_vk_context_init(uint32_t device_idx);

void monero_solver_vk_context_release(struct monero_solver_vk_context *ctx);

bool monero_solver_vk_context_prepare_pipelines(
    struct monero_solver_vk_context *cl);

bool monero_solver_vk_context_prepare_buffers(
    struct monero_solver_vk_context *vk, size_t parallelism);

bool monero_solver_vk_context_prepare_command_buffer(
    struct monero_solver_vk_context *vk, size_t parallelism);

void monero_solver_vk_free(struct monero_solver *ptr)
{
  struct monero_solver_vk *solver = (struct monero_solver_vk *)ptr;

  if (solver->vk != NULL) {
    monero_solver_vk_context_release(solver->vk);
  }

  free(ptr);
}

static inline void print_debug(const char *s, uint8_t *mem, size_t N)
{
  printf("CPU: %s", s);
  for (size_t i = 0; i < N; ++i) {
    printf("%02hhx", mem[i]);
  }
  printf("\n");
}

bool monero_solver_vk_set_job(struct monero_solver *ptr,
                              const uint8_t *input_hash, size_t input_hash_len,
                              const uint64_t target, uint8_t *output_hash,
                              uint32_t *output_nonces, size_t *output_num)
{
  if (input_hash_len > MONERO_INPUT_HASH_LEN) {
    return false;
  }
  struct monero_solver_vk *solver = (struct monero_solver_vk *)ptr;
  struct monero_solver_vk_context *vk = solver->vk;
  solver->input_hash = input_hash;
  solver->input_hash_len = input_hash_len;
  solver->target = target;
  solver->output_hash = output_hash;
  solver->output_nonces = output_nonces;
  solver->output_num = output_num;

  struct {
    uint32_t nonce;
    uint8_t hash[CRYPTONIGHT_STATE_SIZE];
  } input_data = {0};

  memcpy(input_data.hash, input_hash, input_hash_len);
  // padding
  if (input_hash_len < MONERO_INPUT_HASH_LEN) {
    const size_t zero_from = input_hash_len + 1;
    memset(input_data.hash + zero_from, 0, CRYPTONIGHT_STATE_SIZE - zero_from);
    input_data.hash[input_hash_len] = 0x01;
    input_data.hash[135] = 0x80;
  }

  // copy memory to GPU
  memcpy(vk->input_mmapped, &input_data, sizeof(input_data));
  return true;
}

int monero_solver_vk_process(struct monero_solver *ptr, uint32_t nonce_from)
{
  struct monero_solver_vk *solver = (struct monero_solver_vk *)ptr;
  struct monero_solver_vk_context *vk = solver->vk;

  *(uint32_t *)vk->input_mmapped = nonce_from;

  VkSubmitInfo submit_info = {
      VK_STRUCTURE_TYPE_SUBMIT_INFO, NULL, 0, 0, 0, 1, &vk->cmd_buffer, 0, 0};

  VkResult vk_res = vkQueueSubmit(vk->queue, 1, &submit_info, 0);
  if (vk_res != VK_SUCCESS) {
    log_error("Error when calling vkQueueSubmit");
    return -1;
  }

  //  log_info("Waiting for queue");
  vk_res = vkQueueWaitIdle(vk->queue);
  if (vk_res != VK_SUCCESS) {
    log_error("Error when calling vkQueueWaitIdle");
    return -1;
  }

  // read results and compare with CPU version of keccak
#define __VERIFY_VK_
#ifdef __VERIFY_VK_

  struct cryptonight_ctx *cryptonight_ctx = cryptonight_ctx_new();
  struct cryptonight_hash output_cpu;

  log_debug("Verifying results");
  for (size_t k = 0; k < solver->parallelism; ++k) {

    *(uint32_t *)&solver->input_hash[MONERO_NONCE_POSITION] = nonce_from + k;
//    keccak_256(keccak_state, CRYPTONIGHT_STATE_SIZE, solver->input_hash,
//               solver->input_hash_len);

    cryptonight_aesni(solver->input_hash, solver->input_hash_len,
                      &output_cpu, cryptonight_ctx);


    print_debug("FINAL HASH CPU: ", &output_cpu, 200);


    uint8_t *output =
      (uint8_t *)vk->scratchpad_mmapped + k * MONERO_CRYPTONIGHT_MEMORY;
    print_debug("FINAL HASH VK:: ", output, 200);

    if (memcmp(output, &output_cpu, 200) != 0) {
      log_error("Don't match: %lu", k);
    } else {
      log_info("Match: %lu", k);
    }
  }
  exit(1);
#endif

  return solver->parallelism;
}

struct monero_solver *
monero_solver_new_vk(const struct monero_config_solver_vk *cfg)
{
  ////////////////////////////// TEMORARY DEBUG ///////////////////////////
#if 0
  log_info("Dumping shader: %p: %lu", cryptonight_memloop_shader,
           cryptonight_memloop_shader_size);
  FILE *f = fopen("/home/fedor/src/dorenom/src/crypto/binary.spv", "w");
  fwrite((void *)cryptonight_memloop_shader, 1, cryptonight_memloop_shader_size,
         f);
  fclose(f);
  log_info("Wrote %lu, bytes", cryptonight_memloop_shader_size);
  exit(1);
#endif
  ////////////////////////////// TEMORARY DEBUG ///////////////////////////

  assert(cfg != NULL);
  assert(cfg->device_id >= 0);
  struct monero_solver_vk_context *vk_ctx =
      monero_solver_vk_context_init((uint32_t)cfg->device_id);
  if (vk_ctx == NULL) {
    log_error("Error when creating Vulkan Context");
    return NULL;
  }

  // init memory buffers
  if (!monero_solver_vk_context_prepare_buffers(vk_ctx, cfg->parallelism)) {
    log_error("Error when initializing memory buffers");
    monero_solver_vk_context_release(vk_ctx);
    return NULL;
  }

  // init compute shaders and pipelines
  if (!monero_solver_vk_context_prepare_pipelines(vk_ctx)) {
    log_error("Error when initializing compute pipelines");
    monero_solver_vk_context_release(vk_ctx);
    return NULL;
  }

  // init command buffer
  if (!monero_solver_vk_context_prepare_command_buffer(vk_ctx,
                                                       cfg->parallelism)) {
    log_error("Error when initializing command buffers");
    monero_solver_vk_context_release(vk_ctx);
    return NULL;
  }

  struct monero_solver_vk *solver_vk =
      calloc(1, sizeof(struct monero_solver_vk));

  solver_vk->vk = vk_ctx;
  solver_vk->parallelism = cfg->parallelism;
  solver_vk->solver.set_job = monero_solver_vk_set_job;
  solver_vk->solver.process = monero_solver_vk_process;
  solver_vk->solver.free = monero_solver_vk_free;

  if (monero_solver_init(&cfg->solver, &solver_vk->solver)) {
    return &solver_vk->solver;
  } else {
    monero_solver_vk_context_release(solver_vk->vk);
    free(solver_vk);
    return NULL;
  }
}

struct monero_solver_vk_context *
monero_solver_vk_context_init(uint32_t device_idx)
{
  struct monero_solver_vk_context *ctx =
      calloc(1, sizeof(struct monero_solver_vk_context));

  const VkApplicationInfo application_info = {
      VK_STRUCTURE_TYPE_APPLICATION_INFO,
      0,
      "MorBirMoneroSolver",
      0,
      "",
      0,
      VK_MAKE_VERSION(1, 0, 9)};

  const VkInstanceCreateInfo instance_create_info = {
      VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      0,
      0,
      &application_info,
      0,
      0,
      0,
      0};

  VkResult vk_res;

  // create instance
  vk_res = vkCreateInstance(&instance_create_info, 0, &ctx->instance);
  if (vk_res != VK_SUCCESS) {
    log_error("Error when calling vkCreateInstance");
    goto ERROR;
  }

  // get number of physical devices in the system
  uint32_t physical_device_count = 0;
  vk_res = vkEnumeratePhysicalDevices(ctx->instance, &physical_device_count, 0);
  if (vk_res != VK_SUCCESS) {
    log_error("Error when calling vkEnumeratePhysicalDevices");
    goto ERROR;
  }
  log_debug("Physical devices found: %u", physical_device_count);
  if (device_idx >= physical_device_count) {
    log_error("Invalid device id: %u, total number of devices found: %u",
              device_idx, physical_device_count);
    goto ERROR;
  }

  // get physical device handle
  VkPhysicalDevice *physical_devices =
      calloc(physical_device_count, sizeof(VkPhysicalDevice));
  vk_res = vkEnumeratePhysicalDevices(ctx->instance, &physical_device_count,
                                      physical_devices);
  if (vk_res != VK_SUCCESS) {
    log_error("Error when calling vkEnumeratePhysicalDevices");
    free(physical_devices);
    goto ERROR;
  }
  ctx->physical_device = physical_devices[device_idx];
  free(physical_devices);

  // get physical device properties
  vkGetPhysicalDeviceProperties(ctx->physical_device,
                                &ctx->physical_device_properties);
  log_info("Initializing GPU(%u): %s", device_idx,
           ctx->physical_device_properties.deviceName);

  // request compute queue
  uint32_t queue_family_count;
  const float default_queue_priority = 0.0f;
  VkDeviceQueueCreateInfo queue_create_info = {0};
  vkGetPhysicalDeviceQueueFamilyProperties(ctx->physical_device,
                                           &queue_family_count, NULL);

  VkQueueFamilyProperties *queue_family_properties =
      calloc(queue_family_count, sizeof(VkQueueFamilyProperties));
  vkGetPhysicalDeviceQueueFamilyProperties(
      ctx->physical_device, &queue_family_count, queue_family_properties);

  uint32_t queue_family_index = queue_family_count;
  for (uint32_t i = 0; i < queue_family_count; ++i) {
    if (queue_family_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
      queue_family_index = i;
      queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queue_create_info.queueFamilyIndex = i;
      queue_create_info.queueCount = 1;
      queue_create_info.pQueuePriorities = &default_queue_priority;
      break;
    }
  }
  free(queue_family_properties);

  if (queue_family_index == queue_family_count) {
    log_error("Compute queue not found for device: %u", device_idx);
    goto ERROR;
  }

  // Create logical device
  VkDeviceCreateInfo device_create_info = {0};
  device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_create_info.queueCreateInfoCount = 1;
  device_create_info.pQueueCreateInfos = &queue_create_info;
  vk_res = vkCreateDevice(ctx->physical_device, &device_create_info, NULL,
                          &ctx->device);
  if (vk_res != VK_SUCCESS) {
    log_error("Error when calling vkCreateDevice");
    goto ERROR;
  }

  // Get a compute queue
  vkGetDeviceQueue(ctx->device, queue_family_index, 0, &ctx->queue);

  // Compute command pool
  VkCommandPoolCreateInfo cmd_pool_info = {0};
  cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  cmd_pool_info.queueFamilyIndex = queue_family_index;
  cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  vk_res =
      vkCreateCommandPool(ctx->device, &cmd_pool_info, NULL, &ctx->cmd_pool);
  if (vk_res != VK_SUCCESS) {
    log_error("Error when calling vkCreateCommandPool");
    goto ERROR;
  }

  VkCommandBufferAllocateInfo command_buffer_allocate_info = {
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, NULL, ctx->cmd_pool,
      VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1};

  vk_res = vkAllocateCommandBuffers(ctx->device, &command_buffer_allocate_info,
                                    &ctx->cmd_buffer);

  if (vk_res != VK_SUCCESS) {
    log_error("Error when calling vkAllocateCommandBuffers");
    goto ERROR;
  }

  return ctx;

ERROR:
  monero_solver_vk_context_release(ctx);
  return NULL;
}

void monero_solver_vk_context_release(struct monero_solver_vk_context *ctx)
{
  assert(ctx != NULL);

  if (ctx->descriptor_pool != VK_NULL_HANDLE) {
    vkDestroyDescriptorPool(ctx->device, ctx->descriptor_pool, NULL);
  }
  for (size_t i = 0; i < NUM_COMPUTE_PIPELINES; ++i) {
    if (ctx->descriptor_set_layout[i] != VK_NULL_HANDLE) {
      vkDestroyDescriptorSetLayout(ctx->device, ctx->descriptor_set_layout[i],
                                   NULL);
    }
    if (ctx->pipeline_layout[i] != VK_NULL_HANDLE) {
      vkDestroyPipelineLayout(ctx->device, ctx->pipeline_layout[i], NULL);
    }
    if (ctx->pipeline[i] != VK_NULL_HANDLE) {
      vkDestroyPipeline(ctx->device, ctx->pipeline[i], NULL);
    }
  }
  for (size_t i = 0; i < NUM_BUFFERS; ++i) {
    if (ctx->buffer[i] != VK_NULL_HANDLE) {
      vkDestroyBuffer(ctx->device, ctx->buffer[i], NULL);
    }
    if (ctx->memory[i] != VK_NULL_HANDLE) {
      vkUnmapMemory(ctx->device, ctx->memory[i]);
      vkFreeMemory(ctx->device, ctx->memory[i], NULL);
    }
  }
  if (ctx->cmd_pool != VK_NULL_HANDLE) {
    vkDestroyCommandPool(ctx->device, ctx->cmd_pool, NULL);
  }
  if (ctx->device != VK_NULL_HANDLE) {
    vkDestroyDevice(ctx->device, NULL);
  }
  if (ctx->instance != VK_NULL_HANDLE) {
    vkDestroyInstance(ctx->instance, NULL);
  }
  free(ctx);
}

static uint32_t
monero_solver_vk_find_memory(struct monero_solver_vk_context *vk,
                             VkMemoryPropertyFlags memory_property_flags,
                             VkDeviceSize required_memory_size)
{
  VkPhysicalDeviceMemoryProperties properties;
  vkGetPhysicalDeviceMemoryProperties(vk->physical_device, &properties);

  // find appropriate memory
  log_debug("Looking for device memory at least size: %lu",
            required_memory_size);
  for (uint32_t k = 0; k < properties.memoryTypeCount; ++k) {
    VkDeviceSize heap_size =
        properties.memoryHeaps[properties.memoryTypes[k].heapIndex].size;

    bool is_requested_type =
        memory_property_flags & properties.memoryTypes[k].propertyFlags;

    if (is_requested_type && heap_size >= required_memory_size) {
      log_debug("Found suitable memory @index: %u", k);
      return k;
    }
  }
  return VK_MAX_MEMORY_TYPES;
}

bool monero_solver_vk_context_prepare_buffers(
    struct monero_solver_vk_context *vk, size_t parallelism)
{
  VkResult vk_res;

  // calculate required memory size
  const VkDeviceSize buffer_size[NUM_BUFFERS] = {
      sizeof(uint32_t) + CRYPTONIGHT_STATE_SIZE, // input buffer
      CRYPTONIGHT_STATE_SIZE * parallelism,      // state buffer
      MONERO_CRYPTONIGHT_MEMORY * parallelism    // scratchpad buffer
  };

  const VkMemoryPropertyFlags buffer_flags[NUM_BUFFERS] = {
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
  //      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT};

  const VkBufferCreateInfo buffer_create_info[3] = {
      {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, 0, 0, buffer_size[0],
       VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, 1, NULL},
      {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, 0, 0, buffer_size[1],
       VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, 1, NULL},
      {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, 0, 0, buffer_size[2],
       VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, 1, NULL}};

  for (size_t k = 0; k < NUM_BUFFERS; ++k) {
    vk_res = vkCreateBuffer(vk->device, &buffer_create_info[k], NULL,
                            &vk->buffer[k]);
    if (vk_res != VK_SUCCESS) {
      log_error("Error when calling vkCreateBuffer: buffer #%lu of size(%lu)",
                k, buffer_size[k]);
      return false;
    }

    VkMemoryRequirements buffer_memory_requirements;
    // determine buffer memory requirements
    vkGetBufferMemoryRequirements(vk->device, vk->buffer[k],
                                  &buffer_memory_requirements);

    // find appropriate memory
    uint32_t memory_type_index =
        monero_solver_vk_find_memory(vk, buffer_flags[k], buffer_size[k]);
    if (memory_type_index == VK_MAX_MEMORY_TYPES) {
      log_error("Could not find suitable device memory for buffer #%lu. At "
                "least size: %lu is required",
                k, buffer_memory_requirements.size);
      return false;
    }

    const VkMemoryAllocateInfo memory_allocate_info = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, 0,
        buffer_memory_requirements.size, memory_type_index};

    vk_res =
        vkAllocateMemory(vk->device, &memory_allocate_info, 0, &vk->memory[k]);
    if (vk_res != VK_SUCCESS) {
      log_error("Error when calling vkAllocateMemory: size of(%lu)",
                buffer_memory_requirements.size);
      return false;
    }
    log_debug("Successfully allocated %lu bytes of memory @memory type %u",
              buffer_memory_requirements.size, memory_type_index);

    vk_res = vkBindBufferMemory(vk->device, vk->buffer[k], vk->memory[k], 0);
    if (vk_res != VK_SUCCESS) {
      log_error("Error when calling vkBindBufferMemory: buffer #%lu", k);
      return false;
    }
  }

  // map input/output buffers to host memory
  vk_res = vkMapMemory(vk->device, vk->memory[INPUT_BUFFER], 0, VK_WHOLE_SIZE,
                       0, &vk->input_mmapped);

  if (vk_res != VK_SUCCESS) {
    log_error("Error when calling vkMapMemory for input buffer: %d",
              (int)vk_res);
    return false;
  }

  vk_res = vkMapMemory(vk->device, vk->memory[STATE_BUFFER], 0, VK_WHOLE_SIZE,
                       0, &vk->output_mmapped);

  if (vk_res != VK_SUCCESS) {
    log_error("Error when calling vkMapMemory for output buffer: %d",
              (int)vk_res);
    return false;
  }

  // TODO: remove temporary debug
  vk_res = vkMapMemory(vk->device, vk->memory[SCRATCHPAD_BUFFER], 0, VK_WHOLE_SIZE,
                       0, &vk->scratchpad_mmapped);

  if (vk_res != VK_SUCCESS) {
    log_error("Error when calling vkMapMemory for scratchpad buffer: %d",
              (int)vk_res);
    return false;
  }

  return true;
}

bool monero_solver_vk_context_prepare_pipelines(
    struct monero_solver_vk_context *vk)
{
  VkResult vk_res;
  VkShaderModuleCreateInfo cn_init_create_info = {
      VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, 0, 0,
      cryptonight_init_shader_size, cryptonight_init_shader};

  vk_res = vkCreateShaderModule(vk->device, &cn_init_create_info, 0,
                                &vk->compute_shader[PIPELINE_INIT]);
  if (vk_res != VK_SUCCESS) {
    log_error("Error when calling vkCreateShaderModule for cn_init shader");
    return false;
  }
  log_debug("`cn_init` shader initialized");

  VkShaderModuleCreateInfo cn_keccak_create_info = {
      VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, 0, 0,
      cryptonight_keccak_shader_size, cryptonight_keccak_shader};

  vk_res = vkCreateShaderModule(vk->device, &cn_keccak_create_info, 0,
                                &vk->compute_shader[PIPELINE_KECCAK]);
  if (vk_res != VK_SUCCESS) {
    log_error("Error when calling vkCreateShaderModule for `cn_keccak` shader");
    return false;
  }
  log_debug("`cn_keccak` shader initialized");

  VkShaderModuleCreateInfo cn_explode_create_info = {
      VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, 0, 0,
      cryptonight_explode_shader_size, cryptonight_explode_shader};

  vk_res = vkCreateShaderModule(vk->device, &cn_explode_create_info, 0,
                                &vk->compute_shader[PIPELINE_EXPLODE]);
  if (vk_res != VK_SUCCESS) {
    log_error(
        "Error when calling vkCreateShaderModule for `cn_explode` shader");
    return false;
  }
  log_debug("`cn_explode` shader initialized");

  VkShaderModuleCreateInfo cn_memloop_create_info = {
      VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, 0, 0,
      cryptonight_memloop_shader_size, cryptonight_memloop_shader};

  vk_res = vkCreateShaderModule(vk->device, &cn_memloop_create_info, 0,
                                &vk->compute_shader[PIPELINE_MEMLOOP]);
  if (vk_res != VK_SUCCESS) {
    log_error(
        "Error when calling vkCreateShaderModule for `cn_memloop` shader");
    return false;
  }
  log_debug("`cn_memloop` shader initialized");

  // descriptors set
  VkDescriptorSetLayoutBinding input_descriptor_set_layout_bindings[2] = {
      {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, 0},
      {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT,
       0}};

  VkDescriptorSetLayoutBinding keccak_descriptor_set_layout_bindings[1] = {
      {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT,
       0}};

  VkDescriptorSetLayoutBinding explode_descriptor_set_layout_bindings[2] = {
      {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, 0},
      {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT,
       0}};

  VkDescriptorSetLayoutBinding memloop_descriptor_set_layout_bindings[2] = {
      {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, 0},
      {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT,
       0}};

  VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info[4] = {
      {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, 0, 0, 2,
       input_descriptor_set_layout_bindings},
      {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, 0, 0, 1,
       keccak_descriptor_set_layout_bindings},
      {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, 0, 0, 2,
       explode_descriptor_set_layout_bindings},
      {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, 0, 0, 2,
       memloop_descriptor_set_layout_bindings}};

  for (size_t k = 0; k < NUM_COMPUTE_PIPELINES; ++k) {
    vk_res = vkCreateDescriptorSetLayout(vk->device,
                                         &descriptor_set_layout_create_info[k],
                                         NULL, &vk->descriptor_set_layout[k]);

    if (vk_res != VK_SUCCESS) {
      log_error(
          "Error when calling vkCreateDescriptorSetLayout for shader #%lu", k);
      return false;
    }

    // pipeline layouts
    VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        0,
        0,
        1,
        &vk->descriptor_set_layout[k],
        0,
        0};

    vk_res = vkCreatePipelineLayout(vk->device, &pipeline_layout_create_info,
                                    NULL, &vk->pipeline_layout[k]);

    if (vk_res != VK_SUCCESS) {
      log_error("Error when calling vkCreatePipelineLayout for shader #%lu", k);
      return false;
    }

    VkComputePipelineCreateInfo compute_pipeline_create_info = {
        VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        0,
        0,
        {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, 0, 0,
         VK_SHADER_STAGE_COMPUTE_BIT, vk->compute_shader[k], "main", 0},
        vk->pipeline_layout[k],
        0,
        0};

    vk_res = vkCreateComputePipelines(
        vk->device, 0, 1, &compute_pipeline_create_info, 0, &vk->pipeline[k]);
    if (vk_res != VK_SUCCESS) {
      log_error("Error when calling vkCreateComputePipelines for shader #%lu",
                k);
      return false;
    }
  }

  return true;
}

bool monero_solver_vk_context_prepare_command_buffer(
    struct monero_solver_vk_context *vk, size_t parallelism)
{
  VkResult vk_res;

  VkDescriptorPoolSize descriptor_pool_size[NUM_COMPUTE_PIPELINES] = {
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2}};

  VkDescriptorPoolCreateInfo descriptor_pool_create_info = {
      VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      NULL,
      0,
      NUM_COMPUTE_PIPELINES,
      NUM_COMPUTE_PIPELINES,
      descriptor_pool_size};

  vk_res = vkCreateDescriptorPool(vk->device, &descriptor_pool_create_info, 0,
                                  &vk->descriptor_pool);
  if (vk_res != VK_SUCCESS) {
    log_error("Error when calling vkCreateDescriptorPool");
    return false;
  }

  // populate descriptor set
  VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, 0, vk->descriptor_pool,
      NUM_COMPUTE_PIPELINES, vk->descriptor_set_layout};

  vk_res = vkAllocateDescriptorSets(vk->device, &descriptor_set_allocate_info,
                                    vk->descriptor_set);
  if (vk_res != VK_SUCCESS) {
    log_error("Error when calling vkAllocateDescriptorSets");
    return false;
  }

  VkDescriptorBufferInfo buffer_desc[NUM_BUFFERS];
  for (size_t k = 0; k < NUM_BUFFERS; ++k) {
    buffer_desc[k].buffer = vk->buffer[k];
    buffer_desc[k].range = VK_WHOLE_SIZE;
    buffer_desc[k].offset = 0;
  }

  VkWriteDescriptorSet input_write_descriptor_set[2] = {
      {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, NULL,
       vk->descriptor_set[PIPELINE_INIT], 0, 0, 1,
       VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NULL, &buffer_desc[INPUT_BUFFER],
       NULL},
      {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, NULL,
       vk->descriptor_set[PIPELINE_INIT], 1, 0, 1,
       VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NULL, &buffer_desc[STATE_BUFFER],
       NULL}};

  vkUpdateDescriptorSets(vk->device, 2, input_write_descriptor_set, 0, NULL);

  VkWriteDescriptorSet keccak_write_descriptor_set[1] = {
      {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, NULL,
       vk->descriptor_set[PIPELINE_KECCAK], 0, 0, 1,
       VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NULL, &buffer_desc[STATE_BUFFER],
       NULL}};

  vkUpdateDescriptorSets(vk->device, 1, keccak_write_descriptor_set, 0, NULL);

  VkWriteDescriptorSet explode_write_descriptor_set[2] = {
      {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, NULL,
       vk->descriptor_set[PIPELINE_EXPLODE], 0, 0, 1,
       VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NULL, &buffer_desc[STATE_BUFFER],
       NULL},
      {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, NULL,
       vk->descriptor_set[PIPELINE_EXPLODE], 1, 0, 1,
       VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NULL, &buffer_desc[SCRATCHPAD_BUFFER],
       NULL}};

  vkUpdateDescriptorSets(vk->device, 2, explode_write_descriptor_set, 0, NULL);

  VkWriteDescriptorSet memloop_write_descriptor_set[2] = {
      {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, NULL,
       vk->descriptor_set[PIPELINE_MEMLOOP], 0, 0, 1,
       VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NULL, &buffer_desc[STATE_BUFFER],
       NULL},
      {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, NULL,
       vk->descriptor_set[PIPELINE_MEMLOOP], 1, 0, 1,
       VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NULL, &buffer_desc[SCRATCHPAD_BUFFER],
       NULL}};

  vkUpdateDescriptorSets(vk->device, 2, memloop_write_descriptor_set, 0, NULL);

  // record commands
  VkCommandBufferBeginInfo command_buffer_begin_info = {
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, 0,
      VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, 0};

  vk_res = vkBeginCommandBuffer(vk->cmd_buffer, &command_buffer_begin_info);

  if (vk_res != VK_SUCCESS) {
    log_error("Error when calling vkBeginCommandBuffer");
    return false;
  }

  vkCmdBindPipeline(vk->cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                    vk->pipeline[PIPELINE_INIT]);

  vkCmdBindDescriptorSets(vk->cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                          vk->pipeline_layout[PIPELINE_INIT], 0, 1,
                          &vk->descriptor_set[PIPELINE_INIT], 0, 0);

  vkCmdDispatch(vk->cmd_buffer, parallelism, 1, 1);

  VkBufferMemoryBarrier state_buffer_barrier = {
      VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
      NULL,
      VK_ACCESS_SHADER_WRITE_BIT,
      VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
      VK_QUEUE_FAMILY_IGNORED,
      VK_QUEUE_FAMILY_IGNORED,
      vk->buffer[STATE_BUFFER],
      0,
      VK_WHOLE_SIZE};

  VkBufferMemoryBarrier scratchpad_buffer_barrier = {
      VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
      NULL,
      VK_ACCESS_SHADER_WRITE_BIT,
      VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
      VK_QUEUE_FAMILY_IGNORED,
      VK_QUEUE_FAMILY_IGNORED,
      vk->buffer[SCRATCHPAD_BUFFER],
      0,
      VK_WHOLE_SIZE};

  vkCmdPipelineBarrier(vk->cmd_buffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                       VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, NULL, 1,
                       &state_buffer_barrier, 0, NULL);

  vkCmdBindPipeline(vk->cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                    vk->pipeline[PIPELINE_KECCAK]);

  vkCmdBindDescriptorSets(vk->cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                          vk->pipeline_layout[PIPELINE_KECCAK], 0, 1,
                          &vk->descriptor_set[PIPELINE_KECCAK], 0, 0);

  vkCmdDispatch(vk->cmd_buffer, parallelism, 1, 1);

  vkCmdPipelineBarrier(vk->cmd_buffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                       VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, NULL, 1,
                       &state_buffer_barrier, 0, NULL);

  vkCmdBindPipeline(vk->cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                    vk->pipeline[PIPELINE_EXPLODE]);

  vkCmdBindDescriptorSets(vk->cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                          vk->pipeline_layout[PIPELINE_EXPLODE], 0, 1,
                          &vk->descriptor_set[PIPELINE_EXPLODE], 0, 0);

  vkCmdDispatch(vk->cmd_buffer, parallelism, 1, 1);

  vkCmdPipelineBarrier(vk->cmd_buffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                       VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, NULL, 1,
                       &scratchpad_buffer_barrier, 0, NULL);

  vkCmdBindPipeline(vk->cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                    vk->pipeline[PIPELINE_MEMLOOP]);

  vkCmdBindDescriptorSets(vk->cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                          vk->pipeline_layout[PIPELINE_MEMLOOP], 0, 1,
                          &vk->descriptor_set[PIPELINE_MEMLOOP], 0, 0);

  vkCmdDispatch(vk->cmd_buffer, parallelism, 1, 1);

  vkCmdPipelineBarrier(vk->cmd_buffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                       VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, NULL, 1,
                       &scratchpad_buffer_barrier, 0, NULL);

  vk_res = vkEndCommandBuffer(vk->cmd_buffer);

  if (vk_res != VK_SUCCESS) {
    log_error("Error when calling vkEndCommandBuffer");
    return false;
  }

  return true;
}
