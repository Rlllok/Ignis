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
R_VK_CreateBuffer(U64 capacity, BufferUsageFlags usage_flags, BufferPropertyFlags property_flags)
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

func void R_VK_DestroyBuffer(R_VK_Buffer* buffer)
{
  vkFreeMemory(_r_vk_state.device.logical, buffer->memory, 0);
  vkDestroyBuffer(_r_vk_state.device.logical, buffer->handle, 0);

  *buffer = {};
}

// --------------------------------------------------
// Device
func void
R_VK_CreateDevice()
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
R_VK_DestroyDevice()
{
  vkDestroyDevice(_r_vk_state.device.logical, 0);
  _r_vk_state.device = {};
}

// --------------------------------------------------
// Surface/Swapchain
func FrameResources
R_VK_CreateFrameResources()
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
R_VK_DestroyFrameResources(FrameResources* resources)
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
R_VK_DestorySurface()
{
  vkDestroySurfaceKHR(_r_vk_state.instance, _r_vk_state.swapchain.surface, 0);
}

func void
R_VK_CreateSwapchain(OS_Window* window)
{
#if IGNIS_PLATFORM_LINUX
  VkWaylandSurfaceCreateInfoKHR surface_info = {
    .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
    .display = window->handle->display,
    .surface = window->handle->surface
  };

  VK_CHECK(vkCreateWaylandSurfaceKHR(_r_vk_state.instance, &surface_info, 0, &_r_vk_state.swapchain.surface));
#endif // IGNIS_PLATFORM_LINUX

#if IGNIS_PLATFORM_WIN32
  VkWin32SurfaceCreateInfoKHR surface_info = {};
  surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  surface_info.hinstance = window->handle->instance;
  surface_info.hwnd = window->handle->handle;

  VK_CHECK(vkCreateWin32SurfaceKHR(_r_vk_state.instance, &surface_info, 0, &_r_vk_state.swapchain.surface));
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
    .surface = _r_vk_state.swapchain.surface,
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
  swapchain.frame_resources = (FrameResources*)PushArena(swapchain.image_arena, swapchain.image_count * sizeof(FrameResources));
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
    
    swapchain.frame_resources[i] = R_VK_CreateFrameResources();
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
R_VK_DestroySwapchain()
{
  vkDeviceWaitIdle(_r_vk_state.device.logical);
  
  for (I32 i = 0; i < _r_vk_state.swapchain.image_count; i += 1)
  {
    R_VK_DestroyFrameResources(&_r_vk_state.swapchain.frame_resources[i]);
    vkDestroyImageView(_r_vk_state.device.logical, _r_vk_state.swapchain.image_views[i], 0);
  }

  vkDestroySwapchainKHR(_r_vk_state.device.logical, _r_vk_state.swapchain.handle, 0);

  FreeArena(_r_vk_state.swapchain.image_arena);
}

func void
R_VK_RecreateSwapchain(OS_Window* window)
{
  LOG_INFO("Recreate Swapchain\n");
  R_VK_DestroySwapchain();
  R_VK_CreateSwapchain(window);
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
R_VK_CreateGraphicsPipeline(R_Pipeline* pipeline)
{
  PipelineID id = _r_vk_state.graphics_pipelines_count;
  pipeline->backend_handle = id;
	
	// Global Pool
	{
		VkDescriptorPoolSize pool_size = {
			.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = R_VK_MAX_OBJECTS
		};
		
		VkDescriptorPoolCreateInfo pool_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.maxSets = R_VK_MAX_OBJECTS,
			.poolSizeCount = 1,
			.pPoolSizes = &pool_size
		};

		for (I32 i = 0; i < R_VK_FRAMES_IN_FLIGHT; i += 1)
		{
			VK_CHECK(vkCreateDescriptorPool(_r_vk_state.device.logical, &pool_info, 0, &_r_vk_state.graphics_pipelines[id].global_set_pool[i]));
		}
	}

	// Instance Pool
	{
		VkDescriptorPoolSize pool_sizes[2] = {};
		pool_sizes[0] = {
			.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = R_VK_MAX_OBJECTS
		};
		
		pool_sizes[1] = {
			.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = R_VK_MAX_OBJECTS
		};
		
		VkDescriptorPoolCreateInfo pool_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.maxSets = R_VK_MAX_OBJECTS,
			.poolSizeCount = CountArrayElements(pool_sizes),
			.pPoolSizes = pool_sizes
		};

		for (I32 i = 0; i < R_VK_FRAMES_IN_FLIGHT; i += 1)
		{
			VK_CHECK(vkCreateDescriptorPool(_r_vk_state.device.logical, &pool_info, 0, &_r_vk_state.graphics_pipelines[id].instance_set_pool[i]));
		}
	}	

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

		if (binding_count != 0)
		{
			VkDescriptorSetLayoutCreateInfo layout = {
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
				.bindingCount = binding_count,
				.pBindings = bindings
			};
			VK_CHECK(vkCreateDescriptorSetLayout(_r_vk_state.device.logical, &layout, 0, &_r_vk_state.graphics_pipelines[id].global_set_layout));
		}
  }

  // Instance Set
  {
		VkDescriptorSetLayoutBinding bindings[R_MAX_BINDINGS] = {};
		U32 binding_count = 0;
		LOG_INFO("Instance count: %d", pipeline->instance_bindings_count);
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

		if (binding_count != 0)
		{
			VkDescriptorSetLayoutCreateInfo layout = {
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
				.bindingCount = binding_count,
				.pBindings = bindings
			};
			VK_CHECK(vkCreateDescriptorSetLayout(_r_vk_state.device.logical, &layout, 0, &_r_vk_state.graphics_pipelines[id].instance_set_layout));
		}
  }

  VkDescriptorSetLayout set_layouts[] = {
    _r_vk_state.graphics_pipelines[id].global_set_layout,
    _r_vk_state.graphics_pipelines[id].instance_set_layout
  };
  
  VkPipelineLayoutCreateInfo layout_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount = CountArrayElements(set_layouts),
    .pSetLayouts = set_layouts,
  };
  VK_CHECK(vkCreatePipelineLayout(_r_vk_state.device.logical, &layout_info, 0, &_r_vk_state.graphics_pipelines[id].layout));

  U32 stride = 0;
  VkVertexInputAttributeDescription attribute_descriptions[R_MAX_ATTRIBUTES];
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
    .layout = _r_vk_state.graphics_pipelines[id].layout,
    .renderPass = 0,
    .subpass = 0,
  };
  VK_CHECK(vkCreateGraphicsPipelines(_r_vk_state.device.logical,
                                     0, 1, &pipeline_info, 0,
                                     &_r_vk_state.graphics_pipelines[id].handle));


  _r_vk_state.graphics_pipelines_count += 1;
}

