#include "r_vulkan.h"

#include "base/base_core.h"

// --------------------------------------------------
// Buffer
func VkBufferUsageFlags
_VkFromBufferUsageFlags(R_BufferUsageFlags flags)
{
  VkBufferUsageFlags result = 0;

  if((flags & R_BUFFER_USAGE_FLAG_VERTEX) == R_BUFFER_USAGE_FLAG_VERTEX)
  {
    result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  }
  if((flags & R_BUFFER_USAGE_FLAG_INDEX) == R_BUFFER_USAGE_FLAG_INDEX)
  {
    result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  }
  if((flags & R_BUFFER_USAGE_FLAG_UNIFORM) == R_BUFFER_USAGE_FLAG_UNIFORM)
  {
    result |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  }
  if((flags & R_BUFFER_USAGE_FLAG_TRANSFER_SRC) == R_BUFFER_USAGE_FLAG_TRANSFER_SRC)
  {
    result |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  }

  return result;
}

func VkMemoryPropertyFlags
_VkFromBufferPropertyFlags(R_BufferPropertyFlags flags)
{
  VkMemoryPropertyFlags result = 0;

  if ((flags & R_BUFFER_PROPERTY_FLAG_HOST_COHERENT) == R_BUFFER_PROPERTY_FLAG_HOST_COHERENT)
  {
    result |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  }
  if ((flags & R_BUFFER_PROPERTY_FLAG_HOST_VISIBLE) == R_BUFFER_PROPERTY_FLAG_HOST_VISIBLE)
  {
    result |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
  }

  return result;
}

func R_Buffer*
R_VK_CreateBuffer(U32 capacity, R_BufferUsageFlags usage_flags, R_BufferPropertyFlags property_flags)
{
  // @NOTE This is to create Vulkan Buffer and Memory
  R_VK_Buffer* buffer = (R_VK_Buffer*)PushArena(_r_vk_state.arena, sizeof(R_VK_Buffer));
	buffer->capacity = capacity;
  
  VkBufferCreateInfo buffer_info = {
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .size = capacity,
    .usage = _VkFromBufferUsageFlags(usage_flags),
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE
  };

  VK_CHECK(vkCreateBuffer(_r_vk_state.device.logical, &buffer_info, 0, &buffer->handle));

  VkMemoryRequirements memory_requirements;
  vkGetBufferMemoryRequirements(_r_vk_state.device.logical, buffer->handle, &memory_requirements);
  
  VkPhysicalDeviceMemoryProperties mem_properties = {0};
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
  VK_CHECK(vkAllocateMemory(_r_vk_state.device.logical, &allocation_info , 0, &buffer->memory));

  VK_CHECK(vkBindBufferMemory(_r_vk_state.device.logical, buffer->handle, buffer->memory, 0));

  vkMapMemory(_r_vk_state.device.logical, buffer->memory, 0, VK_WHOLE_SIZE, 0, &buffer->mapped);

  return (R_Buffer*)buffer;
}

func U64 R_VK_PushBuffer(R_Buffer* buffer, U8* data, U64 size)
{
	R_VK_Buffer* vk_buffer = (R_VK_Buffer*)buffer;

	Assert((vk_buffer->size + size) > vk_buffer->capacity);
	U32 offset = vk_buffer->size;

	memcpy((U8*)vk_buffer->mapped + vk_buffer->size, data, size);
	vk_buffer->size += size;
	U64 padding = 64 - (vk_buffer->size + 64)%64;
	vk_buffer->size += padding;

	return offset;
}

func void
R_VK_ResetBuffer(R_Buffer* buffer)
{
	R_VK_Buffer* vk_buffer = (R_VK_Buffer*)buffer;
	vk_buffer->size = 0;
}
func void
R_VK_BindIndexBuffer(R_CommandBuffer* command_buffer, R_Buffer* buffer, U64 offset, R_IndexSize index_size)
{
	R_VK_CommandBuffer* vk_command_buffer = (R_VK_CommandBuffer*)command_buffer;
	R_VK_Buffer* vk_buffer = (R_VK_Buffer*)buffer;

	vkCmdBindIndexBuffer(
			vk_command_buffer->handle[_r_vk_state.current_frame],
			vk_buffer->handle,
			offset, R_VK_GetVkIndexTypeFrom(index_size));
}

