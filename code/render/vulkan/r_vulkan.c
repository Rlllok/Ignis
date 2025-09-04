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
  if((flags & R_BUFFER_USAGE_FLAG_TRANSFER) == R_BUFFER_USAGE_FLAG_TRANSFER)
  {
    result |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    result |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
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

func R_VK_Buffer*
R_VK_BufferFromHandle(R_Buffer buffer)
{
  return R_VK_BufferArrayGetPointer(&_r_vk_state.buffers, buffer);
}

func R_Buffer
R_VK_CreateBuffer(U32 capacity, R_BufferUsageFlags usage_flags, R_BufferPropertyFlags property_flags)
{
  // @NOTE This is to create Vulkan Buffer and Memory
  R_VK_Buffer buffer = {0};
	buffer.capacity = capacity;
  
  VkBufferCreateInfo buffer_info = {
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .size = capacity,
    .usage = _VkFromBufferUsageFlags(usage_flags),
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE
  };

  VK_CHECK(vkCreateBuffer(_r_vk_state.device.logical, &buffer_info, 0, &buffer.handle));

  VkMemoryRequirements memory_requirements;
  vkGetBufferMemoryRequirements(_r_vk_state.device.logical, buffer.handle, &memory_requirements);
  
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
  VK_CHECK(vkAllocateMemory(_r_vk_state.device.logical, &allocation_info , 0, &buffer.memory));

  VK_CHECK(vkBindBufferMemory(_r_vk_state.device.logical, buffer.handle, buffer.memory, 0));

  vkMapMemory(_r_vk_state.device.logical, buffer.memory, 0, VK_WHOLE_SIZE, 0, &buffer.mapped);

  return R_VK_BufferArrayAdd(&_r_vk_state.buffers, buffer);
}

func void
R_VK_DestroyBuffer(R_Buffer buffer)
{
  R_VK_Buffer* vk_buffer = R_VK_BufferFromHandle(buffer);

  vkDeviceWaitIdle(_r_vk_state.device.logical);
  vkUnmapMemory(_r_vk_state.device.logical, vk_buffer->memory);
  vkFreeMemory(_r_vk_state.device.logical, vk_buffer->memory, 0);
  vkDestroyBuffer(_r_vk_state.device.logical, vk_buffer->handle, 0);

  *vk_buffer = (R_VK_Buffer){0};
}

func U64 R_VK_PushBuffer(R_Buffer buffer, U8* data, U64 size)
{
  R_VK_Buffer* vk_buffer = R_VK_BufferFromHandle(buffer);

	Assert((vk_buffer->size + size) < vk_buffer->capacity);
	U32 offset = vk_buffer->size;

	memcpy((U8*)vk_buffer->mapped + vk_buffer->size, data, size);
	vk_buffer->size += size;
	U64 padding = 64 - (vk_buffer->size + 64)%64;
	vk_buffer->size += padding;

	return offset;
}

func void
R_VK_ResetBuffer(R_Buffer buffer)
{
  R_VK_Buffer* vk_buffer = R_VK_BufferFromHandle(buffer);
	vk_buffer->size = 0;
}
func void
R_VK_BindIndexBuffer(R_CommandBuffer command_buffer, R_Buffer buffer, U64 offset, R_IndexSize index_size)
{
	R_VK_CommandBuffer* vk_command_buffer = R_VK_CommandBufferFromHandle(command_buffer);
  R_VK_Buffer* vk_buffer = R_VK_BufferFromHandle(buffer);

	vkCmdBindIndexBuffer(
			vk_command_buffer->handle[_r_vk_state.current_frame],
			vk_buffer->handle,
			offset, R_VK_GetVkIndexTypeFrom(index_size));
}

func void
R_VK_BindVertexBuffer(R_CommandBuffer command_buffer, R_Buffer buffer, U64 offset)
{
	R_VK_CommandBuffer* vk_command_buffer = R_VK_CommandBufferFromHandle(command_buffer);
  R_VK_Buffer* vk_buffer = R_VK_BufferFromHandle(buffer);

	VkDeviceSize vk_offset = offset;
	vkCmdBindVertexBuffers(vk_command_buffer->handle[_r_vk_state.current_frame], 0, 1, &vk_buffer->handle, &vk_offset);
}

func void
R_VK_BufferGetData(R_Buffer buffer, U64 offset, void* dst, U64 data_size)
{
  R_VK_Buffer* vk_buffer = R_VK_BufferFromHandle(buffer);
  memcpy(dst, (U8*)vk_buffer->mapped + offset, data_size);
}

// -------------------------------------------------------------------
// Command Buffer
func R_VK_CommandBuffer*
R_VK_CommandBufferFromHandle(R_CommandBuffer command_buffer)
{
  return R_VK_CommandBufferArrayGetPointer(&_r_vk_state.command_buffers, command_buffer);
}

func R_CommandBuffer R_VK_GetCommandBuffer(void)
{
	R_VK_CommandBuffer command_buffer = {0};

	VkCommandBufferAllocateInfo allocate_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = _r_vk_state.command_pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = R_FRAMES_IN_FLIGHT
	};
	VK_CHECK(vkAllocateCommandBuffers(_r_vk_state.device.logical, &allocate_info, command_buffer.handle));

	for (I32 i = 0; i < R_FRAMES_IN_FLIGHT; i += 1)
	{
		VkFenceCreateInfo fence_info = {
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT
		};
		VK_CHECK(vkCreateFence(_r_vk_state.device.logical, &fence_info, 0, (command_buffer.submit_fence + i)));
	}

	for (I32 i = 0; i < R_FRAMES_IN_FLIGHT; i += 1)
	{
		VkSemaphoreCreateInfo semaphore_info = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		};
		VK_CHECK(vkCreateSemaphore(_r_vk_state.device.logical, &semaphore_info, 0, (command_buffer.acquire_semaphore + i)));
		VK_CHECK(vkCreateSemaphore(_r_vk_state.device.logical, &semaphore_info, 0, (command_buffer.release_semaphore + i)));
	}

  return R_VK_CommandBufferArrayAdd(&_r_vk_state.command_buffers, command_buffer);
}

func void
R_VK_ReleaseCommandBuffer(R_CommandBuffer command_buffer)
{
  R_VK_CommandBuffer* vk_command_buffer = R_VK_CommandBufferFromHandle(command_buffer);

  vkDeviceWaitIdle(_r_vk_state.device.logical);
  for (I32 i = 0; i < R_FRAMES_IN_FLIGHT; i += 1)
  {
    vkDestroySemaphore(_r_vk_state.device.logical, vk_command_buffer->acquire_semaphore[i], 0);
    vkDestroySemaphore(_r_vk_state.device.logical, vk_command_buffer->release_semaphore[i], 0);
    vkDestroyFence(_r_vk_state.device.logical, vk_command_buffer->submit_fence[i], 0);
    for (I32 j = 0; j < vk_command_buffer->descriptor_pool[i].pool_count; j += 1)
    {
      vkDestroyDescriptorPool(_r_vk_state.device.logical, vk_command_buffer->descriptor_pool[i].vk_pools[j], 0);
    }
  }
}

func void R_VK_BeginCommandBuffer(R_CommandBuffer command_buffer)
{
  R_VK_CommandBuffer* vk_command_buffer = R_VK_CommandBufferFromHandle(command_buffer);

	vkResetCommandBuffer(vk_command_buffer->handle[_r_vk_state.current_frame], 0);
	for (I32 i = 0; i < vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].pool_count; i += 1)
	{
		vkResetDescriptorPool(_r_vk_state.device.logical, vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].vk_pools[i], 0);
	}

	vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].sets_count = 0;

  VkCommandBufferBeginInfo begin_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
  }; 
  VK_CHECK(vkBeginCommandBuffer(vk_command_buffer->handle[_r_vk_state.current_frame], &begin_info));
}