func void
R_VK_DestroyGraphicsPipeline()
{
}

func void
R_VK_BindPipeline(R_Pipeline* pipeline, U8* global_data, U32 global_data_size)
{
  R_VK_GraphicsPipeline vk_pipeline = _r_vk_state.graphics_pipelines[pipeline->backend_handle];
  VkCommandBuffer cmd = _r_vk_state.swapchain.frame_resources[_r_vk_state.current_frame].cmd_buffer;
  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline.handle);

	U64 global_data_buffer_offset = 0;

  void* data;
  vkMapMemory(_r_vk_state.device.logical, _r_vk_state.geometry_buffer.memory, _r_vk_state.geometry_buffer.size, VK_WHOLE_SIZE, 0, &data);
  {
		global_data_buffer_offset = _r_vk_state.geometry_buffer.size;
    memcpy((U8*)data, global_data, global_data_size);
    U32 offset = (global_data_size + 64) - (global_data_size%64);
    _r_vk_state.geometry_buffer.size += offset;
	}
  vkUnmapMemory(_r_vk_state.device.logical, _r_vk_state.geometry_buffer.memory);

	VkDescriptorSet current_set = vk_pipeline.global_sets[_r_vk_state.current_frame];

	// --AlNov: @TODO There is no Free
	VkDescriptorSetAllocateInfo allocate_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = vk_pipeline.global_set_pool[_r_vk_state.current_frame],
		.descriptorSetCount = 1,
		.pSetLayouts = &vk_pipeline.global_set_layout
	};
	vkAllocateDescriptorSets(_r_vk_state.device.logical, &allocate_info, &current_set);

	VkDescriptorBufferInfo buffer_info = {
		.buffer = _r_vk_state.geometry_buffer.handle,
		.offset = global_data_buffer_offset,
		.range = global_data_size 
	};

	VkWriteDescriptorSet write_info = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = current_set,
		.dstBinding = 0,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.pBufferInfo = &buffer_info
	};
	vkUpdateDescriptorSets(_r_vk_state.device.logical, 1, &write_info, 0, 0);
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline.layout, 1, 1, &current_set, 0, 0);
}

