#include "r_vulkan.h"

#include "base/base_core.h"

#include "render/r_buffer.h"
#include "render/r_pipeline.h"

// --------------------------------------------------
// Buffer
func VkBufferUsageFlags
_VkFromBufferUsageFlags(BufferUsageFlags flags)
{
  VkBufferUsageFlags result = 0;

  if((flags & BUFFER_USAGE_FLAG_VERTEX) == BUFFER_USAGE_FLAG_VERTEX)
  {
    result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  }
  if((flags & BUFFER_USAGE_FLAG_INDEX) == BUFFER_USAGE_FLAG_INDEX)
  {
    result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  }
  if((flags & BUFFER_USAGE_FLAG_UNIFORM) == BUFFER_USAGE_FLAG_UNIFORM)
  {
    result |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  }
  if((flags & BUFFER_USAGE_FLAG_TRANSFER_SRC) == BUFFER_USAGE_FLAG_TRANSFER_SRC)
  {
    result |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  }

  return result;
}

func VkMemoryPropertyFlags
_VkFromBufferPropertyFlags(BufferPropertyFlags flags)
{
  VkMemoryPropertyFlags result = 0;

  if ((flags & BUFFER_PROPERTY_HOST_COHERENT) == BUFFER_PROPERTY_HOST_COHERENT)
  {
    result |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  }
  if ((flags & BUFFER_PROPERTY_HOST_VISIBLE) == BUFFER_PROPERTY_HOST_VISIBLE)
  {
    result |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
  }

  return result;
}

func R_VK_Buffer
R_VK_BufferCreate(U64 capacity, BufferUsageFlags usage_flags, BufferPropertyFlags property_flags)
{
  // @NOTE This is to create Vulkan Buffer and Memory
  R_VK_Buffer result = {};
  result.capacity = capacity;
  
  VkBufferCreateInfo buffer_info = {
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .size = capacity,
    .usage = _VkFromBufferUsageFlags(usage_flags),
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE
  };

  VK_CHECK(vkCreateBuffer(_r_vk_state.device.logical, &buffer_info, 0, &result.handle));

  VkMemoryRequirements memory_requirements;
  vkGetBufferMemoryRequirements(_r_vk_state.device.logical, result.handle, &memory_requirements);
  
  VkPhysicalDeviceMemoryProperties mem_properties = {};
  vkGetPhysicalDeviceMemoryProperties(_r_vk_state.device.physical, &mem_properties);

  VkMemoryPropertyFlags flags = _VkFromBufferPropertyFlags(property_flags);
  U32 memory_type_index = 0;
  for (U32 type_index = 0; type_index < mem_properties.memoryTypeCount; type_index += 1)
  {
    if (memory_requirements.memoryTypeBits & (1 << type_index) && ((mem_properties.memoryTypes[type_index].propertyFlags & flags) == flags))
    {
      memory_type_index = type_index;
      break;
    }
  }

  VkMemoryAllocateInfo allocation_info = {
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .allocationSize = memory_requirements.size,
    .memoryTypeIndex = memory_type_index
  };
  VK_CHECK(vkAllocateMemory(_r_vk_state.device.logical, &allocation_info , 0, &result.memory));

  VK_CHECK(vkBindBufferMemory(_r_vk_state.device.logical, result.handle, result.memory, 0));

  return result;
}

func void R_VK_BufferDestroy(R_VK_Buffer* buffer)
{
  vkFreeMemory(_r_vk_state.device.logical, buffer->memory, 0);
  vkDestroyBuffer(_r_vk_state.device.logical, buffer->handle, 0);

  *buffer = {};
}