func void
R_VK_SubmitCommandBuffer(R_CommandBuffer command_buffer)
{
  R_VK_CommandBuffer* vk_command_buffer = R_VK_CommandBufferFromHandle(command_buffer);

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

	_r_vk_state.current_frame = (_r_vk_state.current_frame + 1)%R_FRAMES_IN_FLIGHT;
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

      VkPhysicalDeviceFeatures enabled_features = {
        .independentBlend = 1,
      };

      VkDeviceCreateInfo device_info = {0};
      device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
      device_info.queueCreateInfoCount = 1;
      device_info.pQueueCreateInfos = &queue_info;
      device_info.enabledExtensionCount = CountArrayElements(required_extensions);
      device_info.ppEnabledExtensionNames = required_extensions;
      device_info.pEnabledFeatures = &enabled_features;
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
  
  Arena* tmp_arena = AllocateArena(Kilobytes(64));
  {
    U32 format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(_r_vk_state.device.physical, _r_vk_state.swapchain.surface, &format_count, 0);
    VkSurfaceFormatKHR* formats = (VkSurfaceFormatKHR*)PushArena(tmp_arena, format_count * sizeof(VkSurfaceFormatKHR));
    vkGetPhysicalDeviceSurfaceFormatsKHR(_r_vk_state.device.physical, _r_vk_state.swapchain.surface, &format_count, formats);

    for (U32 i = 0; i < format_count; i += 1)
    {
      if (formats[i].format == VK_FORMAT_B8G8R8A8_UNORM && formats[i].colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
        _r_vk_state.swapchain.surface_format = formats[i];
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
    _r_vk_state.swapchain.size = window->size;
  }
  else
  {
    _r_vk_state.swapchain.size.w = capabilities.currentExtent.width;
    _r_vk_state.swapchain.size.h = capabilities.currentExtent.height;
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
    .imageFormat = _r_vk_state.swapchain.surface_format.format,
    .imageColorSpace = _r_vk_state.swapchain.surface_format.colorSpace,
    .imageExtent = {.width = _r_vk_state.swapchain.size.w, .height = _r_vk_state.swapchain.size.h},
    .imageArrayLayers = 1,
    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .preTransform = capabilities.currentTransform,
    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .presentMode = present_mode,
    .clipped = true,
    .oldSwapchain = 0
  };

  VK_CHECK(vkCreateSwapchainKHR(_r_vk_state.device.logical, &swapchain_info, 0, &_r_vk_state.swapchain.handle));
	_r_vk_state.swapchain.window = window;

  VK_CHECK(vkGetSwapchainImagesKHR(_r_vk_state.device.logical, _r_vk_state.swapchain.handle, &_r_vk_state.swapchain.image_count, 0));
  _r_vk_state.swapchain.image_arena = AllocateArena(Megabytes(8));
  VkImage* images = (VkImage*)PushArena(_r_vk_state.swapchain.image_arena, _r_vk_state.swapchain.image_count * sizeof(VkImage));
  VK_CHECK(vkGetSwapchainImagesKHR(_r_vk_state.device.logical, _r_vk_state.swapchain.handle, &_r_vk_state.swapchain.image_count, images));

  _r_vk_state.swapchain.frame_resources = (FrameResources*)PushArena(_r_vk_state.swapchain.image_arena, _r_vk_state.swapchain.image_count * sizeof(FrameResources));
  _r_vk_state.swapchain.textures = (R_Texture*)PushArena(_r_vk_state.swapchain.image_arena, _r_vk_state.swapchain.image_count*sizeof(R_Texture));
  for (U32 i = 0; i < _r_vk_state.swapchain.image_count; i += 1)
  {
    VkImageView image_view = {0};

    VkImageViewCreateInfo view_info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = images[i],
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = _r_vk_state.swapchain.surface_format.format,
      .subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1
      }
    };

    VK_CHECK(vkCreateImageView(_r_vk_state.device.logical, &view_info, 0, &image_view));
    
    _r_vk_state.swapchain.frame_resources[i] = R_VK_CreateFrameResources();

    R_VK_Texture vk_texture = {
      .image = images[i],
      .view = image_view,
      .aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT,
      .from_swapchain = 1,
    };

    _r_vk_state.swapchain.textures[i] = R_VK_TextureArrayAdd(&_r_vk_state.textures, vk_texture);
  }
}

func void
R_VK_DestroySwapchain(void)
{
  vkDeviceWaitIdle(_r_vk_state.device.logical);

  for (I32 i = 0; i < _r_vk_state.swapchain.image_count; i += 1)
  {
    R_VK_DestroyFrameResources(&_r_vk_state.swapchain.frame_resources[i]);
    R_VK_DestroyTexture(_r_vk_state.swapchain.textures[i]);
  }
  vkDestroySwapchainKHR(_r_vk_state.device.logical, _r_vk_state.swapchain.handle, 0);
  vkDestroySurfaceKHR(_r_vk_state.instance, _r_vk_state.swapchain.surface, 0);

  FreeArena(_r_vk_state.swapchain.image_arena);
}

func void
R_VK_RecreateSwapchain(OS_Window* window)
{
  LOG_INFO("Recreate Swapchain\n");
  R_VK_DestroySwapchain();
  R_VK_CreateSwapchain(window);
}

func R_TextureFormat
R_VK_GetSwapchainTextureFormat()
{
  return R_VK_TextureFormatFromVkFormat(_r_vk_state.swapchain.surface_format.format);
}

func R_Texture
R_VK_AcquireSwapchainTexture(R_CommandBuffer command_buffer)
{
	R_VK_CommandBuffer* vk_command_buffer = R_VK_CommandBufferFromHandle(command_buffer);

  vkWaitForFences(_r_vk_state.device.logical, 1, vk_command_buffer->submit_fence + _r_vk_state.current_frame, VK_TRUE, U64_MAX);

	while(1)
	{
		VkResult acquire_result = vkAcquireNextImageKHR(
			_r_vk_state.device.logical, _r_vk_state.swapchain.handle, U64_MAX,
			vk_command_buffer->acquire_semaphore[_r_vk_state.current_frame], 0,
			&_r_vk_state.current_target
		);

		if (acquire_result == VK_SUCCESS)
		{
			break;
		}
		else if (acquire_result == VK_SUBOPTIMAL_KHR || acquire_result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			R_VK_RecreateSwapchain(_r_vk_state.swapchain.window);
		}
		else if (acquire_result == VK_NOT_READY || acquire_result == VK_TIMEOUT)
		{
			continue;
		}
		else
		{
			Assert(acquire_result == VK_SUCCESS);
		}
	}

	vkResetFences(_r_vk_state.device.logical, 1, &vk_command_buffer->submit_fence[_r_vk_state.current_frame]);

  return _r_vk_state.swapchain.textures[_r_vk_state.current_target];
}

// -------------------------------------------------------------------
// Descriptor Sets
#if 0
func void
R_VK_BindGlobalVertexShaderData(R_CommandBuffer command_buffer, R_UniformBufferBindingInfo uniform_info)
{
	R_VK_CommandBuffer* vk_command_buffer = R_VK_CommandBufferFromHandle(command_buffer);

	U32 num_sets = vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].pool_count*R_VK_SETS_PER_POOL;
	if (num_sets <= vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].sets_count)
	{
		Assert(vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].pool_count >= R_VK_MAX_POOL_COUNT);

		VkDescriptorPoolSize uniform_pool_size = {
			.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = R_VK_MAX_UNIFORM_BUFFERS_PER_SET*R_VK_SETS_PER_POOL,
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

		vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].pool_count += 1;
	}

	I32 pool_id = (I32)(vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].sets_count/R_VK_SETS_PER_POOL);
	I32 set_id = vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].sets_count%R_VK_SETS_PER_POOL;

	VkDescriptorSetAllocateInfo sets_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].vk_pools[pool_id],
		.descriptorSetCount = 1, // --AlNov: @TODO Only Global Set for now
		.pSetLayouts = &vk_command_buffer->binded_graphics_pipeline->vertex_global_set_layout,
	};
	VkResult allocate_result = vkAllocateDescriptorSets(_r_vk_state.device.logical, &sets_info, &vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].vk_sets[pool_id][set_id]);

	R_VK_Buffer* vk_buffer = R_VK_BufferFromHandle(uniform_info.buffer);
	VkDescriptorBufferInfo buffer_info = {
		.buffer = vk_buffer->handle,
		.offset = uniform_info.offset,
		.range = uniform_info.size,
	};

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
			R_VK_VERTEX_SHADER_GLOBAL_UNIFORM_SET_SLOT, 1, &vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].vk_sets[pool_id][set_id], 0, 0);

	vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].sets_count += 1;
}