// --------------------------------------------------
// Render Pass
func void
R_VK_RenderPassBegin(R_AttachmentLoadOperation load_operation, Vec4f clear_color)
{
  VkClearValue clear_value = {};
  clear_value.color.float32[0] = clear_color.r;
  clear_value.color.float32[1] = clear_color.g;
  clear_value.color.float32[2] = clear_color.b;
  clear_value.color.float32[3] = clear_color.a;

  VkRenderingAttachmentInfo color_attachment = {
    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
    .imageView = _r_vk_state.swapchain.image_views[_r_vk_state.current_target],
    .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    .loadOp = R_VK_GetVkAttachmentLoadOperation(load_operation),
    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    .clearValue = clear_value
  };

  VkRenderingAttachmentInfo depth_attachment = {
    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
    .imageView = _r_vk_state.swapchain.depth_image_view,
    .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
    .loadOp = R_VK_GetVkAttachmentLoadOperation(load_operation),
    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    .clearValue = {
      .depthStencil = {
        .depth = 0.0
      }
    }
  };

  VkExtent2D render_area = {
    .width = _r_vk_state.swapchain.size.width,
    .height = _r_vk_state.swapchain.size.height
  };
  VkRenderingInfo rendering_info = {
    .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
    .renderArea = {
      .offset = { 0, 0 },
      .extent = render_area
    },
    .layerCount = 1,
    .colorAttachmentCount = 1,
    .pColorAttachments = &color_attachment,
    .pDepthAttachment = &depth_attachment
  };
  
  VkCommandBuffer cmd = _r_vk_state.swapchain.frame_resources[_r_vk_state.current_frame].cmd_buffer;
  R_VK_TransitImageLayout(
    cmd,
    _r_vk_state.swapchain.depth_image,
    VK_IMAGE_LAYOUT_UNDEFINED,
    VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
    0,
    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
    VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
    VK_IMAGE_ASPECT_DEPTH_BIT
  );

  vkCmdBeginRendering(cmd, &rendering_info);
}

func void
R_VK_RenderPassEnd()
{
  VkCommandBuffer cmd = _r_vk_state.swapchain.frame_resources[_r_vk_state.current_frame].cmd_buffer;
  R_VK_TransitImageLayout(
    cmd,
    _r_vk_state.swapchain.depth_image,
    VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
    VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
    VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
    VK_IMAGE_ASPECT_DEPTH_BIT
  );
  
  vkCmdEndRendering(cmd);
}
// -------------------------------------------------------------------
// Command Buffer
func VkCommandBuffer
R_VK_BeginSingleCmd()
{
  VkCommandBufferAllocateInfo allocate_info = {};
  allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocate_info.commandPool = _r_vk_state.cmd_pool;
  allocate_info.commandBufferCount = 1;

  VkCommandBuffer command_buffer;
  VK_CHECK(vkAllocateCommandBuffers(_r_vk_state.device.logical, &allocate_info, &command_buffer));

  VkCommandBufferBeginInfo begin_info = {};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  VK_CHECK(vkBeginCommandBuffer(command_buffer, &begin_info));

  return command_buffer; 
}

func void
R_VK_EndSingleCmd(VkCommandBuffer cmd)
{
  VK_CHECK(vkEndCommandBuffer(cmd));

  VkSubmitInfo submit_info = {};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &cmd;

  VK_CHECK(vkQueueSubmit(_r_vk_state.device.graphics_queue, 1, &submit_info, 0));
  VK_CHECK(vkQueueWaitIdle(_r_vk_state.device.graphics_queue));

  vkFreeCommandBuffers(_r_vk_state.device.logical, _r_vk_state.cmd_pool, 1, &cmd);
}

