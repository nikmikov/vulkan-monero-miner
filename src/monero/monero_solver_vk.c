#include "monero/monero_solver.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "logging.h"
#include "resources.h"
#include <vulkan/vulkan.h>

#define NUM_COMPUTE_PIPELINES 2
#define NUM_BUFFERS 3

struct monero_solver_vk_context {
  uint32_t device_idx;
  VkInstance instance;
  VkPhysicalDevice physical_device;
  VkPhysicalDeviceProperties physical_device_properties;
  VkDevice device;
  VkQueue queue;
  VkCommandPool cmd_pool;

  // memory and buffers
  VkDeviceMemory memory;
  VkBuffer buffer[NUM_BUFFERS];

  // shaders
  VkShaderModule compute_shader[NUM_COMPUTE_PIPELINES];
  VkDescriptorSetLayout descriptor_set_layout[NUM_COMPUTE_PIPELINES];
  VkPipelineLayout pipeline_layout[NUM_COMPUTE_PIPELINES];
  VkPipeline pipeline[NUM_COMPUTE_PIPELINES];
};

struct monero_solver_vk {
  struct monero_solver solver;

  /** Vulkan Context */
  struct monero_solver_vk_context *vk;
};

struct monero_solver_vk_context *
monero_solver_vk_context_init(uint32_t device_idx);

void monero_solver_vk_context_release(struct monero_solver_vk_context *ctx);

bool monero_solver_vk_context_prepare_pipelines(
    struct monero_solver_vk_context *cl);

bool monero_solver_vk_context_prepare_buffers(
    struct monero_solver_vk_context *vk, size_t nthreads);