func void
R_VK_BindVertexBuffer(R_CommandBuffer* command_buffer, R_Buffer* buffer, U64 offset)
{
	R_VK_CommandBuffer* vk_command_buffer = (R_VK_CommandBuffer*)command_buffer;
	R_VK_Buffer* vk_buffer = (R_VK_Buffer*)buffer;

	VkDeviceSize vk_offset = offset;
	vkCmdBindVertexBuffers(vk_command_buffer->handle[_r_vk_state.current_frame], 0, 1, &vk_buffer->handle, &vk_offset);
}

func void R_VK_DestroyBuffer(R_VK_Buffer* buffer)
{
  vkFreeMemory(_r_vk_state.device.logical, buffer->memory, 0);
  vkDestroyBuffer(_r_vk_state.device.logical, buffer->handle, 0);

	R_VK_Buffer reset = {0};
  *buffer = reset;
}

// -------------------------------------------------------------------
// Command Buffer
func R_CommandBuffer* R_VK_GetCommandBuffer(void)
{
	R_VK_CommandBuffer* command_buffer = (R_VK_CommandBuffer*)PushArena(_r_vk_state.arena, sizeof(command_buffer));

	VkCommandBufferAllocateInfo allocate_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = _r_vk_state.command_pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = R_FRAMES_IN_FLIGHT
	};
	VK_CHECK(vkAllocateCommandBuffers(_r_vk_state.device.logical, &allocate_info, command_buffer->handle));

	for (I32 i = 0; i < R_FRAMES_IN_FLIGHT; i += 1)
	{
		VkFenceCreateInfo fence_info = {
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT
		};
		VK_CHECK(vkCreateFence(_r_vk_state.device.logical, &fence_info, 0, (command_buffer->submit_fence + i)));
	}

	for (I32 i = 0; i < R_FRAMES_IN_FLIGHT; i += 1)
	{
		VkSemaphoreCreateInfo semaphore_info = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		};
		VK_CHECK(vkCreateSemaphore(_r_vk_state.device.logical, &semaphore_info, 0, (command_buffer->acquire_semaphore + i)));
		VK_CHECK(vkCreateSemaphore(_r_vk_state.device.logical, &semaphore_info, 0, (command_buffer->release_semaphore + i)));
	}

	return (R_CommandBuffer*)command_buffer;
}

func void R_VK_BeginCommandBuffer(R_CommandBuffer* command_buffer)
{
	R_VK_CommandBuffer* vk_command_buffer = (R_VK_CommandBuffer*)command_buffer;

#if 0
	vkResetCommandBuffer(vk_command_buffer->handle[_r_vk_state.current_frame], 0);
	for (I32 i = 0; i < vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].pool_count; i += 1)
	{
		vkDestroyDescriptorPool(_r_vk_state.device.logical, vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].vk_pools[i], 0);
	}

	vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].pool_count = 0;
#endif
	vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].sets_count = 0;

  VkCommandBufferBeginInfo begin_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
  }; 
  VK_CHECK(vkBeginCommandBuffer(vk_command_buffer->handle[_r_vk_state.current_frame], &begin_info));
}

func void
R_VK_SubmitCommandBuffer(R_CommandBuffer* command_buffer)
{
	R_VK_CommandBuffer* vk_command_buffer = (R_VK_CommandBuffer*)command_buffer;

	vkEndCommandBuffer(vk_command_buffer->handle[_r_vk_state.current_frame]);

	VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = vk_command_buffer->acquire_semaphore + _r_vk_state.current_frame,
		.pWaitDstStageMask = &wait_stage,
		.commandBufferCount = 1,
		.pCommandBuffers = vk_command_buffer->handle + _r_vk_state.current_frame,
		.signalSemaphoreCount = 0,
    .pSignalSemaphores = &_r_vk_state.swapchain.frame_resources[_r_vk_state.current_target].release_semaphore, // @TODO change location of release_semaphore
	};

  VK_CHECK(vkQueueSubmit(_r_vk_state.device.graphics_queue, 1, &submit_info, vk_command_buffer->submit_fence[_r_vk_state.current_frame]));

	// @TODO There is not always should be Present command
	VkPresentInfoKHR present_info = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 0,
    .pWaitSemaphores = &_r_vk_state.swapchain.frame_resources[_r_vk_state.current_target].release_semaphore, // @TODO change location of release_semaphore
		.swapchainCount = 1,
		.pSwapchains = &_r_vk_state.swapchain.handle,
    .pImageIndices = &_r_vk_state.current_target,
	};

	VkResult present_result = vkQueuePresentKHR(_r_vk_state.device.graphics_queue, &present_info);

	_r_vk_state.current_frame = (_r_vk_state.current_frame + 1)%R_FRAMES_IN_FLIGHT;
}