// -------------------------------------------------------------------
// Texture
func R_Texture
R_VK_CreateTexture(Str8 path)
{
  R_Texture texture = {};

  I32 tex_width    = 0;
  I32 tex_height   = 0;
  I32 tex_channels = 0;
  U8* tex_pixels   = stbi_load(CFromStr8(path), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);

  if (!tex_pixels)
  {
    LOG_ERROR("Cannot load texture %s\n", path);
  }

  texture.size = tex_width * tex_height * 4;

  void* data;
  vkMapMemory(_r_vk_state.device.logical, _r_vk_state.staging_buffer.memory, 0, VK_WHOLE_SIZE, 0, &data);
    memcpy(data, tex_pixels, texture.size);
  vkUnmapMemory(_r_vk_state.device.logical, _r_vk_state.staging_buffer.memory);

  stbi_image_free(tex_pixels);

  VkImageCreateInfo image_info = {};
  image_info.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image_info.samples       = VK_SAMPLE_COUNT_1_BIT;
  image_info.imageType     = VK_IMAGE_TYPE_2D;
  image_info.extent.width  = tex_width;
  image_info.extent.height = tex_height;
  image_info.extent.depth  = 1;
  image_info.mipLevels     = 1;
  image_info.arrayLayers   = 1;
  image_info.format        = VK_FORMAT_R8G8B8A8_SRGB;
  image_info.tiling        = VK_IMAGE_TILING_OPTIMAL;
  image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_info.usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  image_info.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
  image_info.samples       = VK_SAMPLE_COUNT_1_BIT;
  if (vkCreateImage(_r_vk_state.device.logical, &image_info, 0, &_r_vk_state.default_texture.image) != VK_SUCCESS)
  {
    LOG_ERROR("Cannot create Image for Texture.\n");
    return texture;
  }

  VkMemoryRequirements mem_requirements = {};
  vkGetImageMemoryRequirements(_r_vk_state.device.logical, _r_vk_state.default_texture.image, &mem_requirements);

  VkMemoryAllocateInfo mem_info = {};
  mem_info.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  mem_info.allocationSize  = mem_requirements.size;
  mem_info.memoryTypeIndex = R_VK_FindMemoryTypeIndex(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  VK_CHECK(vkAllocateMemory(_r_vk_state.device.logical, &mem_info, 0, &_r_vk_state.default_texture.memory));

  VK_CHECK(vkBindImageMemory(_r_vk_state.device.logical, _r_vk_state.default_texture.image, _r_vk_state.default_texture.memory, 0));

  VkCommandBuffer cmd = R_VK_BeginSingleCmd();
  {
    R_VK_TransitImageLayout(
      cmd,
      _r_vk_state.default_texture.image,
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      0,
      VK_ACCESS_TRANSFER_WRITE_BIT,
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
      VK_PIPELINE_STAGE_TRANSFER_BIT,
      VK_IMAGE_ASPECT_COLOR_BIT
    );
    
    VkBufferImageCopy copy_info = {};
    copy_info.bufferOffset                    = 0;
    copy_info.bufferRowLength                 = 0;
    copy_info.bufferImageHeight               = 0;
    copy_info.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_info.imageSubresource.mipLevel       = 0;
    copy_info.imageSubresource.baseArrayLayer = 0;
    copy_info.imageSubresource.layerCount     = 1;
    copy_info.imageOffset                     = { 0, 0, 0 };
    copy_info.imageExtent                     = { (U32)tex_width, (U32)tex_height, 1 };
    vkCmdCopyBufferToImage(cmd, _r_vk_state.staging_buffer.handle, _r_vk_state.default_texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_info);
    
    R_VK_TransitImageLayout(
      cmd,
      _r_vk_state.default_texture.image,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      VK_ACCESS_TRANSFER_WRITE_BIT,
      VK_ACCESS_SHADER_READ_BIT,
      VK_PIPELINE_STAGE_TRANSFER_BIT,
      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
      VK_IMAGE_ASPECT_COLOR_BIT
    );
    R_VK_EndSingleCmd(cmd);
  }
  
  // AlNov: Create Texture Image View
  VkImageViewCreateInfo view_info = {};
  view_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  view_info.image                           = _r_vk_state.default_texture.image;
  view_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
  view_info.format                          = VK_FORMAT_R8G8B8A8_SRGB;
  view_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
  view_info.subresourceRange.baseMipLevel   = 0;
  view_info.subresourceRange.levelCount     = 1;
  view_info.subresourceRange.baseArrayLayer = 0;
  view_info.subresourceRange.layerCount     = 1;

  VK_CHECK(vkCreateImageView(_r_vk_state.device.logical, &view_info, 0, &_r_vk_state.default_texture.view));

  // AlNov: Create Texture Sampler
  VkSamplerCreateInfo sampler_info = {};
  sampler_info.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  sampler_info.magFilter               = VK_FILTER_LINEAR;
  sampler_info.minFilter               = VK_FILTER_LINEAR;
  sampler_info.addressModeU            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
  sampler_info.addressModeV            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
  sampler_info.addressModeW            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
  sampler_info.anisotropyEnable        = VK_FALSE;
  sampler_info.borderColor             = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
  sampler_info.unnormalizedCoordinates = VK_FALSE;
  sampler_info.compareEnable           = VK_FALSE;
  sampler_info.compareOp               = VK_COMPARE_OP_ALWAYS;
  sampler_info.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  sampler_info.mipLodBias              = 0.0f;
  sampler_info.minLod                  = 0.0f;
  sampler_info.maxLod                  = 0.0f;

  VK_CHECK(vkCreateSampler(_r_vk_state.device.logical, &sampler_info, 0, &_r_vk_state.default_sampler));

  return texture;  
}

// --------------------------------------------------
// Draw
func void
R_VK_BeginFrame()
{
  vkWaitForFences(_r_vk_state.device.logical, 1, &_r_vk_state.swapchain.frame_resources[_r_vk_state.current_frame].submit_fence, VK_TRUE, U64_MAX);
  
  R_VK_SwapchainAcquireNextImage(&_r_vk_state.current_target);
  _r_vk_state.geometry_buffer.size = 0;

  R_VK_GraphicsPipeline binded_pipeline = _r_vk_state.graphics_pipelines[_r_vk_state.binded_pipeline_id];
  vkResetDescriptorPool(_r_vk_state.device.logical, binded_pipeline.instance_set_pool[_r_vk_state.current_frame], 0);
  
  vkResetFences(_r_vk_state.device.logical, 1, &_r_vk_state.swapchain.frame_resources[_r_vk_state.current_frame].submit_fence);

  vkResetCommandBuffer(_r_vk_state.swapchain.frame_resources[_r_vk_state.current_frame].cmd_buffer, 0);
  
  VkCommandBufferBeginInfo begin_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
  }; 

  VkCommandBuffer cmd = _r_vk_state.swapchain.frame_resources[_r_vk_state.current_frame].cmd_buffer;
  VK_CHECK(vkBeginCommandBuffer(cmd, &begin_info));
  
  R_VK_TransitImageLayout(
    cmd,
    _r_vk_state.swapchain.images[_r_vk_state.current_target],
    VK_IMAGE_LAYOUT_UNDEFINED,
    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    0,
    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    VK_IMAGE_ASPECT_COLOR_BIT
  );
}