func void
R_VK_BindInstanceVertexShaderData(R_CommandBuffer command_buffer, R_UniformBufferBindingInfo uniform_info)
{
	R_VK_CommandBuffer* vk_command_buffer = R_VK_CommandBufferFromHandle(command_buffer);

	U32 num_sets = vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].pool_count*R_VK_SETS_PER_POOL;
	if (num_sets <= vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].sets_count)
	{
		Assert(vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].pool_count >= R_VK_MAX_POOL_COUNT);

		VkDescriptorPoolSize uniform_pool_size = {
			.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = R_VK_MAX_UNIFORM_BUFFERS_PER_SET*R_VK_SETS_PER_POOL,
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

		vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].pool_count += 1;
	}

	I32 pool_id = (I32)(vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].sets_count/R_VK_SETS_PER_POOL);
	I32 set_id = vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].sets_count%R_VK_SETS_PER_POOL;

	VkDescriptorSetAllocateInfo sets_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].vk_pools[pool_id],
		.descriptorSetCount = 1, // --AlNov: @TODO Only Global Set for now
		.pSetLayouts = &vk_command_buffer->binded_graphics_pipeline->vertex_instance_set_layout,
	};
	VkResult allocate_result = vkAllocateDescriptorSets(_r_vk_state.device.logical, &sets_info, &vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].vk_sets[pool_id][set_id]);

	R_VK_Buffer* vk_buffer = R_VK_BufferFromHandle(uniform_info.buffer);
	VkDescriptorBufferInfo buffer_info = {
		.buffer = vk_buffer->handle,
		.offset = uniform_info.offset,
		.range = uniform_info.size,
	};

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

			R_VK_VERTEX_SHADER_INSTANCE_UNIFORM_SET_SLOT, 1, &vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].vk_sets[pool_id][set_id], 0, 0);

	vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].sets_count += 1;
}
#endif

func void
R_VK_BindGlobalShaderData(R_CommandBuffer command_buffer, R_ShaderType shader_type, I32 uniform_buffers_count, R_UniformBufferBindingInfo* uniform_info, I32 samplers_count, R_SamplerBindingInfo* sampler_infos)
{
	R_VK_CommandBuffer* vk_command_buffer = R_VK_CommandBufferFromHandle(command_buffer);

	U32 num_sets = vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].pool_count*R_VK_SETS_PER_POOL;
	if (num_sets <= vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].sets_count)
	{
		Assert(vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].pool_count < R_VK_MAX_POOL_COUNT);

		VkDescriptorPoolSize pool_sizes[] = {
      {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = R_VK_MAX_UNIFORM_BUFFERS_PER_SET*R_VK_SETS_PER_POOL,
      },
      {
        .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = R_VK_MAX_SAMPLERS_PER_SET*R_VK_SETS_PER_POOL,
      }
		};

		VkDescriptorPoolCreateInfo pool_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.flags = 0,
			.maxSets = R_VK_SETS_PER_POOL,
			.poolSizeCount = CountArrayElements(pool_sizes),
			.pPoolSizes = pool_sizes,
		};
		VK_CHECK(vkCreateDescriptorPool(_r_vk_state.device.logical, &pool_info, 0, vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].vk_pools + vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].pool_count));

		vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].pool_count += 1;
	}

	I32 pool_id = (I32)(vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].sets_count/R_VK_SETS_PER_POOL);
	I32 set_id = vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].sets_count%R_VK_SETS_PER_POOL;

  I32 set_slot = 0;
  VkDescriptorSetLayout set_layout = VK_NULL_HANDLE;
  if (shader_type == R_SHADER_TYPE_VERTEX)
  {
    set_slot = R_VK_VERTEX_SHADER_GLOBAL_UNIFORM_SET_SLOT;
		set_layout = vk_command_buffer->binded_graphics_pipeline->vertex_global_set_layout;
  }
  else if (shader_type == R_SHADER_TYPE_FRAGMENT)
  {
    set_slot = R_VK_FRAGMENT_SHADER_GLOBAL_UNIFORM_SET_SLOT;
		set_layout = vk_command_buffer->binded_graphics_pipeline->fragment_global_set_layout;
  }

	VkDescriptorSetAllocateInfo sets_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].vk_pools[pool_id],
		.descriptorSetCount = 1, // --AlNov: @TODO Only Global Set for now
		.pSetLayouts = &set_layout,
	};
	VK_CHECK(vkAllocateDescriptorSets(_r_vk_state.device.logical, &sets_info, &vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].vk_sets[pool_id][set_id]));

  VkWriteDescriptorSet write_infos[R_VK_MAX_UNIFORM_BUFFERS_PER_SET+R_VK_MAX_SAMPLERS_PER_SET] = {0};
  I32 writes_count = 0;

  for (I32 i = 0; i < uniform_buffers_count; i += 1)
  {
    R_VK_Buffer* vk_buffer = R_VK_BufferFromHandle(uniform_info->buffer);
    VkDescriptorBufferInfo buffer_info = {
      .buffer = vk_buffer->handle,
      .offset = uniform_info->offset,
      .range = uniform_info->size,
    };

    VkWriteDescriptorSet write_info = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].vk_sets[pool_id][set_id],
      .dstBinding = i, // @TODO Add more bindings
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .pBufferInfo = &buffer_info,
    };
    write_infos[writes_count] = write_info;

    writes_count += 1;
  }

  for (I32 i = uniform_buffers_count; i < uniform_buffers_count + samplers_count; i += 1)
  {
    I32 index = i - uniform_buffers_count;
    R_VK_TextureSampler* vk_sampler = R_VK_TextureSamplerFromHandle(sampler_infos[index].sampler);
    R_VK_Texture* vk_texture = R_VK_TextureFromHandle(sampler_infos[index].texture);

    VkDescriptorImageInfo image_info = 
    {
      .sampler = vk_sampler->handle,
      .imageView = vk_texture->view,
      .imageLayout = vk_texture->layout,
    };

    VkWriteDescriptorSet write_info = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].vk_sets[pool_id][set_id],
      .dstBinding = i, // @TODO Add more bindings
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .pImageInfo = &image_info,
    };
    write_infos[writes_count] = write_info;

    writes_count += 1;
  }
	vkUpdateDescriptorSets(_r_vk_state.device.logical, writes_count, write_infos, 0, 0);

	vkCmdBindDescriptorSets( 
			vk_command_buffer->handle[_r_vk_state.current_frame],
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			vk_command_buffer->binded_graphics_pipeline->layout,
			set_slot, 1, &vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].vk_sets[pool_id][set_id], 0, 0);

	vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].sets_count += 1;
}