// --------------------------------------------------
// Device
func void
R_VK_DeviceCreate()
{
  U32 device_count = 0;
  VK_CHECK(vkEnumeratePhysicalDevices(_r_vk_state.instance, &device_count, 0));
  
  Arena* tmp_arena = AllocateArena(Kilobytes(16));
  {
    VkPhysicalDevice* devices = (VkPhysicalDevice*)PushArena(tmp_arena, device_count * sizeof(VkPhysicalDevice));
    VK_CHECK(vkEnumeratePhysicalDevices(_r_vk_state.instance, &device_count, devices));

    for (I32 i = 0; i < device_count; i += 1)
    {
      VkPhysicalDevice* device = devices + i;
      
      VkPhysicalDeviceProperties properties;
      vkGetPhysicalDeviceProperties(*device, &properties);

      if (properties.apiVersion < VK_API_VERSION_1_3)
      {
        continue;
      }

      U32 queue_family_count = 0;
      vkGetPhysicalDeviceQueueFamilyProperties(*device, &queue_family_count, 0);
      VkQueueFamilyProperties* queue_properties = (VkQueueFamilyProperties*)PushArena(tmp_arena, queue_family_count * sizeof(VkQueueFamilyProperties));
      vkGetPhysicalDeviceQueueFamilyProperties(*device, &queue_family_count, queue_properties);

      for (I32 i = 0; i < queue_family_count; i += 1)
      {
        VkQueueFamilyProperties* properties = queue_properties + i;

        if (properties->queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
          _r_vk_state.device.graphics_queue_index = i;
          break;
        }
      }

      const char* required_extensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
      };

      VkPhysicalDeviceVulkan13Features vulkan13_features = {};
      vulkan13_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
      vulkan13_features.dynamicRendering = VK_TRUE;

      F32 queue_priority = 1.0f;

      VkDeviceQueueCreateInfo queue_info = {};
      queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queue_info.queueFamilyIndex = _r_vk_state.device.graphics_queue_index;
      queue_info.queueCount = 1;
      queue_info.pQueuePriorities = &queue_priority;

      VkDeviceCreateInfo device_info = {};
      device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
      device_info.queueCreateInfoCount = 1;
      device_info.pQueueCreateInfos = &queue_info;
      device_info.enabledExtensionCount = CountArrayElements(required_extensions);
      device_info.ppEnabledExtensionNames = required_extensions;
      device_info.pEnabledFeatures = 0;
      device_info.pNext = &vulkan13_features;

      _r_vk_state.device.physical = *device;
      VK_CHECK(vkCreateDevice(*device, &device_info, 0, &_r_vk_state.device.logical))

      vkGetDeviceQueue(_r_vk_state.device.logical, _r_vk_state.device.graphics_queue_index, 0, &_r_vk_state.device.graphics_queue);

      LOG_INFO("%s\n", properties.deviceName);
    }
  }
  FreeArena(tmp_arena);
}

func void
R_VK_DeviceDestroy()
{
  vkDestroyDevice(_r_vk_state.device.logical, 0);
  _r_vk_state.device = {};
}

// --------------------------------------------------
// Surface/Swapchain

func FrameResources
R_VK_FrameResourcesCreate()
{
  FrameResources resources = {};

  VkFenceCreateInfo fence_info = {
    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    .flags = VK_FENCE_CREATE_SIGNALED_BIT
  };
  VK_CHECK(vkCreateFence(_r_vk_state.device.logical, &fence_info, 0, &resources.submit_fence));

  VkCommandPoolCreateInfo cmd_pool_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
    .queueFamilyIndex = _r_vk_state.device.graphics_queue_index
  };
  VK_CHECK(vkCreateCommandPool(_r_vk_state.device.logical, &cmd_pool_info, 0, &resources.cmd_pool));

  VkCommandBufferAllocateInfo cmd_buffer = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool = resources.cmd_pool,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = 1
  };
  VK_CHECK(vkAllocateCommandBuffers(_r_vk_state.device.logical, &cmd_buffer, &resources.cmd_buffer));

  VkSemaphoreCreateInfo semaphore_info = {
    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
  };
  VK_CHECK(vkCreateSemaphore(_r_vk_state.device.logical, &semaphore_info, 0, &resources.acquire_semaphore));
  VK_CHECK(vkCreateSemaphore(_r_vk_state.device.logical, &semaphore_info, 0, &resources.release_semaphore));

  return resources;
}

func void
R_VK_FrameResourcesDestroy(FrameResources* resources)
{
  vkDestroySemaphore(_r_vk_state.device.logical, resources->release_semaphore, 0);
  vkDestroySemaphore(_r_vk_state.device.logical, resources->acquire_semaphore, 0);
  vkDestroyCommandPool(_r_vk_state.device.logical, resources->cmd_pool, 0);
  vkDestroyFence(_r_vk_state.device.logical, resources->submit_fence, 0);

  *resources = {};
}

func void
R_VK_SurfaceCreate(OS_Window* window)
{
}

func void
R_VK_SurfaceDestroy()
{
  vkDestroySurfaceKHR(_r_vk_state.instance, _r_vk_state.swapchain.surface, 0);
}