func void
R_VK_EndFrame()
{
  VkCommandBuffer cmd = _r_vk_state.swapchain.frame_resources[_r_vk_state.current_frame].cmd_buffer;
  vkCmdEndRendering(cmd);
  
  R_VK_TransitImageLayout(
    cmd,
    _r_vk_state.swapchain.images[_r_vk_state.current_target],
    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    0,
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
    VK_IMAGE_ASPECT_COLOR_BIT
  );

  VK_CHECK(vkEndCommandBuffer(cmd));

  VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

  VkSubmitInfo submit_info = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = &_r_vk_state.swapchain.frame_resources[_r_vk_state.current_frame].acquire_semaphore,
    .pWaitDstStageMask = &wait_stage,
    .commandBufferCount = 1,
    .pCommandBuffers = &cmd,
    .signalSemaphoreCount = 1,
    .pSignalSemaphores = &_r_vk_state.swapchain.frame_resources[_r_vk_state.current_target].release_semaphore
  };

  VkResult vk_result = (vkQueueSubmit(_r_vk_state.device.graphics_queue, 1, &submit_info, _r_vk_state.swapchain.frame_resources[_r_vk_state.current_frame].submit_fence));
  VK_CHECK(vk_result);
  
  VkPresentInfoKHR present_info = {
    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = &_r_vk_state.swapchain.frame_resources[_r_vk_state.current_target].release_semaphore,
    .swapchainCount = 1,
    .pSwapchains = &_r_vk_state.swapchain.handle,
    .pImageIndices = &_r_vk_state.current_target
  };

  VkResult present_result = vkQueuePresentKHR(_r_vk_state.device.graphics_queue, &present_info);

  if (present_result != VK_SUCCESS)
  {
    // return false;
  }

  _r_vk_state.current_frame = (_r_vk_state.current_frame + 1) % R_VK_FRAMES_IN_FLIGHT;
}