struct monero_solver *
monero_solver_new_vk(const struct monero_config_solver_cl *cfg)
{
  assert(cfg != NULL);
  assert(cfg->device_id >= 0);

  struct monero_solver_vk_context *vk_ctx =
      monero_solver_vk_context_init((uint32_t)cfg->device_id);
  if (vk_ctx == NULL) {
    log_error("Error when creating Vulkan Context");
    return NULL;
  }

  // init memory buffers
  if (!monero_solver_vk_context_prepare_buffers(vk_ctx, cfg->intensity)) {
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

  struct monero_solver_vk *solver_vk =
      calloc(1, sizeof(struct monero_solver_vk));

  solver_vk->vk = vk_ctx;

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

  return ctx;

ERROR:
  monero_solver_vk_context_release(ctx);
  return NULL;
}

void monero_solver_vk_context_release(struct monero_solver_vk_context *ctx)
{
  assert(ctx != NULL);

  for (size_t i = 0; i < NUM_COMPUTE_PIPELINES; ++i) {
    if (ctx->descriptor_set_layout[i] != NULL) {
      vkDestroyDescriptorSetLayout(ctx->device, ctx->descriptor_set_layout[i],
                                   NULL);
    }
    if (ctx->pipeline_layout[i] != NULL) {
      vkDestroyPipelineLayout(ctx->device, ctx->pipeline_layout[i], NULL);
    }
    if (ctx->pipeline[i] != NULL) {
      vkDestroyPipeline(ctx->device, ctx->pipeline[i], NULL);
    }
  }
  for (size_t i = 0; i < NUM_BUFFERS; ++i) {
    if (ctx->buffer[i] != NULL)
      vkDestroyBuffer(ctx->device, ctx->buffer[i], NULL);
  }
  if (ctx->memory != NULL)
    vkFreeMemory(ctx->device, ctx->memory, NULL);
  if (ctx->cmd_pool != NULL)
    vkDestroyCommandPool(ctx->device, ctx->cmd_pool, NULL);
  if (ctx->device != NULL)
    vkDestroyDevice(ctx->device, NULL);
  if (ctx->instance != NULL)
    vkDestroyInstance(ctx->instance, NULL);
  free(ctx);
}

bool monero_solver_vk_context_prepare_buffers(
    struct monero_solver_vk_context *vk, size_t nthreads)
{
  VkResult vk_res;

  // calculate required memory size
  const size_t buffer_size[NUM_BUFFERS] = {
      sizeof(uint32_t) + 88,               // input buffer
      200 * nthreads,                      // state buffer
      MONERO_CRYPTONIGHT_MEMORY * nthreads // scratchpad buffer
  };
  // create buffers
  // input buffer
  const VkBufferCreateInfo buffer_create_info[3] = {
      {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, 0, 0, buffer_size[0],
       VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, 1, NULL},
      {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, 0, 0, buffer_size[1],
       VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, 1, NULL},
      {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, 0, 0, buffer_size[2],
       VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, 1, NULL}};

  VkMemoryRequirements buffer_memory_requirements[NUM_BUFFERS];

  size_t required_memory_size = 0;
  for (size_t k = 0; k < NUM_BUFFERS; ++k) {
    vk_res = vkCreateBuffer(vk->device, &buffer_create_info[k], NULL,
                            &vk->buffer[k]);
    if (vk_res != VK_SUCCESS) {
      log_error("Error when calling vkCreateBuffer: buffer #%lu of size(%lu)",
                k, buffer_size[k]);
      return false;
    }

    // determine buffer memory requirements
    vkGetBufferMemoryRequirements(vk->device, vk->buffer[k],
                                  &buffer_memory_requirements[k]);

    required_memory_size += buffer_memory_requirements[k].size;
  }

  VkPhysicalDeviceMemoryProperties properties;
  vkGetPhysicalDeviceMemoryProperties(vk->physical_device, &properties);

  uint32_t memory_type_index = VK_MAX_MEMORY_TYPES;

  // find appropriate memory
  log_debug("Looking for device memory at least size: %lu",
            required_memory_size);
  for (uint32_t k = 0; k < properties.memoryTypeCount; ++k) {
    VkDeviceSize heap_size =
        properties.memoryHeaps[properties.memoryTypes[k].heapIndex].size;

    bool is_device_local = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT &
                           properties.memoryTypes[k].propertyFlags;

    if (is_device_local && heap_size >= required_memory_size) {
      log_debug("Found suitable memory @index: %u", k);
      memory_type_index = k;
      break;
    }
  }

  if (memory_type_index == VK_MAX_MEMORY_TYPES) {
    log_error("Could not find suitable device memory. At least size: %lu "
              "is required",
              required_memory_size);
    return false;
  }

  // allocate device memory as single block for all buffers
  const VkMemoryAllocateInfo memory_allocate_info = {
      VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, 0, required_memory_size,
      memory_type_index};

  vk_res = vkAllocateMemory(vk->device, &memory_allocate_info, 0, &vk->memory);
  if (vk_res != VK_SUCCESS) {
    log_error("Error when calling vkAllocateMemory: size of(%lu)",
              required_memory_size);
    return false;
  }
  log_debug("Successfully allocated %lu bytes of memory @memory type %u",
            required_memory_size, memory_type_index);

  // bind memory to buffers
  size_t offset = 0;
  for (size_t k = 0; k < NUM_BUFFERS; ++k) {
    size_t buf_sz = buffer_memory_requirements[k].size;
    log_info("Trying to bind buffer #%lu of size(%lu) to memory @offset(%lu)",
             k, buf_sz, offset);
    vk_res = vkBindBufferMemory(vk->device, vk->buffer[k], vk->memory, offset);
    if (vk_res != VK_SUCCESS) {
      log_error("Error when calling vkBindBufferMemory: buffer #%lu", k);
      return false;
    }
    offset += buf_sz;
  }

  return true;
}

bool monero_solver_vk_context_prepare_pipelines(
    struct monero_solver_vk_context *vk)
{
  VkResult vk_res;
  VkShaderModuleCreateInfo cn_init_create_info = {
      VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, 0, 0,
      (spv_cn_init_end - spv_cn_init), (uint32_t *)spv_cn_init};

  vk_res = vkCreateShaderModule(vk->device, &cn_init_create_info, 0,
                                &vk->compute_shader[0]);
  if (vk_res != VK_SUCCESS) {
    log_error("Error when calling vkCreateShaderModule for cn_init shader");
    return false;
  }
  log_debug("`cn_init` shader initialized");

  VkShaderModuleCreateInfo cn_keccak_create_info = {
      VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, 0, 0,
      (spv_cn_keccak_end - spv_cn_keccak), (uint32_t *)spv_cn_keccak};

  vk_res = vkCreateShaderModule(vk->device, &cn_keccak_create_info, 0,
                                &vk->compute_shader[1]);
  if (vk_res != VK_SUCCESS) {
    log_error("Error when calling vkCreateShaderModule for `cn_keccak` shader");
    return false;
  }
  log_debug("`cn_keccak` shader initialized");

  // descriptors set
  VkDescriptorSetLayoutBinding input_descriptor_set_layout_bindings[2] = {
      {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, 0},
      {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT,
       0}};

  VkDescriptorSetLayoutBinding keccak_descriptor_set_layout_bindings[1] = {
      {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT,
       0}};

  VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info[2] = {
      {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, 0, 0, 2,
       input_descriptor_set_layout_bindings},
      {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, 0, 0, 1,
       keccak_descriptor_set_layout_bindings}};

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