func void
R_VK_BindInstanceShaderData(R_CommandBuffer command_buffer, R_ShaderType shader_type, I32 uniform_buffers_count, R_UniformBufferBindingInfo* uniform_info, I32 samplers_count, R_SamplerBindingInfo* sampler_infos)
{
	R_VK_CommandBuffer* vk_command_buffer = R_VK_CommandBufferFromHandle(command_buffer);

	U32 num_sets = vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].pool_count*R_VK_SETS_PER_POOL;
	if (num_sets <= vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].sets_count)
	{
		Assert(vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].pool_count < R_VK_MAX_POOL_COUNT);

		VkDescriptorPoolSize pool_sizes[] = {
      {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = R_VK_MAX_UNIFORM_BUFFERS_PER_SET*R_VK_SETS_PER_POOL,
      },
      {
        .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = R_VK_MAX_SAMPLERS_PER_SET*R_VK_SETS_PER_POOL,
      }
		};

		VkDescriptorPoolCreateInfo pool_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.flags = 0,
			.maxSets = R_VK_SETS_PER_POOL,
			.poolSizeCount = CountArrayElements(pool_sizes),
			.pPoolSizes = pool_sizes,
		};
		VK_CHECK(vkCreateDescriptorPool(_r_vk_state.device.logical, &pool_info, 0, vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].vk_pools + vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].pool_count));

		vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].pool_count += 1;
	}

	I32 pool_id = (I32)(vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].sets_count/R_VK_SETS_PER_POOL);
	I32 set_id = vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].sets_count%R_VK_SETS_PER_POOL;

  I32 set_slot = 0;
  VkDescriptorSetLayout set_layout = VK_NULL_HANDLE;
  if (shader_type == R_SHADER_TYPE_VERTEX)
  {
    set_slot = R_VK_VERTEX_SHADER_INSTANCE_UNIFORM_SET_SLOT;
		set_layout = vk_command_buffer->binded_graphics_pipeline->vertex_instance_set_layout;
  }
  else if (shader_type == R_SHADER_TYPE_FRAGMENT)
  {
    set_slot = R_VK_FRAGMENT_SHADER_INSTANCE_UNIFORM_SET_SLOT;
		set_layout = vk_command_buffer->binded_graphics_pipeline->fragment_instance_set_layout;
  }

	VkDescriptorSetAllocateInfo sets_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].vk_pools[pool_id],
		.descriptorSetCount = 1,
		.pSetLayouts = &set_layout,
	};
	VK_CHECK(vkAllocateDescriptorSets(_r_vk_state.device.logical, &sets_info, &vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].vk_sets[pool_id][set_id]));

  VkWriteDescriptorSet write_infos[R_VK_MAX_UNIFORM_BUFFERS_PER_SET+R_VK_MAX_SAMPLERS_PER_SET] = {0};
  I32 writes_count = 0;
  VkDescriptorBufferInfo buffer_infos[R_VK_MAX_UNIFORM_BUFFERS_PER_SET] = {0};
  I32 buffer_infos_count = 0;
  VkDescriptorImageInfo image_infos[R_VK_MAX_SAMPLERS_PER_SET] = {0};
  I32 image_infos_count = 0;

  for (I32 i = 0; i < uniform_buffers_count; i += 1)
  {
    R_VK_Buffer* vk_buffer = R_VK_BufferFromHandle(uniform_info->buffer);
    buffer_infos[buffer_infos_count] = (VkDescriptorBufferInfo){
      .buffer = vk_buffer->handle,
      .offset = uniform_info->offset,
      .range = uniform_info->size,
    };

    VkWriteDescriptorSet write_info = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].vk_sets[pool_id][set_id],
      .dstBinding = i,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .pBufferInfo = &buffer_infos[buffer_infos_count],
    };
    write_infos[writes_count] = write_info;

    writes_count += 1;
    buffer_infos_count += 1;
  }

  for (I32 i = uniform_buffers_count; i < uniform_buffers_count + samplers_count; i += 1)
  {
    I32 index = i - uniform_buffers_count;
    R_VK_TextureSampler* vk_sampler = R_VK_TextureSamplerFromHandle(sampler_infos[index].sampler);
    R_VK_Texture* vk_texture = R_VK_TextureFromHandle(sampler_infos[index].texture);

    VkCommandBuffer single_cmd = R_VK_BeginSingleCmd();
    {
      R_VK_ChangeTextureLayout(single_cmd, vk_texture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
    R_VK_EndSingleCmd(single_cmd);

    image_infos[image_infos_count] = (VkDescriptorImageInfo){
      .sampler = vk_sampler->handle,
      .imageView = vk_texture->view,
      .imageLayout = vk_texture->layout,
    };

    VkWriteDescriptorSet write_info = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].vk_sets[pool_id][set_id],
      .dstBinding = i,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .pImageInfo = &image_infos[image_infos_count],
    };
    write_infos[writes_count] = write_info;

    writes_count += 1;
    image_infos_count += 1;
  }
	vkUpdateDescriptorSets(_r_vk_state.device.logical, writes_count, write_infos, 0, 0);

	vkCmdBindDescriptorSets( 
    vk_command_buffer->handle[_r_vk_state.current_frame],
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    vk_command_buffer->binded_graphics_pipeline->layout,
    set_slot, 1, &vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].vk_sets[pool_id][set_id], 0, 0
  );

	vk_command_buffer->descriptor_pool[_r_vk_state.current_frame].sets_count += 1;
}

// --------------------------------------------------
// Pipeline
func R_VK_GraphicsPipeline*
R_VK_GraphicsPipelineFromHandle(R_GraphicsPipeline pipeline)
{
  return R_VK_GraphicsPipelineArrayGetPointer(&_r_vk_state.graphics_pipelines, pipeline);
}