// -------------------------------------------------------------------
// Descriptor Sets
func void
R_VK_BindGlobalVertexUniformData(R_CommandBuffer* command_buffer, R_Buffer* buffer, U64 offset, U64 data_size)
{
	R_VK_CommandBuffer* vk_command_buffer = (R_VK_CommandBuffer*)command_buffer;

	U32 num_sets = vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].pool_count*R_VK_SETS_PER_POOL;
	if (num_sets <= vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].sets_count)
	{
		Assert(vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].pool_count >= R_VK_MAX_POOL_COUNT);

		LOG_DEBUG("CreateDescriptroPool number %d\n", vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].pool_count);

		VkDescriptorPoolSize uniform_pool_size = {
			.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = R_VK_UNIFORM_BUFFERS_PER_SET*R_VK_SETS_PER_POOL,
		};

		VkDescriptorPoolSize pool_sizes[] = {
			uniform_pool_size,
		};

		VkDescriptorPoolCreateInfo pool_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.flags = 0,
			.maxSets = R_VK_SETS_PER_POOL,
			.poolSizeCount = CountArrayElements(pool_sizes),
			.pPoolSizes = pool_sizes,
		};
		VK_CHECK(vkCreateDescriptorPool(_r_vk_state.device.logical, &pool_info, 0, vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].vk_pools + vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].pool_count));

		VkDescriptorSetLayout layouts[R_VK_SETS_PER_POOL] = {0};
		for (I32 i = 0; i < R_VK_SETS_PER_POOL; i += 1)
		{
			layouts[i] = vk_command_buffer->binded_graphics_pipeline->vertex_shader_set_layout;
		}

		VkDescriptorSetAllocateInfo sets_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].vk_pools[vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].pool_count],
			.descriptorSetCount = R_VK_SETS_PER_POOL,
			.pSetLayouts = layouts,
		};
		VK_CHECK(vkAllocateDescriptorSets(_r_vk_state.device.logical, &sets_info, &vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].vk_sets[vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].pool_count][0]));

		vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].pool_count += 1;
	}

	R_VK_Buffer* vk_buffer = (R_VK_Buffer*)buffer;
	VkDescriptorBufferInfo buffer_info = {
		.buffer = vk_buffer->handle,
		.offset = offset,
		.range = data_size,
	};
	
	I32 pool_id = (U32)(vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].sets_count/R_VK_SETS_PER_POOL);
	I32 set_id = vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].sets_count%R_VK_SETS_PER_POOL;
	VkWriteDescriptorSet write_info = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].vk_sets[pool_id][set_id],
		.dstBinding = 0, // @TODO Add more bindings
		.dstArrayElement = 0,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.pBufferInfo = &buffer_info,
	};
	vkUpdateDescriptorSets(_r_vk_state.device.logical, 1, &write_info, 0, 0);

	vkCmdBindDescriptorSets(
			vk_command_buffer->handle[_r_vk_state.current_frame],
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			vk_command_buffer->binded_graphics_pipeline->layout,
			0, 1, &vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].vk_sets[pool_id][set_id], 0, 0);

	vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].sets_count += 1;
}

func void
R_VK_BindGlobalFragmentUniformData(R_CommandBuffer* command_buffer, R_Buffer* buffer, U64 offset, U64 data_size)
{
}