func void
R_VK_SwapchainCreate(OS_Window* window)
{
#if IGNIS_PLATFORM_LINUX
  VkWaylandSurfaceCreateInfoKHR surface_info = {
    .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
    .display = window->handle->display,
    .surface = window->handle->surface
  };

  VK_CHECK(vkCreateWaylandSurfaceKHR(state->instance, &surface_info, 0, &state->surface.handle));
#endif // IGNIS_PLATFORM_LINUX

#if IGNIS_PLATFORM_WIN32
  VkWin32SurfaceCreateInfoKHR surface_info = {};
  surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  surface_info.hinstance = window->handle->instance;
  surface_info.hwnd = window->handle->handle;

  VK_CHECK(vkCreateWin32SurfaceKHR(state->instance, &surface_info, 0, &state->surface.handle));
#endif // IGNIS_PLATFORM_WIN32

  R_VK_Swapchain swapchain = {};
  
  Arena* tmp_arena = AllocateArena(Kilobytes(64));
  {
    U32 format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(_r_vk_state.device.physical, _r_vk_state.swapchain.surface, &format_count, 0);
    VkSurfaceFormatKHR* formats = (VkSurfaceFormatKHR*)PushArena(tmp_arena, format_count * sizeof(VkSurfaceFormatKHR));
    vkGetPhysicalDeviceSurfaceFormatsKHR(_r_vk_state.device.physical, _r_vk_state.swapchain.surface, &format_count, formats);

    for (U32 i = 0; i < format_count; i += 1)
    {
      if (formats[i].format == VK_FORMAT_B8G8R8A8_UNORM && formats[i].colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
        swapchain.surface_format = formats[i];
      }
    }
  }
  FreeArena(tmp_arena);
  
  VkSurfaceCapabilitiesKHR capabilities;
  VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_r_vk_state.device.physical,
                                                     _r_vk_state.swapchain.surface,
                                                     &capabilities));
  if (capabilities.currentExtent.width == U32_MAX)
  {
    swapchain.size = window->size;
    LOG_WARNING("SWAP ===== %d / %d\n", window->size.x, window->size.y);
  }
  else
  {
    swapchain.size.width = capabilities.currentExtent.width;
    swapchain.size.height = capabilities.currentExtent.height;
  }

  VkPresentModeKHR present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;

  U32 image_count = capabilities.minImageCount + 1;
  if ((capabilities.maxImageCount > 0) && (image_count > capabilities.maxImageCount))
  {
    image_count = capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR swapchain_info = {
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .surface = _r_vk_state.surface.handle,
    .minImageCount = image_count,
    .imageFormat = swapchain.surface_format.format,
    .imageColorSpace = swapchain.surface_format.colorSpace,
    .imageExtent = { .width = swapchain.size.width, .height = swapchain.size.height },
    .imageArrayLayers = 1,
    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .preTransform = capabilities.currentTransform,
    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .presentMode = present_mode,
    .clipped = true,
    .oldSwapchain = 0
  };

  VK_CHECK(vkCreateSwapchainKHR(_r_vk_state.device.logical, &swapchain_info, 0, &swapchain.handle));

  VK_CHECK(vkGetSwapchainImagesKHR(_r_vk_state.device.logical, swapchain.handle, &swapchain.image_count, 0));
  swapchain.image_arena = AllocateArena(Megabytes(8));
  swapchain.images = (VkImage*)PushArena(swapchain.image_arena, swapchain.image_count * sizeof(VkImage));
  VK_CHECK(vkGetSwapchainImagesKHR(_r_vk_state.device.logical, swapchain.handle, &swapchain.image_count, swapchain.images));

  swapchain.image_views = (VkImageView*)PushArena(swapchain.image_arena, swapchain.image_count * sizeof(VkImageView));
  for (U32 i = 0; i < swapchain.image_count; i += 1)
  {
    VkImageViewCreateInfo view_info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = swapchain.images[i],
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = swapchain.surface_format.format,
      .subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1
      }
    };

    VK_CHECK(vkCreateImageView(_r_vk_state.device.logical, &view_info, 0, &swapchain.image_views[i]));
    
    swapchain.frame_resources[i] = R_VK_FrameResourcesCreate();
  }

  {
    VkImageCreateInfo image_info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = VK_FORMAT_D32_SFLOAT,
      .extent = { .width = swapchain.size.width, .height = swapchain.size.height, .depth = 1 },
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = 0,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    VK_CHECK(vkCreateImage(_r_vk_state.device.logical, &image_info, 0, &swapchain.depth_image));

    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(_r_vk_state.device.logical, swapchain.depth_image, &memory_requirements);

    VkMemoryAllocateInfo allocate_info = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = memory_requirements.size,
      .memoryTypeIndex = R_VK_FindMemoryTypeIndex(memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
    };
    VK_CHECK(vkAllocateMemory(_r_vk_state.device.logical, &allocate_info, 0, &swapchain.depth_image_memory));
    
    vkBindImageMemory(_r_vk_state.device.logical, swapchain.depth_image, swapchain.depth_image_memory, 0);

    VkImageViewCreateInfo view_info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = swapchain.depth_image,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = VK_FORMAT_D32_SFLOAT,
      .subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1
      }
    };

    VK_CHECK(vkCreateImageView(_r_vk_state.device.logical, &view_info, 0, &swapchain.depth_image_view));
  }
  
  _r_vk_state.swapchain = swapchain;
}