func R_GraphicsPipeline
R_VK_CreateGraphicsPipeline(R_GraphicsPipelineCreateInfo* pipeline_info)
{
  R_VK_GraphicsPipeline pipeline = {0};
	pipeline.vertex_global_uniforms_count = pipeline_info->vertex_shader.global_uniforms_count;
  pipeline.vertex_global_samplers_count = pipeline_info->vertex_shader.global_samplers_count;
  pipeline.vertex_instance_uniforms_count = pipeline_info->vertex_shader.instance_uniforms_count;
  pipeline.vertex_instance_samplers_count = pipeline_info->vertex_shader.instance_samplers_count;
  pipeline.fragment_global_uniforms_count = pipeline_info->fragment_shader.global_uniforms_count;
  pipeline.fragment_global_samplers_count = pipeline_info->fragment_shader.global_samplers_count;
	pipeline.fragment_instance_uniforms_count = pipeline_info->fragment_shader.instance_uniforms_count;
  pipeline.fragment_instance_samplers_count = pipeline_info->fragment_shader.instance_samplers_count;

	VkDescriptorSetLayout set_layouts[4] = {0};
	I32 set_layouts_count = 0;
	{
		VkDescriptorSetLayoutBinding vertex_global_bindings[R_VK_MAX_UNIFORM_BUFFERS_PER_SET + R_VK_MAX_SAMPLERS_PER_SET] = {0};
		for (I32 i = 0; i < pipeline.vertex_global_uniforms_count; i += 1)
		{
			vertex_global_bindings[i].binding = i; // @TODO Add more bindings
			vertex_global_bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			vertex_global_bindings[i].descriptorCount = 1;
			vertex_global_bindings[i].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
			vertex_global_bindings[i].pImmutableSamplers = 0;
		};
    for (I32 i = pipeline.vertex_global_uniforms_count; i < pipeline.vertex_global_uniforms_count + pipeline.vertex_global_samplers_count; i += 1)
    {
			vertex_global_bindings[i].binding = i; // @TODO Add more bindings
			vertex_global_bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			vertex_global_bindings[i].descriptorCount = 1;
			vertex_global_bindings[i].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
			vertex_global_bindings[i].pImmutableSamplers = 0;
    }

		VkDescriptorSetLayoutCreateInfo vertex_global_set_layout_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.flags = 0,
			.bindingCount = pipeline.vertex_global_uniforms_count + pipeline.vertex_global_samplers_count,
			.pBindings = vertex_global_bindings,
		};
		VK_CHECK(vkCreateDescriptorSetLayout(_r_vk_state.device.logical, &vertex_global_set_layout_info, 0, &pipeline.vertex_global_set_layout));
		set_layouts[set_layouts_count] = pipeline.vertex_global_set_layout;
		set_layouts_count += 1;
	}

	{
		VkDescriptorSetLayoutBinding vertex_instance_bindings[R_VK_MAX_UNIFORM_BUFFERS_PER_SET + R_VK_MAX_SAMPLERS_PER_SET] = {0};
		for (I32 i = 0; i < pipeline.vertex_instance_uniforms_count; i += 1)
		{
			vertex_instance_bindings[i].binding = i; // @TODO Add more bindings
			vertex_instance_bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			vertex_instance_bindings[i].descriptorCount = 1;
			vertex_instance_bindings[i].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
			vertex_instance_bindings[i].pImmutableSamplers = 0;
		};
    for (I32 i = pipeline.vertex_instance_uniforms_count; i < pipeline.vertex_instance_uniforms_count + pipeline.vertex_instance_samplers_count; i += 1)
    {
			vertex_instance_bindings[i].binding = i; // @TODO Add more bindings
			vertex_instance_bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			vertex_instance_bindings[i].descriptorCount = 1;
			vertex_instance_bindings[i].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
			vertex_instance_bindings[i].pImmutableSamplers = 0;
    }

		VkDescriptorSetLayoutCreateInfo vertex_instance_set_layout_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.flags = 0,
			.bindingCount = pipeline.vertex_instance_uniforms_count + pipeline.vertex_instance_samplers_count,
			.pBindings = vertex_instance_bindings,
		};
		VK_CHECK(vkCreateDescriptorSetLayout(_r_vk_state.device.logical, &vertex_instance_set_layout_info, 0, &pipeline.vertex_instance_set_layout));
		set_layouts[set_layouts_count] = pipeline.vertex_instance_set_layout;
		set_layouts_count += 1;
	}

	{
		VkDescriptorSetLayoutBinding fragment_global_bindings[R_VK_MAX_UNIFORM_BUFFERS_PER_SET + R_VK_MAX_SAMPLERS_PER_SET] = {0};
		for (I32 i = 0; i < pipeline.fragment_global_uniforms_count; i += 1)
		{
			fragment_global_bindings[i].binding = i; // @TODO Add more bindings
			fragment_global_bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			fragment_global_bindings[i].descriptorCount = 1;
			fragment_global_bindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragment_global_bindings[i].pImmutableSamplers = 0;
		};
    for (I32 i = pipeline.fragment_global_uniforms_count; i < pipeline.fragment_global_uniforms_count + pipeline.fragment_global_samplers_count; i += 1)
    {
			fragment_global_bindings[i].binding = i; // @TODO Add more bindings
			fragment_global_bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			fragment_global_bindings[i].descriptorCount = 1;
			fragment_global_bindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragment_global_bindings[i].pImmutableSamplers = 0;
    }

		VkDescriptorSetLayoutCreateInfo fragment_global_set_layout_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.flags = 0,
			.bindingCount = pipeline.fragment_global_uniforms_count + pipeline.fragment_global_samplers_count,
			.pBindings = fragment_global_bindings,
		};
		VK_CHECK(vkCreateDescriptorSetLayout(_r_vk_state.device.logical, &fragment_global_set_layout_info, 0, &pipeline.fragment_global_set_layout));
		set_layouts[set_layouts_count] = pipeline.fragment_global_set_layout;
		set_layouts_count += 1;
	}

	{
		VkDescriptorSetLayoutBinding fragment_instance_bindings[R_VK_MAX_UNIFORM_BUFFERS_PER_SET + R_VK_MAX_SAMPLERS_PER_SET] = {0};
		for (I32 i = 0; i < pipeline.fragment_instance_uniforms_count; i += 1)
		{
			fragment_instance_bindings[i].binding = i; // @TODO Add more bindings
			fragment_instance_bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			fragment_instance_bindings[i].descriptorCount = 1;
			fragment_instance_bindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragment_instance_bindings[i].pImmutableSamplers = 0;
		};
    for (I32 i = pipeline.fragment_instance_uniforms_count; i < pipeline.fragment_instance_uniforms_count + pipeline.fragment_instance_samplers_count; i += 1)
    {
			fragment_instance_bindings[i].binding = i; // @TODO Add more bindings
			fragment_instance_bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			fragment_instance_bindings[i].descriptorCount = 1;
			fragment_instance_bindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragment_instance_bindings[i].pImmutableSamplers = 0;
    }

		VkDescriptorSetLayoutCreateInfo fragment_instance_set_layout_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.flags = 0,
			.bindingCount = pipeline.fragment_instance_uniforms_count + pipeline.fragment_instance_samplers_count,
			.pBindings = fragment_instance_bindings,
		};
		VK_CHECK(vkCreateDescriptorSetLayout(_r_vk_state.device.logical, &fragment_instance_set_layout_info, 0, &pipeline.fragment_instance_set_layout));
		set_layouts[set_layouts_count] = pipeline.fragment_instance_set_layout;
		set_layouts_count += 1;
	}

  VkPipelineLayoutCreateInfo layout_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount = set_layouts_count,
    .pSetLayouts = set_layouts,
  };
  VK_CHECK(vkCreatePipelineLayout(_r_vk_state.device.logical, &layout_info, 0, &pipeline.layout));

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
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
  };

  // @TODO Support multiple attachments
  VkPipelineColorBlendAttachmentState blend_attachments[8] = {0};
  for (I32 i = 0; i < pipeline_info->color_targets_count; i += 1)
  {
    blend_attachments[i].blendEnable = pipeline_info->color_target_infos[i].blend_enable;
    blend_attachments[i].srcColorBlendFactor=VK_BLEND_FACTOR_SRC_ALPHA;
    blend_attachments[i].dstColorBlendFactor=VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blend_attachments[i].colorBlendOp=VK_BLEND_OP_ADD;
    blend_attachments[i].srcAlphaBlendFactor=VK_BLEND_FACTOR_ONE;
    blend_attachments[i].dstAlphaBlendFactor=VK_BLEND_FACTOR_ZERO;
    blend_attachments[i].alphaBlendOp=VK_BLEND_OP_ADD;
    blend_attachments[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  }

  VkPipelineColorBlendStateCreateInfo blend = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .attachmentCount = pipeline_info->color_targets_count,
    .pAttachments = blend_attachments,
  };

  VkPipelineViewportStateCreateInfo viewport_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .viewportCount = 1,
		.pViewports = 0,
		.scissorCount = 1,
		.pScissors = 0,
  };

  VkPipelineDepthStencilStateCreateInfo depth_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
    .depthTestEnable = pipeline_info->depth_stencil_state.depth_test_enable,
    .depthWriteEnable = pipeline_info->depth_stencil_state.depth_write_enable,
    .depthCompareOp = R_VK_GetVkFromCompareOperation(pipeline_info->depth_stencil_state.depth_compare_operation),
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

  VkFormat color_attachment_formats[8] = {0};
  for (I32 i = 0; i < pipeline_info->color_targets_count; i += 1)
  {
    color_attachment_formats[i] = R_VK_GetVkFormat(pipeline_info->color_target_infos[i].format);
  }
  VkPipelineRenderingCreateInfo rendering_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
    .colorAttachmentCount = pipeline_info->color_targets_count,
    .pColorAttachmentFormats = color_attachment_formats,
    .depthAttachmentFormat = R_VK_GetVkFormat(pipeline_info->depth_stencil_state.depth_target_format),
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
    .layout = pipeline.layout,
    .renderPass = 0,
    .subpass = 0,
  };
  VK_CHECK(vkCreateGraphicsPipelines(_r_vk_state.device.logical,
                                     0, 1, &vk_pipeline_info, 0,
                                     &pipeline.handle));

  vkDestroyShaderModule(_r_vk_state.device.logical, vertex_module, 0);
  vkDestroyShaderModule(_r_vk_state.device.logical, fragment_module, 0);

	return R_VK_GraphicsPipelineArrayAdd(&_r_vk_state.graphics_pipelines, pipeline);
}