// --------------------------------------------------
// Device
func void
R_VK_CreateDevice(void)
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

      for (I32 j = 0; j < queue_family_count; j += 1)
      {
        VkQueueFamilyProperties* properties = queue_properties + j;

        if (properties->queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
          _r_vk_state.device.graphics_queue_index = j;
          break;
        }
      }

      const char* required_extensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
      };

      VkPhysicalDeviceVulkan13Features vulkan13_features = {0};
      vulkan13_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
      vulkan13_features.dynamicRendering = VK_TRUE;

      F32 queue_priority = 1.0f;

      VkDeviceQueueCreateInfo queue_info = {0};
      queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queue_info.queueFamilyIndex = _r_vk_state.device.graphics_queue_index;
      queue_info.queueCount = 1;
      queue_info.pQueuePriorities = &queue_priority;

      VkDeviceCreateInfo device_info = {0};
      device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
      device_info.queueCreateInfoCount = 1;
      device_info.pQueueCreateInfos = &queue_info;
      device_info.enabledExtensionCount = CountArrayElements(required_extensions);
      device_info.ppEnabledExtensionNames = required_extensions;
      device_info.pEnabledFeatures = 0;
      device_info.pNext = &vulkan13_features;

      LOG_INFO("%s\n", properties.deviceName);
      _r_vk_state.device.physical = *device;
      VK_CHECK(vkCreateDevice(*device, &device_info, 0, &_r_vk_state.device.logical))
			if(_r_vk_state.device.logical)
			{
				vkGetDeviceQueue(_r_vk_state.device.logical, _r_vk_state.device.graphics_queue_index, 0, &_r_vk_state.device.graphics_queue);
				break;
				// @TODO Choose GPU by parameters
			}
    }
  }
  FreeArena(tmp_arena);
}

func void
R_VK_DestroyDevice(void)
{
  vkDestroyDevice(_r_vk_state.device.logical, 0);

	R_VK_Device reset = {0};
  _r_vk_state.device = reset;
}

// --------------------------------------------------
// Surface/Swapchain
func FrameResources
R_VK_CreateFrameResources(void)
{
  FrameResources resources = {0};

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

	FrameResources reset = {0};
  *resources = reset;
}

func void
R_VK_SurfaceCreate(OS_Window* window)
{
}

func void
R_VK_DestorySurface(void)
{
  vkDestroySurfaceKHR(_r_vk_state.instance, _r_vk_state.swapchain.surface, 0);
}