func void
R_VK_SwapchainDestroy()
{
  vkDeviceWaitIdle(_r_vk_state.device.logical);
  
  for (I32 i = 0; i < _r_vk_state.swapchain.image_count; i += 1)
  {
    R_VK_FrameResourcesDestroy(&_r_vk_state.swapchain.frame_resources[i]);
    vkDestroyImageView(_r_vk_state.device.logical, _r_vk_state.swapchain.image_views[i], 0);
  }

  vkDestroySwapchainKHR(_r_vk_state.device.logical, _r_vk_state.swapchain.handle, 0);

  FreeArena(_r_vk_state.swapchain.image_arena);
}

func void
R_VK_SwapchainRecreate(OS_Window* window)
{
  LOG_INFO("Recreate Swapchain\n");
  R_VK_SwapchainDestroy();
  R_VK_SwapchainCreate(window);
}

func B32
R_VK_SwapchainAcquireNextImage(U32 *image_index)
{
  VkResult acquire_result = vkAcquireNextImageKHR(
    _r_vk_state.device.logical, _r_vk_state.swapchain.handle, U64_MAX,
    _r_vk_state.swapchain.frame_resources[_r_vk_state.current_frame].acquire_semaphore, 0,
    image_index
  );
  
  if (acquire_result != VK_SUCCESS) {
    return false;
  }

  return true;
}

// --------------------------------------------------
// Pipeline