func void R_VK_GeometryPrepare(AST_Geometry* geometry)
{
  void* data;
  vkMapMemory(_r_vk_state.device.logical, _r_vk_state.geometry_buffer.memory, _r_vk_state.geometry_buffer.size, VK_WHOLE_SIZE, 0, &data);
  {
    U64 offset = 0;
    
    U64 index_data_size = geometry->index_size * geometry->index_count;
    memcpy(data, geometry->index_data, index_data_size);
    geometry->index_r_backend_offset = _r_vk_state.geometry_buffer.size;
    offset += index_data_size + index_data_size%4;
    _r_vk_state.geometry_buffer.size += offset;
    
    U64 vertex_data_size = geometry->vertex_size * geometry->vertex_count;
    memcpy((U8*)data + offset, geometry->vertex_data, vertex_data_size);
    geometry->vertex_r_backend_offset = _r_vk_state.geometry_buffer.size;
    offset += vertex_data_size + vertex_data_size%4;
    _r_vk_state.geometry_buffer.size += offset;
  }
  vkUnmapMemory(_r_vk_state.device.logical, _r_vk_state.geometry_buffer.memory);
}

func B32
R_VK_GeometryDraw(R_DrawGeometryInfo* draw_info)
{
  VkCommandBuffer cmd = _r_vk_state.swapchain.frame_resources[_r_vk_state.current_frame].cmd_buffer;
  
  VkExtent2D render_area = {
    .width = _r_vk_state.swapchain.size.width,
    .height = _r_vk_state.swapchain.size.height
  };
    
  VkViewport viewport = {
    .width = (F32)draw_info->viewport.w,
    .height = (F32)draw_info->viewport.h,
    .minDepth = 0.0f,
    .maxDepth = 1.0f
  };
  vkCmdSetViewport(cmd, 0, 1, &viewport);

  VkRect2D scissor = {
    .offset = {
      .x = draw_info->scissor.x,
      .y = draw_info->scissor.y
    },
    .extent = {
      .width = (U32)draw_info->scissor.w,
      .height = (U32)draw_info->scissor.h
    }
  };
  vkCmdSetScissor(cmd, 0, 1, &scissor);
  
	U64 global_data_buffer_offset = 0;
  U64 instance_data_buffer_offset = 0;
  
  _r_vk_state.geometry_buffer.size = (_r_vk_state.geometry_buffer.size + 64) - _r_vk_state.geometry_buffer.size%64;
  
  void* data;
  vkMapMemory(_r_vk_state.device.logical, _r_vk_state.geometry_buffer.memory, _r_vk_state.geometry_buffer.size, VK_WHOLE_SIZE, 0, &data);
  {
    instance_data_buffer_offset = _r_vk_state.geometry_buffer.size;
    U64 instance_data_size = draw_info->instance_data_size;
    memcpy((U8*)data, draw_info->instance_data, instance_data_size);
    U32 offset = (instance_data_size + 64) - (instance_data_size%64);
    _r_vk_state.geometry_buffer.size += offset;
  }
  vkUnmapMemory(_r_vk_state.device.logical, _r_vk_state.geometry_buffer.memory);

	R_VK_GraphicsPipeline* current_pipeline = _r_vk_state.graphics_pipelines + draw_info->pipeline->backend_handle;

	// Intance Set
	{
		// --AlNov: @TODO How to choose what descriptor set to use
		current_pipeline->object_number = 1;
		VkDescriptorSet set = current_pipeline->instance_sets[current_pipeline->object_number];
		// current_pipeline->object_number += 1;

		VkDescriptorSetAllocateInfo allocate_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = current_pipeline->instance_set_pool[_r_vk_state.current_frame],
			.descriptorSetCount = 1,
			.pSetLayouts = &current_pipeline->instance_set_layout
		};
		vkAllocateDescriptorSets(_r_vk_state.device.logical, &allocate_info, &set);

		VkDescriptorBufferInfo buffer_info = {
			.buffer = _r_vk_state.geometry_buffer.handle,
			.offset = instance_data_buffer_offset,
			.range = draw_info->instance_data_size
		};

		VkDescriptorImageInfo image_info = {
			.sampler = _r_vk_state.default_sampler,
			.imageView = _r_vk_state.default_texture.view,
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};

		VkWriteDescriptorSet write_infos[4] = {};
		U32 write_count = 0;

		for (U32 i = 0; i < draw_info->pipeline->instance_bindings_count; i += 1)
		{
			R_BindingInfo* binding_info = &draw_info->pipeline->instance_bindings[i];

			if (binding_info->type == R_BINDING_TYPE_UNIFORM_BUFFER)
			{
				write_infos[i] = {
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = set,
					.dstBinding = i,
					.dstArrayElement = 0,
					.descriptorCount = 1,
					.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					.pBufferInfo = &buffer_info
				};
			}
			else if (binding_info->type == R_BINDING_TYPE_TEXTURE_2D)
			{
				write_infos[i] = {
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = set,
					.dstBinding = i,
					.dstArrayElement = 0,
					.descriptorCount = 1,
					.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.pImageInfo = &image_info
				};
			}
			write_count += 1;
		}

		vkUpdateDescriptorSets(_r_vk_state.device.logical, write_count, write_infos, 0, 0);
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _r_vk_state.graphics_pipelines[draw_info->pipeline->backend_handle].layout, 0, 1, &set, 0, 0);
	}

  vkCmdBindIndexBuffer(cmd, _r_vk_state.geometry_buffer.handle, draw_info->geometry->index_r_backend_offset, VK_INDEX_TYPE_UINT16);
  
  VkDeviceSize offset = { draw_info->geometry->vertex_r_backend_offset };
  vkCmdBindVertexBuffers(cmd, 0, 1, &_r_vk_state.geometry_buffer.handle, &offset);
  
  vkCmdDrawIndexed(cmd, draw_info->geometry->index_count, 1, 0, 0, 0);

  return true;
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

  VK_CHECK(vkCreateInstance(&instance_info, 0, &_r_vk_state.instance));