func void
R_VK_CreateSwapchain(OS_Window* window)
{
#if IGNIS_PLATFORM_WIN32
  VkWin32SurfaceCreateInfoKHR surface_info = {0};
  surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  surface_info.hinstance = window->handle->instance;
  surface_info.hwnd = window->handle->handle;

  VK_CHECK(vkCreateWin32SurfaceKHR(_r_vk_state.instance, &surface_info, 0, &_r_vk_state.swapchain.surface));
#endif // IGNIS_PLATFORM_WIN32

#if IGNIS_PLATFORM_LINUX_WAYLAND
  VkWaylandSurfaceCreateInfoKHR surface_info = {
    .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
    .display = window->handle->display,
    .surface = window->handle->surface
  };

  VK_CHECK(vkCreateWaylandSurfaceKHR(_r_vk_state.instance, &surface_info, 0, &_r_vk_state.swapchain.surface));
#endif // IGNIS_PLATFORM_LINUX

#if IGNIS_PLATFORM_LINUX_X11
	VkXlibSurfaceCreateInfoKHR surface_info = {
    .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
		.dpy = window->handle->display,
		.window = window->handle->window,
	};
	VK_CHECK(vkCreateXlibSurfaceKHR(_r_vk_state.instance, &surface_info, 0, &_r_vk_state.swapchain.surface));
#endif // IGNIS_PLATFORM_LINUX_X11

  R_VK_Swapchain swapchain = {0};
  
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
    swapchain.size.w = capabilities.currentExtent.width;
    swapchain.size.h = capabilities.currentExtent.height;
  }

  VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;

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
    .imageExtent = {.width = swapchain.size.w, .height = swapchain.size.h},
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
	swapchain.window = window;

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

	swapchain.images_texture = (R_VK_Texture*)PushArena(swapchain.image_arena, sizeof(R_VK_Texture)*swapchain.image_count);
	for (I32 i = 0; i < swapchain.image_count; i += 1)
	{
		swapchain.images_texture[i].image = swapchain.images[i];
		swapchain.images_texture[i].view = swapchain.image_views[i];
	}

  {
    VkImageCreateInfo image_info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = VK_FORMAT_D32_SFLOAT,
      .extent.width = swapchain.size.w,
			.extent.height = swapchain.size.h,
			.extent.depth = 1,
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
R_VK_DestroySwapchain(void)
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

func R_TextureTest*
R_VK_AcquireSwapchainTexture(R_CommandBuffer* command_buffer)
{
	R_VK_CommandBuffer* vk_command_buffer = (R_VK_CommandBuffer*)command_buffer;

  vkWaitForFences(_r_vk_state.device.logical, 1, vk_command_buffer->submit_fence + _r_vk_state.current_frame, VK_TRUE, U64_MAX);

	while(1)
	{
		VkResult acquire_result = vkAcquireNextImageKHR(
			_r_vk_state.device.logical, _r_vk_state.swapchain.handle, U64_MAX,
			vk_command_buffer->acquire_semaphore[_r_vk_state.current_frame], 0,
			&_r_vk_state.current_target
		);

		if (acquire_result == VK_SUCCESS||acquire_result == VK_SUBOPTIMAL_KHR)
		{
			break;
		}

		R_VK_RecreateSwapchain(_r_vk_state.swapchain.window);
	}

	vkResetFences(_r_vk_state.device.logical, 1, &vk_command_buffer->submit_fence[_r_vk_state.current_frame]);

  return (R_TextureTest*)(_r_vk_state.swapchain.images_texture + _r_vk_state.current_target);
}

// --------------------------------------------------
// Pipeline
func R_GraphicsPipeline*
R_VK_CreateGraphicsPipeline(R_GraphicsPipelineCreateInfo* pipeline_info)
{
	R_VK_GraphicsPipeline* pipeline = _r_vk_state.graphics_pipelines + _r_vk_state.graphics_pipelines_count;

	// AlNov: @TODO Create Set Layouts based on pipeline_info 
	VkDescriptorSetLayoutBinding binding = {
		.binding = 0, // @TODO Add more bindings
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		.pImmutableSamplers = 0,
	};

	VkDescriptorSetLayoutCreateInfo set_layout_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.flags = 0,
		.bindingCount = 1,
		.pBindings = &binding,
	};
	VK_CHECK(vkCreateDescriptorSetLayout(_r_vk_state.device.logical, &set_layout_info, 0, &pipeline->vertex_shader_set_layout));

  VkPipelineLayoutCreateInfo layout_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount = 1,
    .pSetLayouts = &pipeline->vertex_shader_set_layout,
  };
  VK_CHECK(vkCreatePipelineLayout(_r_vk_state.device.logical, &layout_info, 0, &pipeline->layout));

	U32 stride = 0;
  VkVertexInputAttributeDescription attribute_descriptions[R_MAX_VERTEX_ATTRIBUTES];
  for (U32 i = 0; i < pipeline_info->vertex_attributes_count; i += 1)
  {
		R_VertexAttribute* vertex_attribute = pipeline_info->vertex_attributes + i;

		attribute_descriptions[i].location = vertex_attribute->location,
		attribute_descriptions[i].binding = 0,
		attribute_descriptions[i].format = R_VK_GetVkFormatAttribute(vertex_attribute->format),
		attribute_descriptions[i].offset = vertex_attribute->offset,

		stride += GetSizeOfVertexAttributeFormat(vertex_attribute->format);
  }
	LOG_DEBUG("STRIDE: %d\n", stride);

  VkVertexInputBindingDescription binding_description = {
    .binding = 0,
    .stride = stride,
    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
  };
  
  VkPipelineVertexInputStateCreateInfo vertex_input = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .vertexBindingDescriptionCount = 1,
    .pVertexBindingDescriptions = &binding_description,
    .vertexAttributeDescriptionCount = pipeline_info->vertex_attributes_count,
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
    .cullMode = 0,
    .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    .depthBiasEnable = false,
    .lineWidth = 1.0f
  };

  VkDynamicState dynamic_states[] = {
    // VK_DYNAMIC_STATE_VIEWPORT,
    // VK_DYNAMIC_STATE_SCISSOR
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

	VkViewport viewport = {
		.x = 0,
		.y = 0,
		.width = (F32)_r_vk_state.swapchain.size.x,
		.height = (F32)_r_vk_state.swapchain.size.y,
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};

	VkRect2D scissor = {
		.offset.x = 0,
		.offset.y = 0,
		.extent.width = _r_vk_state.swapchain.size.x,
		.extent.height = _r_vk_state.swapchain.size.y,
	};

  VkPipelineViewportStateCreateInfo viewport_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .viewportCount = 1,
		.pViewports = &viewport,
		.scissorCount = 1,
		.pScissors = &scissor,
  };

  VkPipelineDepthStencilStateCreateInfo depth_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
    .depthTestEnable = VK_FALSE,
    .depthWriteEnable = VK_FALSE,
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
      .codeSize = pipeline_info->vertex_shader.code_size,
      .pCode = (U32*)pipeline_info->vertex_shader.code
    };
    VK_CHECK(vkCreateShaderModule(_r_vk_state.device.logical, &module_info, 0, &vertex_module));
  }
  VkPipelineShaderStageCreateInfo vertex_shader = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .stage = VK_SHADER_STAGE_VERTEX_BIT,
    .module = vertex_module,
    .pName = "main",
  };
  
  VkShaderModule fragment_module;
  {
    VkShaderModuleCreateInfo module_info = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = pipeline_info->fragment_shader.code_size,
      .pCode = (U32*)pipeline_info->fragment_shader.code
    };
    VK_CHECK(vkCreateShaderModule(_r_vk_state.device.logical, &module_info, 0, &fragment_module));
  }
  VkPipelineShaderStageCreateInfo fragment_shader = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
    .module = fragment_module,
    .pName = "main",
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

  VkGraphicsPipelineCreateInfo vk_pipeline_info = {
    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .pNext = &rendering_info,
    .stageCount = CountArrayElements(shaders),
    .pStages = shaders,
    .pVertexInputState = &vertex_input,
    .pInputAssemblyState = &input_assembly,
    .pViewportState = &viewport_state,
    .pRasterizationState = &rasterization,
    .pMultisampleState = &multisample,
    .pDepthStencilState = &depth_state,
    .pColorBlendState = &blend,
    .pDynamicState = &dynamic,
    .layout = pipeline->layout,
    .renderPass = 0,
    .subpass = 0,
  };
  VK_CHECK(vkCreateGraphicsPipelines(_r_vk_state.device.logical,
                                     0, 1, &vk_pipeline_info, 0,
                                     &pipeline->handle));


  _r_vk_state.graphics_pipelines_count += 1;

	return (R_GraphicsPipeline*)pipeline;
}