func void
R_VK_GraphicsShaderCreate(R_Pipeline* pipeline)
{
  PipelineID id = _r_vk_state.graphics_shaders_count;
  pipeline->backend_handle = id;

  // Global Set
  {
    VkDescriptorSetLayoutBinding bindings[R_MAX_BINDINGS] = {};
    U32 binding_count = 0;
    for (U32 i = 0; i < pipeline->global_bindings_count; i += 1)
    {
      R_BindingInfo* binding_info = &pipeline->global_bindings[i];

      bindings[i] = {
        .binding = i,
        .descriptorType = R_VK_GetVkDescriptorType(binding_info->type),
        .descriptorCount = 1,
        .stageFlags = R_VK_GetVkShaderStage(binding_info->shader_type)
      };

      binding_count += 1;
    }

    VkDescriptorSetLayoutCreateInfo layout = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = binding_count,
      .pBindings = bindings
    };
    VK_CHECK(vkCreateDescriptorSetLayout(_r_vk_state.device.logical, &layout, 0, &_r_vk_state.graphics_shaders[id].global_set_layout));
  }

  // Instance Set
  {
    VkDescriptorSetLayoutBinding bindings[R_MAX_BINDINGS] = {};
    U32 binding_count = 0;
    for (U32 i = 0; i < pipeline->instance_bindings_count; i += 1)
    {
      R_BindingInfo* binding_info = &pipeline->instance_bindings[i];

      bindings[i] = {
        .binding = i,
        .descriptorType = R_VK_GetVkDescriptorType(binding_info->type),
        .descriptorCount = 1,
        .stageFlags = R_VK_GetVkShaderStage(binding_info->shader_type)
      };

      binding_count += 1;
    }

    VkDescriptorSetLayoutCreateInfo layout = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = binding_count,
      .pBindings = bindings
    };
    VK_CHECK(vkCreateDescriptorSetLayout(_r_vk_state.device.logical, &layout, 0, &_r_vk_state.graphics_shaders[id].instance_set_layout));
  }

  VkDescriptorSetLayout set_layouts[] = {
    _r_vk_state.graphics_shaders[id].global_set_layout,
    _r_vk_state.graphics_shaders[id].instance_set_layout
  };
  
  VkPipelineLayoutCreateInfo layout_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount = CountArrayElements(set_layouts),
    .pSetLayouts = set_layouts,
  };
  VK_CHECK(vkCreatePipelineLayout(_r_vk_state.device.logical, &layout_info, 0, &_r_vk_state.graphics_shaders[id].pipeline_layout));

  U32 stride = 0;
  VkVertexInputAttributeDescription attribute_descriptions[MAX_ATTRIBUTES];
  for (U32 i = 0; i < pipeline->attributes_count; i += 1)
  {
    attribute_descriptions[i] = {
      .location = i,
      .binding = 0,
      .format = R_VK_GetVkFormatAttribute(pipeline->attributes[i]),
      .offset = stride
    };

    stride += R_H_OffsetFromAttributeFormat(pipeline->attributes[i]);
  }

  VkVertexInputBindingDescription binding_description = {
    .binding = 0,
    .stride = stride,
    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
  };
  
  VkPipelineVertexInputStateCreateInfo vertex_input = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .vertexBindingDescriptionCount = 1,
    .pVertexBindingDescriptions = &binding_description,
    .vertexAttributeDescriptionCount = pipeline->attributes_count,
    .pVertexAttributeDescriptions = attribute_descriptions
  };

  VkPipelineInputAssemblyStateCreateInfo input_assembly = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .primitiveRestartEnable = false
  };

  VkPipelineRasterizationStateCreateInfo rasterization = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .depthClampEnable = false,
    .rasterizerDiscardEnable = false,
    .polygonMode = VK_POLYGON_MODE_FILL,
    .cullMode = (VkCullModeFlags)(pipeline->is_back_culing_enabled * VK_CULL_MODE_BACK_BIT),
    .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    .depthBiasEnable = false,
    .lineWidth = 1.0f
  };

  VkDynamicState dynamic_states[] = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
  };

  VkPipelineColorBlendAttachmentState blend_attachment = {
    .blendEnable = VK_TRUE,
    .srcColorBlendFactor=VK_BLEND_FACTOR_SRC_ALPHA,
    .dstColorBlendFactor=VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    .colorBlendOp=VK_BLEND_OP_ADD,
    .srcAlphaBlendFactor=VK_BLEND_FACTOR_ONE,
    .dstAlphaBlendFactor=VK_BLEND_FACTOR_ZERO,
    .alphaBlendOp=VK_BLEND_OP_ADD,
    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
  };

  VkPipelineColorBlendStateCreateInfo blend = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .attachmentCount = 1,
    .pAttachments = &blend_attachment
  };

  VkPipelineViewportStateCreateInfo viewport = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .viewportCount = 1,
    .scissorCount = 1
  };

  VkPipelineDepthStencilStateCreateInfo depth_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
    .depthTestEnable = (VkBool32)pipeline->is_depth_test_enabled,
    .depthWriteEnable = (VkBool32)pipeline->is_depth_test_enabled,
    .depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL
  };

  VkPipelineMultisampleStateCreateInfo multisample = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
  };

  VkPipelineDynamicStateCreateInfo dynamic = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    .dynamicStateCount = CountArrayElements(dynamic_states),
    .pDynamicStates = dynamic_states
  };

  VkShaderModule vertex_module;
  {
    VkShaderModuleCreateInfo module_info = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = pipeline->shaders[R_SHADER_TYPE_VERTEX].code_size,
      .pCode = (U32*)pipeline->shaders[R_SHADER_TYPE_VERTEX].code
    };
    VK_CHECK(vkCreateShaderModule(_r_vk_state.device.logical, &module_info, 0, &vertex_module));
  }
  VkPipelineShaderStageCreateInfo vertex_shader = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .stage = VK_SHADER_STAGE_VERTEX_BIT,
    .module = vertex_module,
    .pName = pipeline->shaders[R_SHADER_TYPE_VERTEX].entry_point
  };
  
  VkShaderModule fragment_module;
  {
    VkShaderModuleCreateInfo module_info = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = pipeline->shaders[R_SHADER_TYPE_FRAGMENT].code_size,
      .pCode = (U32*)pipeline->shaders[R_SHADER_TYPE_FRAGMENT].code
    };
    VK_CHECK(vkCreateShaderModule(_r_vk_state.device.logical, &module_info, 0, &fragment_module));
  }
  VkPipelineShaderStageCreateInfo fragment_shader = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
    .module = fragment_module,
    .pName = pipeline->shaders[R_SHADER_TYPE_FRAGMENT].entry_point
  };

  VkPipelineShaderStageCreateInfo shaders[] = {
    vertex_shader,
    fragment_shader
  };

  VkPipelineRenderingCreateInfo rendering_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
    .colorAttachmentCount = 1,
    .pColorAttachmentFormats = &_r_vk_state.swapchain.surface_format.format,
    .depthAttachmentFormat = VK_FORMAT_D32_SFLOAT
  };

  VkGraphicsPipelineCreateInfo pipeline_info = {
    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .pNext = &rendering_info,
    .stageCount = CountArrayElements(shaders),
    .pStages = shaders,
    .pVertexInputState = &vertex_input,
    .pInputAssemblyState = &input_assembly,
    .pViewportState = &viewport,
    .pRasterizationState = &rasterization,
    .pMultisampleState = &multisample,
    .pDepthStencilState = &depth_state,
    .pColorBlendState = &blend,
    .pDynamicState = &dynamic,
    .layout = _r_vk_state.graphics_shaders[id].pipeline_layout,
    .renderPass = 0,
    .subpass = 0,
  };
  VK_CHECK(vkCreateGraphicsPipelines(_r_vk_state.device.logical,
                                     0, 1, &pipeline_info, 0,
                                     &_r_vk_state.graphics_shaders[id].pipeline));


  _r_vk_state.graphics_shaders_count += 1;
}