func void
R_VK_DestroyGraphicsPipeline(R_GraphicsPipeline pipeline)
{
  R_VK_GraphicsPipeline* vk_pipeline = R_VK_GraphicsPipelineFromHandle(pipeline);
  vkDestroyDescriptorSetLayout(_r_vk_state.device.logical, vk_pipeline->vertex_global_set_layout, 0);
  vkDestroyDescriptorSetLayout(_r_vk_state.device.logical, vk_pipeline->fragment_global_set_layout, 0);
  vkDestroyPipelineLayout(_r_vk_state.device.logical, vk_pipeline->layout, 0);
  vkDestroyPipeline(_r_vk_state.device.logical, vk_pipeline->handle, 0);
}

func void
R_VK_BindGraphicsPipeline(R_CommandBuffer command_buffer, R_GraphicsPipeline pipeline)
{
	R_VK_CommandBuffer* vk_command_buffer = R_VK_CommandBufferFromHandle(command_buffer);
  R_VK_GraphicsPipeline* vk_pipeline = R_VK_GraphicsPipelineFromHandle(pipeline);

	vkCmdBindPipeline(vk_command_buffer->handle[_r_vk_state.current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline->handle);
	vk_command_buffer->binded_graphics_pipeline = vk_pipeline;
}

// --------------------------------------------------
// Render Pass
func R_RenderPass*
R_VK_BeginRenderPass(R_CommandBuffer command_buffer, U32 color_targets_count, R_ColorTarget* color_targets, R_DepthStencilTarget* depth_stencil_target)
{
	R_VK_CommandBuffer* vk_command_buffer = R_VK_CommandBufferFromHandle(command_buffer);

  // --AlNov: @TODO Only one now
  VkRenderingAttachmentInfo vk_color_attachment_infos[2] = {0};
  for (I32 i = 0; i < color_targets_count; i += 1)
  {
    R_VK_Texture* vk_attachment_texture = R_VK_TextureFromHandle(color_targets[i].texture);

    VkClearValue clear_value = {0};
    clear_value.color.float32[0] = color_targets[i].clear_color.r;
    clear_value.color.float32[1] = color_targets[i].clear_color.g;
    clear_value.color.float32[2] = color_targets[i].clear_color.b;
    clear_value.color.float32[3] = color_targets[i].clear_color.a;

    vk_color_attachment_infos[i].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    vk_color_attachment_infos[i].imageView = vk_attachment_texture->view;
    vk_color_attachment_infos[i].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    vk_color_attachment_infos[i].loadOp = R_VK_GetVkAttachmentLoadOperation(color_targets[0].load_operation);
    vk_color_attachment_infos[i].storeOp = R_VK_GetVkAttachmentStoreOperation(color_targets[0].store_operation);
    vk_color_attachment_infos[i].clearValue = clear_value;

    R_VK_ChangeTextureLayout(vk_command_buffer->handle[_r_vk_state.current_frame], vk_attachment_texture, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  }

  VkRenderingAttachmentInfo vk_depth_attachment_info = {0};
  vk_depth_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
  if (depth_stencil_target != 0)
  {
    R_VK_Texture* vk_depth_texture = R_VK_TextureFromHandle(depth_stencil_target->texture);
    vk_depth_attachment_info.imageView = vk_depth_texture->view;
    vk_depth_attachment_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    vk_depth_attachment_info.loadOp = R_VK_GetVkAttachmentLoadOperation(depth_stencil_target->depth_load_operation);
    vk_depth_attachment_info.storeOp = R_VK_GetVkAttachmentStoreOperation(depth_stencil_target->depth_store_operation);
    vk_depth_attachment_info.clearValue.depthStencil.depth = depth_stencil_target->clear_depth;
  }

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
    .colorAttachmentCount = color_targets_count,
    .pColorAttachments = vk_color_attachment_infos,
    .pDepthAttachment = &vk_depth_attachment_info,
  };
  
  vkCmdBeginRendering(vk_command_buffer->handle[_r_vk_state.current_frame], &rendering_info);

	return 0; // @TODO
}

func void
R_VK_EndRenderPass(R_CommandBuffer command_buffer, R_RenderPass* render_pass)
{
	R_VK_CommandBuffer* vk_command_buffer = R_VK_CommandBufferFromHandle(command_buffer);

  vkCmdEndRendering(vk_command_buffer->handle[_r_vk_state.current_frame]);
}

// -------------------------------------------------------------------
// Draw
func void
R_VK_SetViewport(R_CommandBuffer command_buffer, RectI32 viewport)
{
	R_VK_CommandBuffer* vk_command_buffer = R_VK_CommandBufferFromHandle(command_buffer);

	VkViewport vk_viewport = {
		.x = viewport.x,
		.y = viewport.y,
		.width = viewport.w,
		.height = viewport.h,
    .minDepth = 0.0f,
    .maxDepth = 1.0f,
	};
	vkCmdSetViewport(vk_command_buffer->handle[_r_vk_state.current_frame], 0, 1, &vk_viewport);
}

func void
R_VK_SetScissor(R_CommandBuffer command_buffer, RectI32 scissor)
{
	R_VK_CommandBuffer* vk_command_buffer = R_VK_CommandBufferFromHandle(command_buffer);
	VkRect2D vk_scissor = {
		.offset.x = scissor.x,
		.offset.y = scissor.y,
		.extent.width = scissor.w,
		.extent.height = scissor.h,
	};
	vkCmdSetScissor(vk_command_buffer->handle[_r_vk_state.current_frame], 0, 1, &vk_scissor);
}

func void
R_VK_DrawPrimitives(R_CommandBuffer command_buffer, U32 vertex_count, U32 instance_count, U32 first_vertex, U32 first_instance)
{
	R_VK_CommandBuffer* vk_command_buffer = R_VK_CommandBufferFromHandle(command_buffer);

	vkCmdDraw(vk_command_buffer->handle[_r_vk_state.current_frame], vertex_count, instance_count, first_vertex, first_instance);
}

func void
R_VK_DrawIndexedPrimitives(R_CommandBuffer command_buffer, U32 index_count, U32 instance_count, U32 first_index, I32 vertex_offset, U32 first_instance)
{
	R_VK_CommandBuffer* vk_command_buffer = R_VK_CommandBufferFromHandle(command_buffer);

	vkCmdDrawIndexed(vk_command_buffer->handle[_r_vk_state.current_frame], index_count, instance_count, first_index, vertex_offset, first_instance);
}

func void
R_VK_PresentTexture(R_CommandBuffer command_buffer, R_Texture texture)
{
  R_VK_CommandBuffer* vk_command_buffer = R_VK_CommandBufferFromHandle(command_buffer);
  R_VK_Texture* vk_texture = R_VK_TextureFromHandle(texture);

  if (vk_texture->from_swapchain)
  {
    VkCommandBuffer single_cmd = R_VK_BeginSingleCmd();
    {
      R_VK_ChangeTextureLayout(single_cmd, R_VK_TextureFromHandle(_r_vk_state.swapchain.textures[_r_vk_state.current_target]), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    }
    R_VK_EndSingleCmd(single_cmd);

    VkPresentInfoKHR present_info = {
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .waitSemaphoreCount = 0,
      .pWaitSemaphores = &_r_vk_state.swapchain.frame_resources[_r_vk_state.current_target].release_semaphore, // @TODO change location of release_semaphore
      .swapchainCount = 1,
      .pSwapchains = &_r_vk_state.swapchain.handle,
      .pImageIndices = &_r_vk_state.current_target,
    };

    VK_CHECK(vkQueuePresentKHR(_r_vk_state.device.graphics_queue, &present_info));
  }
  else
  {
    LOG_WARNING("Trying to present image not from swapchain\n");
    return;
  }

}

// -------------------------------------------------------------------
// Texture
func R_VK_Texture*
R_VK_TextureFromHandle(R_Texture handle)
{
  return R_VK_TextureArrayGetPointer(&_r_vk_state.textures, handle);
}

func R_Texture
R_VK_CreateTexture(R_TextureCreateInfo* info)
{
  R_VK_Texture texture = {0};
  texture.format = info->format;
  texture.size.x = info->width;
  texture.size.y = info->height;
#if 0
#endif

  VkImageCreateInfo image_info = {0};
  image_info.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image_info.samples       = VK_SAMPLE_COUNT_1_BIT;
  image_info.imageType     = R_VK_GetVkImageType(info->type);
  image_info.extent.width  = info->width;
  image_info.extent.height = info->height;
  image_info.extent.depth  = info->depth;
  image_info.mipLevels     = 1;
  image_info.arrayLayers   = 1;
  image_info.format        = R_VK_GetVkFormat(info->format);
  image_info.tiling        = VK_IMAGE_TILING_OPTIMAL;
  image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_info.usage         = R_VK_GetVkImageUsageFlags(info->usage_flags);
  image_info.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
  image_info.samples       = VK_SAMPLE_COUNT_1_BIT;
  if (vkCreateImage(_r_vk_state.device.logical, &image_info, 0, &texture.image) != VK_SUCCESS)
  {
    LOG_ERROR("Cannot create Image for Texture.\n");
    return R_NIL;
  }

  VkMemoryRequirements mem_requirements = {0};
  vkGetImageMemoryRequirements(_r_vk_state.device.logical, texture.image, &mem_requirements);

  VkMemoryAllocateInfo mem_info = {0};
  mem_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  mem_info.allocationSize  = mem_requirements.size;
  mem_info.memoryTypeIndex = R_VK_FindMemoryTypeIndex(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  VK_CHECK(vkAllocateMemory(_r_vk_state.device.logical, &mem_info, 0, &texture.memory));

  VK_CHECK(vkBindImageMemory(_r_vk_state.device.logical, texture.image, texture.memory, 0));

  VkImageAspectFlags texture_aspect = 0;
  if((info->usage_flags & R_TEXTURE_USAGE_FLAG_COLOR_ATTACHMENT) == R_TEXTURE_USAGE_FLAG_COLOR_ATTACHMENT)
  {
    texture_aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    texture.aspect_mask = texture_aspect;
  }
  else if ((info->usage_flags & R_TEXTURE_USAGE_FLAG_DEPTH_STENCIL_ATTACHMENT) == R_TEXTURE_USAGE_FLAG_DEPTH_STENCIL_ATTACHMENT)
  {
    texture_aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
    texture.aspect_mask = texture_aspect;
  }
  else 
  {
    texture_aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    texture.aspect_mask = texture_aspect;
  }

  VkCommandBuffer cmd = R_VK_BeginSingleCmd();
  {
    if ((info->usage_flags & R_TEXTURE_USAGE_FLAG_DEPTH_STENCIL_ATTACHMENT) == R_TEXTURE_USAGE_FLAG_DEPTH_STENCIL_ATTACHMENT)
    {
      R_VK_ChangeTextureLayout(cmd, &texture, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    }
  }
  R_VK_EndSingleCmd(cmd);
  
  // AlNov: Create Texture Image View
  VkImageViewCreateInfo view_info = {0};
  view_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  view_info.image                           = texture.image;
  view_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D; // @TODO
  view_info.format = R_VK_GetVkFormat(info->format);
  view_info.subresourceRange.aspectMask     = texture_aspect;
  view_info.subresourceRange.baseMipLevel   = 0;
  view_info.subresourceRange.levelCount     = 1;
  view_info.subresourceRange.baseArrayLayer = 0;
  view_info.subresourceRange.layerCount     = 1;

  VK_CHECK(vkCreateImageView(_r_vk_state.device.logical, &view_info, 0, &texture.view));

  if (_r_vk_state.textures_free_list.length > 0)
  {
    I32 index = R_VK_TextureFreeListRemoveSwapback(&_r_vk_state.textures_free_list, 0);
    R_VK_TextureArraySet(&_r_vk_state.textures, index, texture);
    return index;
  }
  else
  {
    return R_VK_TextureArrayAdd(&_r_vk_state.textures, texture);  
  }
}

func B32
R_VK_DestroyTexture(R_Texture texture)
{
  R_VK_Texture* vk_texture = R_VK_TextureFromHandle(texture);

  vkDestroyImageView(_r_vk_state.device.logical, vk_texture->view, 0);
  if (!vk_texture->from_swapchain)
  {
    vkFreeMemory(_r_vk_state.device.logical, vk_texture->memory, 0);
    vkDestroyImage(_r_vk_state.device.logical, vk_texture->image, 0);
  }

  R_VK_TextureArraySet(&_r_vk_state.textures, texture, R_VK_TextureDefaultValue);
  R_VK_TextureFreeListAdd(&_r_vk_state.textures_free_list, texture);

  return 1;
}

func void
R_VK_LoadDataToTexture(U8* data, U64 data_size, R_Texture texture)
{
  R_VK_Texture* vk_texture = R_VK_TextureFromHandle(texture);
}

func void
R_VK_CopyTexture(R_CommandBuffer command_buffer, R_Texture source, R_Texture destination)
{
  R_VK_CommandBuffer* vk_command_buffer = R_VK_CommandBufferFromHandle(command_buffer);
  R_VK_Texture* vk_source = R_VK_TextureFromHandle(source);
  R_VK_Texture* vk_destination = R_VK_TextureFromHandle(destination);

  R_VK_ChangeTextureLayout(vk_command_buffer->handle[_r_vk_state.current_frame], vk_source, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  R_VK_ChangeTextureLayout(vk_command_buffer->handle[_r_vk_state.current_frame], vk_destination, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  VkImageBlit blit_info = {
    .srcSubresource = {
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .baseArrayLayer = 0,
      .layerCount = 1,
      .mipLevel = 0,
    },
    .srcOffsets[1] = {
      .x = vk_source->size.x,
      .y = vk_source->size.y,
      .z = 1,
    },
    .dstSubresource = {
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .baseArrayLayer = 0,
      .layerCount = 1,
      .mipLevel = 0,
    },
    .dstOffsets[1] = {
      .x = vk_destination->size.x,
      .y = vk_destination->size.y,
      .z = 1,
    },
  };
  vkCmdBlitImage(
      vk_command_buffer->handle[_r_vk_state.current_frame],
      vk_source->image,
      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
      vk_destination->image,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      1, &blit_info, VK_FILTER_LINEAR);
}

func U64
R_VK_CopyTextureToBuffer(R_CommandBuffer command_buffer, R_Texture texture, R_Buffer buffer)
{
  R_VK_CommandBuffer* vk_command_buffer = R_VK_CommandBufferFromHandle(command_buffer);
  R_VK_Texture* vk_texture = R_VK_TextureFromHandle(texture);
  R_VK_Buffer* vk_buffer = R_VK_BufferFromHandle(buffer);

  R_VK_ChangeTextureLayout(vk_command_buffer->handle[_r_vk_state.current_frame], vk_texture, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

  VkBufferImageCopy copy_info = {
    .bufferOffset = vk_buffer->size,
    .imageSubresource = {
      .aspectMask = vk_texture->aspect_mask,
      .mipLevel = 0,
      .baseArrayLayer = 0,
      .layerCount = 1,
    },
    .imageOffset = {
      .x = 0,
      .y = 0,
      .z = 0,
    },
    .imageExtent = {
      .width = vk_texture->size.x,
      .height = vk_texture->size.y,
      .depth = 1,
    },
  };

  vkCmdCopyImageToBuffer(
      vk_command_buffer->handle[_r_vk_state.current_frame],
      vk_texture->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
      vk_buffer->handle, 1, &copy_info);

  return vk_buffer->size;
}

func void
R_VK_CopyBufferToTexture(R_CommandBuffer command_buffer, R_Buffer buffer, U64 offset, U64 size, R_Texture texture)
{
  R_VK_Buffer* vk_buffer = R_VK_BufferFromHandle(buffer);
  R_VK_Texture* vk_texture = R_VK_TextureFromHandle(texture);

  VkCommandBuffer single_cmd = R_VK_BeginSingleCmd();
  {
    R_VK_ChangeTextureLayout(single_cmd, vk_texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkBufferImageCopy copy_info = {
      .bufferOffset = offset,
      .imageSubresource = {
        .aspectMask = vk_texture->aspect_mask,
        .mipLevel = 0,
        .baseArrayLayer = 0,
        .layerCount = 1,
      },
      .imageOffset = {
        .x = 0,
        .y = 0,
        .z = 0,
      },
      .imageExtent = {
        .width = vk_texture->size.x,
        .height = vk_texture->size.y,
        .depth = 1,
      },
    };
    vkCmdCopyBufferToImage(single_cmd, vk_buffer->handle, vk_texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_info);

    R_VK_ChangeTextureLayout(single_cmd, vk_texture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  }
  R_VK_EndSingleCmd(single_cmd);
}

func R_TextureFormat
R_VK_GetTextureFormat(R_Texture texture)
{
  return R_VK_TextureFromHandle(texture)->format;
}

func void
R_VK_ChangeTextureLayout(VkCommandBuffer cmd, R_VK_Texture* texture, VkImageLayout new_layout)
{
  VkPipelineStageFlags source_stages = 0;
  VkPipelineStageFlags destination_stages = 0;
  VkImageMemoryBarrier image_barrier = {0};
  image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  image_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  image_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  image_barrier.image = texture->image;
  image_barrier.subresourceRange.aspectMask = texture->aspect_mask;
  image_barrier.subresourceRange.baseMipLevel = 0;
  image_barrier.subresourceRange.levelCount = 1;
  image_barrier.subresourceRange.baseArrayLayer = 0;
  image_barrier.subresourceRange.layerCount = 1;

  if (texture->layout == VK_IMAGE_LAYOUT_UNDEFINED)
  {
    source_stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    image_barrier.srcAccessMask = 0;
    image_barrier.oldLayout = texture->layout;
  }
  else if (texture->layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
  {
    source_stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    image_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    image_barrier.oldLayout = texture->layout;
  }
  else if (texture->layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
  {
    source_stages = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    image_barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    image_barrier.oldLayout = texture->layout;
  }
  else if (texture->layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
  {
    source_stages = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    image_barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    image_barrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  }
  else if (texture->layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
  {
    source_stages = VK_PIPELINE_STAGE_TRANSFER_BIT;
    image_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    image_barrier.oldLayout = texture->layout;
  }
  else if (texture->layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
  {
    source_stages = VK_PIPELINE_STAGE_TRANSFER_BIT;
    image_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    image_barrier.oldLayout = texture->layout;
  }
  else if(texture->layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
  {
    source_stages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    image_barrier.srcAccessMask = 0;
    image_barrier.oldLayout = texture->layout;
  }
  else
  {
    AssertMessage(0, "Image Layout is not supproted");
  }

  if (new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
  {
    destination_stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    image_barrier.newLayout = new_layout;
  }
  else if (new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
  {
    destination_stages = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    image_barrier.newLayout = new_layout;
  }
  else if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
  {
    destination_stages = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    image_barrier.newLayout = new_layout;
  }
  else if (new_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
  {
    destination_stages = VK_PIPELINE_STAGE_TRANSFER_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    image_barrier.newLayout = new_layout;
  }
  else if (new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
  {
    destination_stages = VK_PIPELINE_STAGE_TRANSFER_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    image_barrier.newLayout = new_layout;
  }
  else if (new_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
  {
    destination_stages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    image_barrier.dstAccessMask = 0;
    image_barrier.newLayout = new_layout;
  }
  else
  {
    AssertMessage(0, "Image Layout is not supproted");
  }

  vkCmdPipelineBarrier(cmd, source_stages, destination_stages, 0, 0, 0, 0, 0, 1, &image_barrier);

  texture->layout = new_layout;
}

func R_VK_TextureSampler*
R_VK_TextureSamplerFromHandle(R_TextureSampler sampler)
{
  return R_VK_TextureSamplerArrayGetPointer(&_r_vk_state.samplers, sampler);
}

func R_TextureSampler
R_VK_CreateTextureSampler(R_TextureSamplerCreateInfo* info)
{
  R_VK_TextureSampler sampler = {0};

  VkSamplerCreateInfo sampler_info = {
    .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
    .magFilter = R_VK_GetVkFilter(info->mag_filter),
    .minFilter = R_VK_GetVkFilter(info->min_filter),
    .mipmapMode = R_VK_GetVkSamplerMipmapMode(info->mipmap_mode),
    .addressModeU = R_VK_GetVkSamplerAddressMode(info->address_mode_u),
    .addressModeV = R_VK_GetVkSamplerAddressMode(info->address_mode_v),
    .addressModeW = R_VK_GetVkSamplerAddressMode(info->address_mode_w),
    .mipLodBias = info->mip_lod_bias,
    .anisotropyEnable = info->anisotropy_enable,
    .maxAnisotropy = info->max_anisotropy,
    .compareEnable = info->compare_enable,
    .compareOp = R_VK_GetVkFromCompareOperation(info->compare_operation),
    .minLod = info->min_lod,
    .maxLod = info->max_lod,
  };
  VK_CHECK(vkCreateSampler(_r_vk_state.device.logical, &sampler_info, 0, &sampler.handle));

  return R_VK_TextureSamplerArrayAdd(&_r_vk_state.samplers, sampler);
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
  _r_vk_state.buffers = R_VK_BufferArrayAllocate(_r_vk_state.arena, 32);
  _r_vk_state.buffers.elements[0] = (R_VK_Buffer){0};
  _r_vk_state.graphics_pipelines = R_VK_GraphicsPipelineArrayAllocate(_r_vk_state.arena, 32);
  _r_vk_state.graphics_pipelines.elements[0] = (R_VK_GraphicsPipeline){0};
  _r_vk_state.command_buffers = R_VK_CommandBufferArrayAllocate(_r_vk_state.arena, 16);
  _r_vk_state.command_buffers.elements[0] = (R_VK_CommandBuffer){0};
  _r_vk_state.samplers = R_VK_TextureSamplerArrayAllocate(_r_vk_state.arena, 32);
  _r_vk_state.samplers.elements[0] = (R_VK_TextureSampler){0};
  _r_vk_state.textures = R_VK_TextureArrayAllocate(_r_vk_state.arena, 32);
  _r_vk_state.textures.elements[0] = (R_VK_Texture){0};
  _r_vk_state.textures_free_list = R_VK_TextureFreeListAllocate(_r_vk_state.arena, 32);
  _r_vk_state.textures_free_list.elements[0] = 1;

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

  VK_CHECK(vkCreateInstance(&instance_info, 0, &_r_vk_state.instance));

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

  LOG_INFO("Rendered is initialized\n");
	return 0;
}

func B32
R_VK_Shutdown(void)
{
  vkDeviceWaitIdle(_r_vk_state.device.logical);

  for (I32 i = 0; i < _r_vk_state.textures.length; i += 1)
  {
    R_VK_DestroyTexture(i);
  }

  for (I32 i = 0; i < _r_vk_state.command_buffers.length; i += 1)
  {
    R_VK_ReleaseCommandBuffer(i);
  }
  
  for (I32 i = 0; i < _r_vk_state.graphics_pipelines.length; i += 1)
  {
    R_VK_DestroyGraphicsPipeline(i);
  }

  vkDestroyCommandPool(_r_vk_state.device.logical, _r_vk_state.command_pool, 0);

  R_VK_DestroySwapchain();

  vkDestroyDevice(_r_vk_state.device.logical, 0);
#if IGNIS_DEBUG
  R_VK_DestroyDebugUtilsMessenger(_r_vk_state.instance, _r_vk_state.debug_messenger, 0);
#endif // IGNIS_DEBUG
  vkDestroyInstance(_r_vk_state.instance, 0);

  FreeArena(_r_vk_state.arena);

	return 1;
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
  LOG_WARNING("VK_VALIDATION: %s\n", pCallbackData->pMessage);

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