func void
R_VK_BindGraphicsPipeline(R_CommandBuffer* command_buffer, R_GraphicsPipeline* pipeline)
{
	R_VK_CommandBuffer* vk_command_buffer = (R_VK_CommandBuffer*)command_buffer;
	R_VK_GraphicsPipeline* vk_pipeline = (R_VK_GraphicsPipeline*)pipeline;

	vkCmdBindPipeline(vk_command_buffer->handle[_r_vk_state.current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline->handle);
	vk_command_buffer->binded_graphics_pipeline = vk_pipeline;
}

// --------------------------------------------------
// Render Pass
func R_RenderPass*
R_VK_BeginRenderPass(R_CommandBuffer* command_buffer, R_ColorAttachment* color_attachment)
{
	R_VK_CommandBuffer* vk_command_buffer = (R_VK_CommandBuffer*)command_buffer;
	R_VK_Texture* vk_attachment_texture = (R_VK_Texture*)color_attachment->texture;

  VkClearValue clear_value = {0};
  clear_value.color.float32[0] = color_attachment->clear_color.r;
  clear_value.color.float32[1] = color_attachment->clear_color.g;
  clear_value.color.float32[2] = color_attachment->clear_color.b;
  clear_value.color.float32[3] = color_attachment->clear_color.a;

  VkRenderingAttachmentInfo vk_color_attachment_info = {
    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
    .imageView = vk_attachment_texture->view,
    .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    .loadOp = R_VK_GetVkAttachmentLoadOperation(color_attachment->load_operation),
    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    .clearValue = clear_value
  };

  VkExtent2D render_area = {
    .width = _r_vk_state.swapchain.size.w,
    .height = _r_vk_state.swapchain.size.h,
  };
  VkRenderingInfo rendering_info = {
    .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
    .renderArea = {
      .offset = { 0, 0 },
      .extent = render_area
    },
    .layerCount = 1,
    .colorAttachmentCount = 1,
    .pColorAttachments = &vk_color_attachment_info,
  };
  
  R_VK_TransitImageLayout(
    vk_command_buffer->handle[_r_vk_state.current_frame],
		vk_attachment_texture->image,
    VK_IMAGE_LAYOUT_UNDEFINED,
    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    0,
    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    VK_IMAGE_ASPECT_COLOR_BIT
  );

  vkCmdBeginRendering(vk_command_buffer->handle[_r_vk_state.current_frame], &rendering_info);

	return 0; // @TODO
}

func void
R_VK_EndRenderPass(R_CommandBuffer* command_buffer, R_RenderPass* render_pass)
{
	R_VK_CommandBuffer* vk_command_buffer = (R_VK_CommandBuffer*)command_buffer;

  vkCmdEndRendering(vk_command_buffer->handle[_r_vk_state.current_frame]);

  R_VK_TransitImageLayout(
    vk_command_buffer->handle[_r_vk_state.current_frame],
    _r_vk_state.swapchain.images[_r_vk_state.current_target],
    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    0,
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
    VK_IMAGE_ASPECT_COLOR_BIT
  );
}

// -------------------------------------------------------------------
// Draw
func void
R_VK_DrawPrimitives(R_CommandBuffer* command_buffer, U32 vertex_count, U32 instance_count, U32 first_vertex, U32 first_instance)
{
	R_VK_CommandBuffer* vk_command_buffer = (R_VK_CommandBuffer*)command_buffer;

	vkCmdDraw(vk_command_buffer->handle[_r_vk_state.current_frame], vertex_count, instance_count, first_vertex, first_instance);
}

func void
R_VK_DrawIndexedPrimitives(R_CommandBuffer* command_buffer, U32 index_count, U32 instance_count, U32 first_index, I32 vertex_offset, U32 first_instance)
{
	R_VK_CommandBuffer* vk_command_buffer = (R_VK_CommandBuffer*)command_buffer;

	vkCmdDrawIndexed(vk_command_buffer->handle[_r_vk_state.current_frame], index_count, instance_count, first_index, vertex_offset, first_instance);
}

// -------------------------------------------------------------------
// Texture
func R_Texture
R_VK_CreateTexture(Str8 path)
{
  R_Texture texture = {0};

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

  VkImageCreateInfo image_info = {0};
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

  VkMemoryRequirements mem_requirements = {0};
  vkGetImageMemoryRequirements(_r_vk_state.device.logical, _r_vk_state.default_texture.image, &mem_requirements);

  VkMemoryAllocateInfo mem_info = {0};
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
    
    VkBufferImageCopy copy_info = {0};
    copy_info.bufferOffset                    = 0;
    copy_info.bufferRowLength                 = 0;
    copy_info.bufferImageHeight               = 0;
    copy_info.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_info.imageSubresource.mipLevel       = 0;
    copy_info.imageSubresource.baseArrayLayer = 0;
    copy_info.imageSubresource.layerCount     = 1;
		copy_info.imageOffset.x = 0;
		copy_info.imageOffset.y = 0;
		copy_info.imageOffset.z = 0;
    copy_info.imageExtent.width = (U32)tex_width;
		copy_info.imageExtent.height = (U32)tex_height;
		copy_info.imageExtent.depth = 1;

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
  VkImageViewCreateInfo view_info = {0};
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
  VkSamplerCreateInfo sampler_info = {0};
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

// -------------------------------------------------------------------
// Command Buffer
func VkCommandBuffer
R_VK_BeginSingleCmd(void)
{
  VkCommandBufferAllocateInfo allocate_info = {0};
  allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocate_info.commandPool = _r_vk_state.command_pool;
  allocate_info.commandBufferCount = 1;

  VkCommandBuffer command_buffer;
  VK_CHECK(vkAllocateCommandBuffers(_r_vk_state.device.logical, &allocate_info, &command_buffer));

  VkCommandBufferBeginInfo begin_info = {0};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  VK_CHECK(vkBeginCommandBuffer(command_buffer, &begin_info));

  return command_buffer; 
}

func void
R_VK_EndSingleCmd(VkCommandBuffer cmd)
{
  VK_CHECK(vkEndCommandBuffer(cmd));

  VkSubmitInfo submit_info = {0};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &cmd;

  VK_CHECK(vkQueueSubmit(_r_vk_state.device.graphics_queue, 1, &submit_info, 0));
  VK_CHECK(vkQueueWaitIdle(_r_vk_state.device.graphics_queue));

  vkFreeCommandBuffers(_r_vk_state.device.logical, _r_vk_state.command_pool, 1, &cmd);
}


// --------------------------------------------------
// Global State
func B32
R_VK_Init(OS_Window* window)
{
  _r_vk_state.arena = AllocateArena(Megabytes(64));

  VkApplicationInfo app_info = {0};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "VulkanRenderingFramework";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "RenderingEngine";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_API_VERSION_1_3;

  const char* extension_names[] = {
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
    "VK_KHR_surface",
  #if IGNIS_PLATFORM_WIN32
    "VK_KHR_win32_surface",
  #endif // IGNIS_PLATFORM_WIN32
  #if IGNIS_PLATFORM_LINUX_WAYLAND
    "VK_KHR_wayland_surface",
  #endif // IGNIS_PLATFORM_LINUX
  #if IGNIS_PLATFORM_LINUX_X11
    "VK_KHR_xlib_surface",
  #endif // IGNIS_PLATFORM_LINUX
  };

#if IGNIS_DEBUG
  const char* validation_layers[] = {
    "VK_LAYER_KHRONOS_validation",
  };
#else
  const char* validation_layers[] = {0};
#endif // IGNIS_DEBUG

  VkInstanceCreateInfo instance_info = {0};
  instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_info.pApplicationInfo = &app_info;
  instance_info.enabledLayerCount = CountArrayElements(validation_layers);
  instance_info.ppEnabledLayerNames = validation_layers;
  instance_info.enabledExtensionCount = CountArrayElements(extension_names);
  instance_info.ppEnabledExtensionNames = extension_names;

#if IGNIS_DEBUG
  VkDebugUtilsMessengerCreateInfoEXT messenger_info = R_VK_PopulateDebugMessengerCreateInfo();
  instance_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&messenger_info;
#endif // IGNIS_DEBUG

  VkResult result = (vkCreateInstance(&instance_info, 0, &_r_vk_state.instance));

#if IGNIS_DEBUG
  VK_CHECK(R_VK_CreateDebugMessenger(_r_vk_state.instance, &_r_vk_state.debug_messenger));
#endif // IGNIS_DEBUG

  R_VK_CreateDevice();
  R_VK_CreateSwapchain(window);

  VkCommandPoolCreateInfo command_pool_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
    .queueFamilyIndex = _r_vk_state.device.graphics_queue_index
  };
  VK_CHECK(vkCreateCommandPool(_r_vk_state.device.logical, &command_pool_info, 0, &_r_vk_state.command_pool));

	return 0;
}

func B32
R_VK_Shutdown(void)
{
	return 0;
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

func VkDebugUtilsMessengerCreateInfoEXT
R_VK_PopulateDebugMessengerCreateInfo(void)
{
	VkDebugUtilsMessengerCreateInfoEXT messenger_info = {0};
  messenger_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  messenger_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  messenger_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
  messenger_info.pfnUserCallback = R_VK_DebugCallback;
  messenger_info.pUserData = 0;

	return messenger_info;
}

func VkResult
R_VK_CreateDebugUtilsMessenger(VkInstance instance, VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMesseneger)
{
  PFN_vkCreateDebugUtilsMessengerEXT f = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

  if (f != 0)
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
  PFN_vkDestroyDebugUtilsMessengerEXT f = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

  if (f != 0)
  {
    f(instance, debugMessenger, pAllocator);
  }
}

func VkResult
R_VK_CreateDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT* debugMessenger)
{
  VkDebugUtilsMessengerCreateInfoEXT messengerInfo = R_VK_PopulateDebugMessengerCreateInfo();

  return R_VK_CreateDebugUtilsMessenger(instance, &messengerInfo, 0, debugMessenger);
}
#endif // IGNIS_DEBUG