func void R_VK_GraphicsShaderDestroy(R_VK_State* state)
{
}

// --------------------------------------------------
// Global State
func B32
R_VK_Init(OS_Window* window)
{
  _r_vk_state.arena = AllocateArena(Megabytes(64));

  VkApplicationInfo app_info = {};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "VulkanRenderingFramework";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "RenderingEngine";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_API_VERSION_1_3;

  const char* extension_names[] = {
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
    "VK_KHR_surface",
  #if IGNIS_PLATFORM_LINUX
    "VK_KHR_wayland_surface",
  #endif // IGNIS_PLATFORM_LINUX
  #if IGNIS_PLATFORM_WIN32
    "VK_KHR_win32_surface",
  #endif // IGNIS_PLATFORM_WIN32
  };

#if IGNIS_DEBUG
  const char* validation_layers[] = {
    "VK_LAYER_KHRONOS_validation",
  };
#else
  const char* validation_layers[] = {};
#endif // IGNIS_DEBUG

  VkInstanceCreateInfo instance_info = {};
  instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_info.pApplicationInfo = &app_info;
  instance_info.enabledLayerCount = CountArrayElements(validation_layers);
  instance_info.ppEnabledLayerNames = validation_layers;
  instance_info.enabledExtensionCount = CountArrayElements(extension_names);
  instance_info.ppEnabledExtensionNames = extension_names;

#if IGNIS_DEBUG
  VkDebugUtilsMessengerCreateInfoEXT messenger_info;
  R_VK_PopulateDebugMessengerCreateInfo(messenger_info);
  instance_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&messenger_info;
#endif // IGNIS_DEBUG

  VK_CHECK(vkCreateInstance(&instance_info, 0, &r_vk_state.instance));

#if IGNIS_DEBUG
  VK_CHECK(R_VK_CreateDebugMessenger(r_vk_state.instance, &r_vk_state.debug_messenger));
#endif // IGNIS_DEBUG

  R_VK_DeviceCreate();
  R_VK_SwapchainCreate(window);

  _r_vk_state.geometry_buffer = R_VK_BufferCreate(Megabytes(256), BUFFER_USAGE_FLAG_UNIFORM | BUFFER_USAGE_FLAG_VERTEX | BUFFER_USAGE_FLAG_VERTEX);
  _r_vk_state.staging_buffer = R_VK_BufferCreate(Megabytes(64), BUFFER_USAGE_FLAG_TRANSFER_SRC, BUFFER_PROPERTY_HOST_COHERENT | BUFFER_PROPERTY_HOST_VISIBLE)
}

func B32
R_VK_Shutdown()
{
  
}