#if IGNIS_DEBUG
  VK_CHECK(R_VK_CreateDebugMessenger(_r_vk_state.instance, &_r_vk_state.debug_messenger));
#endif // IGNIS_DEBUG

  R_VK_CreateDevice();
  R_VK_CreateSwapchain(window);

  VkCommandPoolCreateInfo cmd_pool_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
    .queueFamilyIndex = _r_vk_state.device.graphics_queue_index
  };
  VK_CHECK(vkCreateCommandPool(_r_vk_state.device.logical, &cmd_pool_info, 0, &_r_vk_state.cmd_pool));

  _r_vk_state.geometry_buffer = R_VK_CreateBuffer(Megabytes(256), BUFFER_USAGE_FLAG_UNIFORM | BUFFER_USAGE_FLAG_VERTEX | BUFFER_USAGE_FLAG_INDEX, BUFFER_PROPERTY_HOST_COHERENT | BUFFER_PROPERTY_HOST_VISIBLE);
  _r_vk_state.staging_buffer = R_VK_CreateBuffer(Megabytes(64), BUFFER_USAGE_FLAG_TRANSFER_SRC, BUFFER_PROPERTY_HOST_COHERENT | BUFFER_PROPERTY_HOST_VISIBLE);

	return 0;
}

func B32
R_VK_Shutdown()
{
  
	return 0;
}

func void
R_VK_Shutdown(OS_Window* window)
{
}

func void
R_VK_HandleResize(OS_Window* window)
{
  R_VK_RecreateSwapchain(window);
}

// -------------------------------------------------------------------
// Debug Tools
#if IGNIS_DEBUG
VKAPI_ATTR VkBool32 VKAPI_CALL
R_VK_DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
  LOG_INFO("VK_VALIDATION: %s\n", pCallbackData->pMessage);

  return VK_FALSE;
}

func void
R_VK_PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& messengerInfo)
{
  messengerInfo = {};

  messengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  messengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  messengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
  messengerInfo.pfnUserCallback = R_VK_DebugCallback;
  messengerInfo.pUserData = nullptr;
}

func VkResult
R_VK_CreateDebugUtilsMessenger(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMesseneger)
{
  auto f = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

  if (f != nullptr)
  {
    return f(instance, pCreateInfo, pAllocator, pDebugMesseneger);
  }
  else
  {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

func void
R_VK_DestroyDebugUtilsMessenger(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, VkAllocationCallbacks* pAllocator)
{
  auto f = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

  if (f != nullptr)
  {
    f(instance, debugMessenger, pAllocator);
  }
}

func VkResult
R_VK_CreateDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT* debugMessenger)
{
  VkDebugUtilsMessengerCreateInfoEXT messengerInfo = {};
  R_VK_PopulateDebugMessengerCreateInfo(messengerInfo);

  return R_VK_CreateDebugUtilsMessenger(instance, &messengerInfo, nullptr, debugMessenger);
}
#endif // IGNIS_DEBUG
