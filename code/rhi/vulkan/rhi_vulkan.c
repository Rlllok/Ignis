#include "rhi_vulkan.h"
#include "rhi_vk_utils.h"

#include "vei/vei.h"

#include "base/base_core.h"

// -------------------------------------------------------------------
// -- Buffer ---------------------------------------------------------
func VkBufferUsageFlags
_VkFromBufferUsageFlags(RHI_BufferUsageFlags flags) {
  VkBufferUsageFlags result = 0;

  if(flags & RHI_BufferUsageFlag_Vertex) {
    result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  }
  if(flags & RHI_BufferUsageFlag_Index) {
    result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  }
  if(flags & RHI_BufferUsageFlag_Uniform) {
    result |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  }
  if(flags & RHI_BufferUsageFlag_Storage) {
    result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  }
  if(flags & RHI_BufferUsageFlag_Transfer) {
    result |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    result |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  }
  if (flags & RHI_BufferUsageFlag_Address) {
    result |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
  }

  return result;
}

func VkMemoryPropertyFlags
_VkFromBufferPropertyFlags(RHI_BufferPropertyFlags flags) {
  VkMemoryPropertyFlags result = 0;

  if (flags & RHI_BufferPropertyFlag_HostCoherent) {
    result |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  }
  if (flags & RHI_BufferPropertyFlag_HostVisible) {
    result |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
  }

  return result;
}

func RHI_VK_Buffer*
RHI_VK_BufferFromHandle(RHI_Buffer buffer) {
  return RHI_VK_BufferArrayGetPointer(&_rhi_vk_state.buffers, buffer);
}

func RHI_Buffer
RHI_VK_CreateBuffer(Str8 label, U32 capacity, RHI_BufferUsageFlags usage_flags, RHI_BufferPropertyFlags property_flags) {
  // @NOTE This is to create Vulkan Buffer and Memory
  RHI_VK_Buffer buffer = {0};
	buffer.capacity = capacity;
  
  VkBufferCreateInfo buffer_info = {
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .size = capacity,
    .usage = _VkFromBufferUsageFlags(usage_flags),
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE
  };

  VK_CHECK(vkCreateBuffer(_rhi_vk_state.device.logical, &buffer_info, 0, &buffer.vk));

  VkMemoryRequirements memory_requirements;
  vkGetBufferMemoryRequirements(_rhi_vk_state.device.logical, buffer.vk, &memory_requirements);
  VkPhysicalDeviceMemoryProperties mem_properties = {0};
  vkGetPhysicalDeviceMemoryProperties(_rhi_vk_state.device.physical, &mem_properties);

  VkMemoryPropertyFlags flags = _VkFromBufferPropertyFlags(property_flags);
  U32 memory_type_index = 0;
  for (U32 type_index = 0; type_index < mem_properties.memoryTypeCount; type_index += 1) {
    if (memory_requirements.memoryTypeBits & (1 << type_index) && ((mem_properties.memoryTypes[type_index].propertyFlags & flags) == flags)) {
      memory_type_index = type_index;
      break;
    }
  }

  VkMemoryAllocateFlagsInfo allocation_flags_info = {
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
  };
  if (usage_flags & RHI_BufferUsageFlag_Address) {
    allocation_flags_info.flags |= VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
  }

  VkMemoryAllocateInfo allocation_info = {
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .pNext = &allocation_flags_info,
    .allocationSize = memory_requirements.size,
    .memoryTypeIndex = memory_type_index
  };
  VK_CHECK(vkAllocateMemory(_rhi_vk_state.device.logical, &allocation_info , 0, &buffer.memory));
  VK_CHECK(vkBindBufferMemory(_rhi_vk_state.device.logical, buffer.vk, buffer.memory, 0));
  vkMapMemory(_rhi_vk_state.device.logical, buffer.memory, 0, VK_WHOLE_SIZE, 0, &buffer.mapped);

  return RHI_VK_BufferArrayAdd(&_rhi_vk_state.buffers, buffer);
}

func void
RHI_VK_DestroyBuffer(RHI_Buffer buffer) {
  RHI_VK_Buffer* vk_buffer = RHI_VK_BufferFromHandle(buffer);

  vkDeviceWaitIdle(_rhi_vk_state.device.logical);
  vkUnmapMemory(_rhi_vk_state.device.logical, vk_buffer->memory);
  vkFreeMemory(_rhi_vk_state.device.logical, vk_buffer->memory, 0);
  vkDestroyBuffer(_rhi_vk_state.device.logical, vk_buffer->vk, 0);

  *vk_buffer = (RHI_VK_Buffer){0};
}

func U64
RHI_VK_PushBuffer(RHI_Buffer buffer, U8* data, U64 size) {
  RHI_VK_Buffer* vk_buffer = RHI_VK_BufferFromHandle(buffer);

	// Assert((vk_buffer->size + size) < vk_buffer->capacity);
  if (vk_buffer->size + size > vk_buffer->capacity) {
    LogDebug("RHI_VK. Ring buffer reseted to zero\n");
    vk_buffer->size = 0;
  }
	U32 offset = vk_buffer->size;

	memcpy((U8*)vk_buffer->mapped + vk_buffer->size, data, size);
	vk_buffer->size += size;
  U64 allignment = 16;
	U64 padding = allignment - (vk_buffer->size + allignment)%allignment;
	vk_buffer->size += padding;

	return offset;
}

func void
RHI_VK_ResetBuffer(RHI_Buffer buffer) {
}

func RHI_DeviceAddress
RHI_VK_BufferDeviceAddress(RHI_Buffer buffer) {
  RHI_DeviceAddress result = 0;

  RHI_VK_Buffer* vk_buffer = RHI_VK_BufferFromHandle(buffer);
  VkBufferDeviceAddressInfo info = {
    .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
    .buffer = vk_buffer->vk,
  };
  result = vkGetBufferDeviceAddress(_rhi_vk_state.device.logical, &info);
  return result;
}

func void
RHI_VK_BindIndexBuffer(RHI_CommandBuffer command_buffer, RHI_Buffer buffer, U64 offset, RHI_IndexSize index_size) {
	RHI_VK_CommandBuffer* vk_command_buffer = RHI_VK_CommandBufferFromHandle(command_buffer);
  RHI_VK_Buffer* vk_buffer = RHI_VK_BufferFromHandle(buffer);

	vkCmdBindIndexBuffer(vk_command_buffer->vk[_rhi_vk_state.current_frame], vk_buffer->vk, offset, RHI_VK_GetVkIndexTypeFrom(index_size));
}

func void
RHI_VK_BindVertexBuffer(RHI_CommandBuffer command_buffer, RHI_Buffer buffer, U64 offset) {
	RHI_VK_CommandBuffer* vk_command_buffer = RHI_VK_CommandBufferFromHandle(command_buffer);
  RHI_VK_Buffer* vk_buffer = RHI_VK_BufferFromHandle(buffer);
	VkDeviceSize vk_offset = offset;

	vkCmdBindVertexBuffers(vk_command_buffer->vk[_rhi_vk_state.current_frame], 0, 1, &vk_buffer->vk, &vk_offset);
}

func void
RHI_VK_BufferGetData(RHI_Buffer buffer, U64 offset, void* dst, U64 data_size) {
  RHI_VK_Buffer* vk_buffer = RHI_VK_BufferFromHandle(buffer);
  memcpy(dst, (U8*)vk_buffer->mapped + offset, data_size);
}

// -------------------------------------------------------------------
// -- Command Buffer -------------------------------------------------
func RHI_VK_CommandBuffer*
RHI_VK_CommandBufferFromHandle(RHI_CommandBuffer command_buffer) {
  return RHI_VK_CommandBufferArrayGetPointer(&_rhi_vk_state.command_buffers, command_buffer);
}

func RHI_CommandBuffer RHI_VK_GetCommandBuffer(void) {
	RHI_VK_CommandBuffer command_buffer = {0};

	VkCommandBufferAllocateInfo allocate_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = _rhi_vk_state.command_pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = RHI_FRAMES_IN_FLIGHT
	};
	VK_CHECK(vkAllocateCommandBuffers(_rhi_vk_state.device.logical, &allocate_info, command_buffer.vk));

	for (I32 i = 0; i < RHI_FRAMES_IN_FLIGHT; i += 1) {
		VkFenceCreateInfo fence_info = {
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT
		};
		VK_CHECK(vkCreateFence(_rhi_vk_state.device.logical, &fence_info, 0, (command_buffer.submit_fence + i)));
	}

	for (I32 i = 0; i < RHI_FRAMES_IN_FLIGHT; i += 1) {
		VkSemaphoreCreateInfo semaphore_info = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		};
		VK_CHECK(vkCreateSemaphore(_rhi_vk_state.device.logical, &semaphore_info, 0, (command_buffer.acquire_semaphore + i)));
	}

  return RHI_VK_CommandBufferArrayAdd(&_rhi_vk_state.command_buffers, command_buffer);
}

func void
RHI_VK_ReleaseCommandBuffer(RHI_CommandBuffer command_buffer) {
  RHI_VK_CommandBuffer* vk_command_buffer = RHI_VK_CommandBufferFromHandle(command_buffer);

  vkDeviceWaitIdle(_rhi_vk_state.device.logical);
  for (I32 i = 0; i < RHI_FRAMES_IN_FLIGHT; i += 1) {
    vkDestroySemaphore(_rhi_vk_state.device.logical, vk_command_buffer->acquire_semaphore[i], 0);
    vkDestroyFence(_rhi_vk_state.device.logical, vk_command_buffer->submit_fence[i], 0);
    for (I32 j = 0; j < vk_command_buffer->descriptor_pool[i].pool_count; j += 1) {
      vkDestroyDescriptorPool(_rhi_vk_state.device.logical, vk_command_buffer->descriptor_pool[i].vk_pools[j], 0);
    }
  }
}

func void
RHI_VK_BeginCommandBuffer(RHI_CommandBuffer command_buffer) {
  RHI_VK_CommandBuffer* vk_command_buffer = RHI_VK_CommandBufferFromHandle(command_buffer);
  
  vk_command_buffer->active_render_pass = 0;
  vk_command_buffer->binded_graphics_pipeline = 0;
  vk_command_buffer->current_swapchain_texture = 0;
  vk_command_buffer->current_viewport = (VkViewport)ZeroStruct();

  vkWaitForFences(_rhi_vk_state.device.logical, 1, vk_command_buffer->submit_fence + _rhi_vk_state.current_frame, VK_TRUE, U64_MAX);
	vkResetFences(_rhi_vk_state.device.logical, 1, vk_command_buffer->submit_fence + _rhi_vk_state.current_frame);

	vkResetCommandBuffer(vk_command_buffer->vk[_rhi_vk_state.current_frame], 0);
	for (I32 i = 0; i < vk_command_buffer->descriptor_pool[_rhi_vk_state.current_frame].pool_count; i += 1) {
		vkResetDescriptorPool(_rhi_vk_state.device.logical, vk_command_buffer->descriptor_pool[_rhi_vk_state.current_frame].vk_pools[i], 0);
	}

	vk_command_buffer->descriptor_pool[_rhi_vk_state.current_frame].sets_count = 0;

  VkCommandBufferBeginInfo begin_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
  }; 
  VK_CHECK(vkBeginCommandBuffer(vk_command_buffer->vk[_rhi_vk_state.current_frame], &begin_info));
}

func void
RHI_VK_SubmitCommandBuffer(RHI_CommandBuffer command_buffer) {
  RHI_VK_CommandBuffer* vk_command_buffer = RHI_VK_CommandBufferFromHandle(command_buffer);

  RHI_VK_Texture* swapchain_texture = vk_command_buffer->current_swapchain_texture;
  if (swapchain_texture) {
    RHI_VK_ChangeTextureLayout(vk_command_buffer->vk[_rhi_vk_state.current_frame], swapchain_texture, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
  }

	vkEndCommandBuffer(vk_command_buffer->vk[_rhi_vk_state.current_frame]);

	VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = vk_command_buffer->acquire_semaphore + _rhi_vk_state.current_frame,
		.pWaitDstStageMask = &wait_stage,
		.commandBufferCount = 1,
		.pCommandBuffers = vk_command_buffer->vk + _rhi_vk_state.current_frame,
		.signalSemaphoreCount = 1,
    .pSignalSemaphores = _rhi_vk_state.swapchain.render_finished_semaphores + _rhi_vk_state.current_target,
	};

  VK_CHECK(vkQueueSubmit(_rhi_vk_state.device.graphics_queue, 1, &submit_info, vk_command_buffer->submit_fence[_rhi_vk_state.current_frame]));

  if (swapchain_texture) {
    VkPresentInfoKHR present_info = {
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = _rhi_vk_state.swapchain.render_finished_semaphores + _rhi_vk_state.current_target,
      .swapchainCount = 1,
      .pSwapchains = &_rhi_vk_state.swapchain.vk,
      .pImageIndices = &_rhi_vk_state.current_target,
    };

    vkQueuePresentKHR(_rhi_vk_state.device.graphics_queue, &present_info);
  }

	_rhi_vk_state.current_frame = (_rhi_vk_state.current_frame + 1)%RHI_FRAMES_IN_FLIGHT;
}

// -------------------------------------------------------------------
// -- Device ---------------------------------------------------------
func void
RHI_VK_CreateDevice() {
  U32 device_count = 0;
  VK_CHECK(vkEnumeratePhysicalDevices(_rhi_vk_state.instance, &device_count, 0));
  
  ScratchArena scratch = BeginScratchArena(_rhi_vk_state.arena);
  {
    VkPhysicalDevice* devices = (VkPhysicalDevice*)PushArena(scratch.arena, device_count * sizeof(VkPhysicalDevice));
    VK_CHECK(vkEnumeratePhysicalDevices(_rhi_vk_state.instance, &device_count, devices));

    for (I32 i = 0; i < device_count; i += 1) {
      VkPhysicalDevice* device = devices + i;
      
      VkPhysicalDeviceProperties properties;
      vkGetPhysicalDeviceProperties(*device, &properties);

      if (properties.apiVersion < VK_API_VERSION_1_3) continue;

      U32 queue_family_count = 0;
      vkGetPhysicalDeviceQueueFamilyProperties(*device, &queue_family_count, 0);
      VkQueueFamilyProperties* queue_properties = (VkQueueFamilyProperties*)PushArena(scratch.arena, queue_family_count * sizeof(VkQueueFamilyProperties));
      vkGetPhysicalDeviceQueueFamilyProperties(*device, &queue_family_count, queue_properties);

      for (I32 j = 0; j < queue_family_count; j += 1) {
        VkQueueFamilyProperties* properties = queue_properties + j;

        if (properties->queueFlags & VK_QUEUE_GRAPHICS_BIT) {
          _rhi_vk_state.device.graphics_queue_index = j;
          break;
        }
      }

      const char* required_extensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
      };

      VkPhysicalDeviceVulkan12Features vulkan12_features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .descriptorIndexing = 1,
        .shaderSampledImageArrayNonUniformIndexing = 1,
        .descriptorBindingVariableDescriptorCount = 1,
        .runtimeDescriptorArray = 1,
        .bufferDeviceAddress = 1,
      };

      VkPhysicalDeviceVulkan13Features vulkan13_features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &vulkan12_features,
        .dynamicRendering = 1,
      };

      F32 queue_priority = 1.0f;

      VkDeviceQueueCreateInfo queue_info = {0};
      queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queue_info.queueFamilyIndex = _rhi_vk_state.device.graphics_queue_index;
      queue_info.queueCount = 1;
      queue_info.pQueuePriorities = &queue_priority;

      VkPhysicalDeviceFeatures enabled_features = {
        .independentBlend = 1,
        .shaderInt64 = 1,
      };

      VkDeviceCreateInfo device_info = {0};
      device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
      device_info.queueCreateInfoCount = 1;
      device_info.pQueueCreateInfos = &queue_info;
      device_info.enabledExtensionCount = ArrayLength(required_extensions);
      device_info.ppEnabledExtensionNames = required_extensions;
      device_info.pEnabledFeatures = &enabled_features;
      device_info.pNext = &vulkan13_features;

      LogInfo("%s\n", properties.deviceName);
      _rhi_vk_state.device.physical = *device;
      VK_CHECK(vkCreateDevice(*device, &device_info, 0, &_rhi_vk_state.device.logical))
			if(_rhi_vk_state.device.logical) {
				vkGetDeviceQueue(_rhi_vk_state.device.logical, _rhi_vk_state.device.graphics_queue_index, 0, &_rhi_vk_state.device.graphics_queue);
				break;
				// @TODO Choose GPU by parameters
			}
    }
  }
  EndScratchArena(scratch);
}

func void
RHI_VK_DestroyDevice() {
  vkDestroyDevice(_rhi_vk_state.device.logical, 0);

  _rhi_vk_state.device = (RHI_VK_Device)ZeroStruct();
}

// -------------------------------------------------------------------
// -- Surface/Swapchain ----------------------------------------------
func void
RHI_VK_CreateSwapchain(OS_Window* window) {
#if IGNIS_PLATFORM_WIN32
  OS_Win32_Window* win32_window = (OS_Win32_Window*)window;

  VkWin32SurfaceCreateInfoKHR surface_info = {0};
  surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  surface_info.hinstance = win32_window->instance;
  surface_info.hwnd = win32_window->handle;

  VK_CHECK(vkCreateWin32SurfaceKHR(_rhi_vk_state.instance, &surface_info, 0, &_rhi_vk_state.swapchain.surface));
#endif // IGNIS_PLATFORM_WIN32

#if IGNIS_PLATFORM_LINUX_WAYLAND
  OS_WL_Window* wayland_window = (OS_WL_Window*)window;

  VkWaylandSurfaceCreateInfoKHR surface_info = {
    .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
    .display = wayland_window->display,
    .surface = wayland_window->surface
  };

  VK_CHECK(vkCreateWaylandSurfaceKHR(_rhi_vk_state.instance, &surface_info, 0, &_rhi_vk_state.swapchain.surface));
#endif // IGNIS_PLATFORM_LINUX

#if IGNIS_PLATFORM_LINUX_X11
	VkXlibSurfaceCreateInfoKHR surface_info = {
    .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
		.dpy = window->vk->display,
		.window = window->vk->window,
	};
	VK_CHECK(vkCreateXlibSurfaceKHR(_rhi_vk_state.instance, &surface_info, 0, &_rhi_vk_state.swapchain.surface));
#endif // IGNIS_PLATFORM_LINUX_X11
  
  ScratchArena scratch = BeginScratchArena(_rhi_vk_state.arena);
  {
    U32 format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(_rhi_vk_state.device.physical, _rhi_vk_state.swapchain.surface, &format_count, 0);
    VkSurfaceFormatKHR* formats = (VkSurfaceFormatKHR*)PushArena(scratch.arena, format_count * sizeof(VkSurfaceFormatKHR));
    vkGetPhysicalDeviceSurfaceFormatsKHR(_rhi_vk_state.device.physical, _rhi_vk_state.swapchain.surface, &format_count, formats);

    for (U32 i = 0; i < format_count; i += 1) {
      if (formats[i].format == VK_FORMAT_B8G8R8A8_UNORM && formats[i].colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
        _rhi_vk_state.swapchain.surface_format = formats[i];
      }
    }
  }
  EndScratchArena(scratch);
  
  VkSurfaceCapabilitiesKHR capabilities;
  VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_rhi_vk_state.device.physical,
                                                     _rhi_vk_state.swapchain.surface,
                                                     &capabilities));
  if (capabilities.currentExtent.width == U32_MAX) {
    _rhi_vk_state.swapchain.size = window->size;
  }
  else {
    _rhi_vk_state.swapchain.size.w = capabilities.currentExtent.width;
    _rhi_vk_state.swapchain.size.h = capabilities.currentExtent.height;
  }

  // VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
  // VkPresentModeKHR present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
  VkPresentModeKHR present_mode = VK_PRESENT_MODE_MAILBOX_KHR;

  U32 image_count = capabilities.minImageCount + 1;
  if ((capabilities.maxImageCount > 0) && (image_count > capabilities.maxImageCount)) {
    image_count = capabilities.maxImageCount;
  }
  Assert(image_count <= RHI_VK_SWAPCHAIN_MAX_IMAGES)

  VkSwapchainCreateInfoKHR swapchain_info = {
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .surface = _rhi_vk_state.swapchain.surface,
    .minImageCount = image_count,
    .imageFormat = _rhi_vk_state.swapchain.surface_format.format,
    .imageColorSpace = _rhi_vk_state.swapchain.surface_format.colorSpace,
    .imageExtent = {.width = _rhi_vk_state.swapchain.size.w, .height = _rhi_vk_state.swapchain.size.h},
    .imageArrayLayers = 1,
    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .preTransform = capabilities.currentTransform,
    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .presentMode = present_mode,
    .clipped = 0,
    .oldSwapchain = 0
  };

  VK_CHECK(vkCreateSwapchainKHR(_rhi_vk_state.device.logical, &swapchain_info, 0, &_rhi_vk_state.swapchain.vk));
	_rhi_vk_state.swapchain.window = window;

  VK_CHECK(vkGetSwapchainImagesKHR(_rhi_vk_state.device.logical, _rhi_vk_state.swapchain.vk, &_rhi_vk_state.swapchain.image_count, 0));
  VkImage images[RHI_VK_SWAPCHAIN_MAX_IMAGES] = ZeroStruct();
  VK_CHECK(vkGetSwapchainImagesKHR(_rhi_vk_state.device.logical, _rhi_vk_state.swapchain.vk, &_rhi_vk_state.swapchain.image_count, images));

  for (U32 i = 0; i < _rhi_vk_state.swapchain.image_count; i += 1) {
    VkImageView image_view = {0};

    VkImageViewCreateInfo view_info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = images[i],
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = _rhi_vk_state.swapchain.surface_format.format,
      .subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1
      }
    };

    VK_CHECK(vkCreateImageView(_rhi_vk_state.device.logical, &view_info, 0, &image_view));

    RHI_VK_Texture vk_texture = {
      .format = RHI_VK_FormatFromVk(_rhi_vk_state.swapchain.surface_format.format),
      .image = images[i],
      .view = image_view,
      .aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT,
      .size = {
        .x = _rhi_vk_state.swapchain.size.w,
        .y = _rhi_vk_state.swapchain.size.h,
      },
      .from_swapchain = 1,
    };

    _rhi_vk_state.swapchain.textures[i] = RHI_VK_TextureArrayAdd(&_rhi_vk_state.textures, vk_texture);

    VkSemaphoreCreateInfo semaphore_info = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    VK_CHECK(vkCreateSemaphore(_rhi_vk_state.device.logical, &semaphore_info, 0, &_rhi_vk_state.swapchain.render_finished_semaphores[i]));
  }
}

func void
RHI_VK_DestroySwapchain() {
  vkDeviceWaitIdle(_rhi_vk_state.device.logical);

  for (I32 i = 0; i < _rhi_vk_state.swapchain.image_count; i += 1) {
    RHI_VK_DestroyTexture(_rhi_vk_state.swapchain.textures[i]);
    vkDestroySemaphore(_rhi_vk_state.device.logical, _rhi_vk_state.swapchain.render_finished_semaphores[i], 0);
  }
  vkDestroySwapchainKHR(_rhi_vk_state.device.logical, _rhi_vk_state.swapchain.vk, 0);
  vkDestroySurfaceKHR(_rhi_vk_state.instance, _rhi_vk_state.swapchain.surface, 0);
}

func void
RHI_VK_RecreateSwapchain(OS_Window* window) {
  LogInfo("Recreate Swapchain\n");
  RHI_VK_DestroySwapchain();
  RHI_VK_CreateSwapchain(window);
}

func RHI_TextureFormat
RHI_VK_GetSwapchainTextureFormat() {
  return RHI_VK_TextureFormatFromVkFormat(_rhi_vk_state.swapchain.surface_format.format);
}

func RHI_Texture
RHI_VK_AcquireSwapchainTexture(RHI_CommandBuffer command_buffer) {
	RHI_VK_CommandBuffer* vk_command_buffer = RHI_VK_CommandBufferFromHandle(command_buffer);

	while(1) {
		VkResult acquire_result = vkAcquireNextImageKHR(
			_rhi_vk_state.device.logical, _rhi_vk_state.swapchain.vk, U64_MAX,
			vk_command_buffer->acquire_semaphore[_rhi_vk_state.current_frame], 0,
			&_rhi_vk_state.current_target
		);

		if (acquire_result == VK_SUCCESS) {
			break;
		}
		else if (acquire_result == VK_SUBOPTIMAL_KHR || acquire_result == VK_ERROR_OUT_OF_DATE_KHR) {
      LogInfo("Swapchain should be recreated\n");
			RHI_VK_RecreateSwapchain(_rhi_vk_state.swapchain.window);
		}
		else if (acquire_result == VK_NOT_READY || acquire_result == VK_TIMEOUT) {
			continue;
		}
		else {
			Assert(acquire_result == VK_SUCCESS);
		}
	}

  vk_command_buffer->current_swapchain_texture = RHI_VK_TextureFromHandle(_rhi_vk_state.swapchain.textures[_rhi_vk_state.current_target]);
  return _rhi_vk_state.swapchain.textures[_rhi_vk_state.current_target];
}

func void
RHI_VK_BindShaderArguments(RHI_CommandBuffer command_buffer, RHI_ShaderKind stage, RHI_ShaderArgument* arguments, I32 arguments_count) {
  RHI_VK_CommandBuffer* vk_command_buffer = RHI_VK_CommandBufferFromHandle(command_buffer);
  RHI_VK_GraphicsPipeline* vk_pipeline = vk_command_buffer->binded_graphics_pipeline;

  Assert(vk_pipeline != 0);

  ScratchArena scratch = BeginScratchArena(_rhi_vk_state.arena); {
    I32 arguments_size = 0;
    for (I32 argument_index = 0; argument_index < arguments_count; argument_index += 1) {
      RHI_ShaderArgument* argument = arguments + argument_index;
      if (argument->kind == RHI_ShaderArgumentKind_BufferAddress) {
        arguments_size += sizeof(RHI_DeviceAddress);
      }
    }

    U8* arguments_data = PushArena(scratch.arena, arguments_size);
    U8* current_argument = arguments_data;
    for (I32 argument_index = 0; argument_index < arguments_count; argument_index += 1) {
      RHI_ShaderArgument* argument = arguments + argument_index;
      if (argument->kind == RHI_ShaderArgumentKind_BufferAddress) {
        *(RHI_DeviceAddress*)current_argument = argument->address;
        current_argument += sizeof(RHI_DeviceAddress);
      }
    }

    VkShaderStageFlags stage_flags = 0;
    if (stage == RHI_ShaderKind_Vertex) {
      stage_flags = VK_SHADER_STAGE_VERTEX_BIT;
    }
    else if (stage == RHI_ShaderKind_Fragment) {
      stage_flags = VK_SHADER_STAGE_FRAGMENT_BIT;
    }
    vkCmdPushConstants(vk_command_buffer->vk[_rhi_vk_state.current_frame], vk_pipeline->layout, stage_flags, 0, arguments_size, arguments_data);
  }
  EndScratchArena(scratch);
}

func void
RHI_VK_BindShaderData(RHI_CommandBuffer command_buffer, RHI_ShaderKind shader_kind, B32 is_global, I32 uniform_buffers_count, RHI_UniformBufferBindingInfo* uniform_infos, I32 sampler_count, RHI_SamplerBindingInfo* sampler_infos) {
#if 0
  RHI_VK_CommandBuffer* vk_command_buffer = RHI_VK_CommandBufferFromHandle(command_buffer);
  RHI_VK_DescriptorPool* descriptor_pool = &vk_command_buffer->descriptor_pool[_rhi_vk_state.current_frame];
  U32 max_sets = descriptor_pool->pool_count*RHI_VK_SETS_PER_POOL;

  Assert(vk_command_buffer->binded_graphics_pipeline != 0);

  if (max_sets <= descriptor_pool->sets_count) {
    Assert(descriptor_pool->pool_count < RHI_VK_MAX_POOL_COUNT);

    VkDescriptorPoolSize pool_sizes[] = {
      {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = RHI_VK_MAX_UNIFORM_BUFFERS_PER_SET*RHI_VK_SETS_PER_POOL,
      },
      {
        .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = RHI_VK_MAX_SAMPLERS_PER_SET*RHI_VK_SETS_PER_POOL,
      }
    };

    VkDescriptorPoolCreateInfo pool_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .flags = 0,
      .maxSets = RHI_VK_SETS_PER_POOL,
      .poolSizeCount = ArrayLength(pool_sizes),
      .pPoolSizes = pool_sizes,
    };
		VK_CHECK(vkCreateDescriptorPool(_rhi_vk_state.device.logical, &pool_info, 0, vk_command_buffer->descriptor_pool[_rhi_vk_state.current_frame].vk_pools + vk_command_buffer->descriptor_pool[_rhi_vk_state.current_frame].pool_count));
    descriptor_pool->pool_count += 1;
  }
  
  I32 pool_id = (I32)(descriptor_pool->sets_count/RHI_VK_SETS_PER_POOL);
  I32 set_id = descriptor_pool->sets_count%RHI_VK_SETS_PER_POOL;

  I32 set_slot = 0;
  VkDescriptorSetLayout set_layout = VK_NULL_HANDLE;
  if (shader_kind == RHI_ShaderKind_Vertex) {
    set_slot = is_global ? RHI_VK_VERTEX_SHADER_GLOBAL_UNIFORM_SET_SLOT : RHI_VK_VERTEX_SHADER_INSTANCE_UNIFORM_SET_SLOT;
    set_layout = is_global ? vk_command_buffer->binded_graphics_pipeline->vertex_global_set_layout : vk_command_buffer->binded_graphics_pipeline->vertex_instance_set_layout;
  } else {
    set_slot = is_global ? RHI_VK_FRAGMENT_SHADER_GLOBAL_UNIFORM_SET_SLOT : RHI_VK_FRAGMENT_SHADER_INSTANCE_UNIFORM_SET_SLOT;
    set_layout = is_global ? vk_command_buffer->binded_graphics_pipeline->fragment_global_set_layout : vk_command_buffer->binded_graphics_pipeline->fragment_instance_set_layout;
  }

  VkDescriptorSetAllocateInfo sets_info = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
    .descriptorPool = descriptor_pool->vk_pools[pool_id],
    .descriptorSetCount = 1,
    .pSetLayouts = &set_layout,
  };

  VK_CHECK(vkAllocateDescriptorSets(_rhi_vk_state.device.logical, &sets_info, &descriptor_pool->vk_sets[pool_id][set_id]));

  VkWriteDescriptorSet write_infos[RHI_VK_MAX_UNIFORM_BUFFERS_PER_SET+RHI_VK_MAX_SAMPLERS_PER_SET] = ZeroStruct();
  I32 writes_count = 0;

  for (I32 i = 0; i < uniform_buffers_count; i += 1) {
    RHI_VK_Buffer* vk_buffer = RHI_VK_BufferFromHandle(uniform_infos[i].buffer);
    VkDescriptorBufferInfo buffer_info = {
      .buffer = vk_buffer->vk,
      .offset = uniform_infos[i].offset,
      .range = uniform_infos[i].size,
    };
    VkWriteDescriptorSet write_info = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = descriptor_pool->vk_sets[pool_id][set_id],
      .dstBinding = i,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .pBufferInfo = &buffer_info,
    };
    write_infos[writes_count] = write_info;
    writes_count += 1;
  }

  I32 start_index = writes_count;
  for (I32 i = start_index; i < start_index + sampler_count; i += 1) {
    RHI_VK_TextureSampler* vk_sampler = RHI_VK_TextureSamplerFromHandle(sampler_infos[i - start_index].sampler);
    RHI_VK_Texture* vk_texture = RHI_VK_TextureFromHandle(sampler_infos[i - start_index].texture);
  
    RHI_VK_ChangeTextureLayout(vk_command_buffer->vk[_rhi_vk_state.current_frame], vk_texture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    VkDescriptorImageInfo image_info = {
      .sampler = vk_sampler->vk,
      .imageView = vk_texture->view,
      .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };

    VkWriteDescriptorSet write_info = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = descriptor_pool->vk_sets[pool_id][set_id],
      .dstBinding = i,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .pImageInfo = &image_info,
    };
    write_infos[writes_count] = write_info;
    writes_count += 1;
  }

  vkUpdateDescriptorSets(_rhi_vk_state.device.logical, writes_count, write_infos, 0, 0);

  vkCmdBindDescriptorSets(
    vk_command_buffer->vk[_rhi_vk_state.current_frame],
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    vk_command_buffer->binded_graphics_pipeline->layout,
    set_slot, 1, &descriptor_pool->vk_sets[pool_id][set_id], 0, 0
  );
  descriptor_pool->sets_count += 1;
#endif
}

// -------------------------------------------------------------------
// -- Pipeline -------------------------------------------------------
func U32
_R_GlslangStageFromShaderType(RHI_ShaderKind kind) {
  switch (kind) {
    case RHI_ShaderKind_Vertex:   return GLSLANG_STAGE_VERTEX;
    case RHI_ShaderKind_Fragment: return GLSLANG_STAGE_FRAGMENT;

    default: Assert(0 && "Frong shader kind (Vertex and Fragment are available)"); return 0;
  }
}

func RHI_Shader
RHI_VK_CreateShader(Arena* arena, RHI_ShaderCreateInfo* info) {
  RHI_Shader result = {0};

  ScratchArena scratch = BeginScratchArena(arena);
    Str8 file_name = ConcatStr8(scratch.arena, info->file_name, Str8C(".glsl"));
    FILE* file = fopen(CFromStr8(file_name), "r");
    Assert(file);
  EndScratchArena(scratch);

  fseek(file, 0L, SEEK_END);
  U32 shader_code_size = ftell(file);
  U8* shader_code = (U8*)PushArena(arena, shader_code_size * sizeof(U8));
  rewind(file);
  fread(shader_code, shader_code_size * sizeof(U8), 1, file);
  fclose(file);

  glslang_initialize_process();

  glslang_input_t input = {0};
  input.language = GLSLANG_SOURCE_GLSL,
  input.stage = (glslang_stage_t)_R_GlslangStageFromShaderType(info->kind);
  input.client = GLSLANG_CLIENT_VULKAN;
  input.client_version = GLSLANG_TARGET_VULKAN_1_3;
  input.target_language = GLSLANG_TARGET_SPV;
  input.target_language_version = GLSLANG_TARGET_SPV_1_6;
  input.code = (const char*)shader_code;
  input.default_version = 100;
  input.default_profile = GLSLANG_NO_PROFILE;
  input.force_default_version_and_profile = 0;
  input.forward_compatible = 0;
  input.messages = GLSLANG_MSG_DEFAULT_BIT;
  input.resource = glslang_default_resource();

  LogInfo("Compiling shader \"%s\" ...\n", CFromStr8(info->file_name));

  glslang_shader_t* shader = glslang_shader_create(&input);

  if (!glslang_shader_preprocess(shader, &input)) {
    LogError("GLSL preprocessing failed");
    LogError("%s", glslang_shader_get_info_log(shader));
    LogError("%s", glslang_shader_get_info_debug_log(shader));
    glslang_shader_delete(shader);
    Assert(0);
  }

  if (!glslang_shader_parse(shader, &input)) {
    LogError("GLSL parsing failed");
    LogError("%s", glslang_shader_get_info_log(shader));
    LogError("%s", glslang_shader_get_info_debug_log(shader));
    // LogError("%s", glslang_shader_get_preprocessed_code(shader));
    glslang_shader_delete(shader);
    Assert(0);
  }

  glslang_program_t* program = glslang_program_create();
  glslang_program_add_shader(program, shader);

  if (!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT)) {
    LogError("GLSL linking failed");
    LogError("%s", glslang_program_get_info_log(program));
    LogError("%s", glslang_program_get_info_debug_log(program));
    glslang_program_delete(program);
    glslang_shader_delete(shader);
    Assert(0);
  }

  glslang_program_SPIRV_generate(program, input.stage);
  
  result.kind = info->kind;
  result.language = RHI_ShaderLanguage_SPIRV;
  result.code_size = 4 * glslang_program_SPIRV_get_size(program);
  result.code = (U8*)PushArena(arena, result.code_size * sizeof(U8));
  result.arguments = PushArena(arena, sizeof(RHI_ShaderArgumentKind)*info->arguments_count);
  for (I32 argument_index = 0; argument_index < info->arguments_count; argument_index += 1) {
    result.arguments[argument_index] = info->arguments[argument_index];
  }
  result.arguments_count = info->arguments_count;

  glslang_program_SPIRV_get(program, (U32*)result.code);

  const char* spirv_messages = glslang_program_SPIRV_get_messages(program);
  if (spirv_messages) {
    LogError("(%s) %s\b");
  }

  glslang_program_delete(program);
  glslang_shader_delete(shader);
  glslang_finalize_process();

	return result;
}

func RHI_VK_GraphicsPipeline*
RHI_VK_GraphicsPipelineFromHandle(RHI_GraphicsPipeline pipeline) {
  return RHI_VK_GraphicsPipelineArrayGetPointer(&_rhi_vk_state.graphics_pipelines, pipeline);
}

func RHI_GraphicsPipeline
RHI_VK_CreateGraphicsPipeline(RHI_GraphicsPipelineCreateInfo* pipeline_info) {
  RHI_VK_GraphicsPipeline pipeline = ZeroStruct();
  pipeline.vertex_shader = pipeline_info->vertex_shader;
  pipeline.fragment_shader = pipeline_info->fragment_shader;

  // --AlNov: @TODO Doesn't support textures (Planning to add bindless support)
  VkPushConstantRange push_constant_ranges[2] = ZeroStruct(); // --AlNov: For Vertex and Fragment shaders
  I32 push_constants_count = 0;
  if (pipeline.vertex_shader->arguments_count != 0) {
    push_constant_ranges[push_constants_count].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    for (I32 argument_index = 0; argument_index < pipeline.vertex_shader->arguments_count; argument_index += 1) {
      if (pipeline.vertex_shader->arguments[argument_index] == RHI_ShaderArgumentKind_BufferAddress) {
        push_constant_ranges[push_constants_count].size += sizeof(RHI_DeviceAddress);
      }
    }
    push_constants_count += 1;
  }
  if (pipeline.fragment_shader->arguments_count != 0) {
    push_constant_ranges[push_constants_count].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    for (I32 argument_index = 0; argument_index < pipeline.fragment_shader->arguments_count; argument_index += 1) {
      if (pipeline_info->fragment_shader->arguments[argument_index] == RHI_ShaderArgumentKind_BufferAddress) {
        push_constant_ranges[push_constants_count].size += sizeof(RHI_DeviceAddress);
      }
    }
    push_constants_count += 1;
  }
  VkPipelineLayoutCreateInfo layout_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .pushConstantRangeCount = push_constants_count,
    .pPushConstantRanges = push_constant_ranges,
  };
  VK_CHECK(vkCreatePipelineLayout(_rhi_vk_state.device.logical, &layout_info, 0, &pipeline.layout));

	U32 stride = 0;
  VkVertexInputAttributeDescription attribute_descriptions[RHI_MAX_VERTEX_ATTRIBUTES];
  for (U32 i = 0; i < pipeline_info->vertex_attributes_count; i += 1) {
		RHI_VertexAttribute* vertex_attribute = pipeline_info->vertex_attributes + i;

		attribute_descriptions[i].location = vertex_attribute->location,
		attribute_descriptions[i].binding = 0,
		attribute_descriptions[i].format = RHI_VK_GetVkFormatAttribute(vertex_attribute->format),
		attribute_descriptions[i].offset = vertex_attribute->offset,

		stride += RHI_GetSizeOfVertexAttributeFormat(vertex_attribute->format);
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
    .primitiveRestartEnable = 0,
  };

  VkPipelineRasterizationStateCreateInfo rasterization = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .depthClampEnable = 0,
    .rasterizerDiscardEnable = 0,
    .polygonMode = VK_POLYGON_MODE_FILL,
    .cullMode = 0,
    .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    .depthBiasEnable = 0,
    .lineWidth = 1.0f
  };

  VkDynamicState dynamic_states[] = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
  };

  // @TODO Support multiple attachments
  VkPipelineColorBlendAttachmentState blend_attachments[8] = {0};
  for (I32 i = 0; i < pipeline_info->color_targets_count; i += 1) {
    blend_attachments[i].blendEnable = pipeline_info->color_target_infos[i].blend_enable;
    blend_attachments[i].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    blend_attachments[i].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blend_attachments[i].colorBlendOp = VK_BLEND_OP_ADD;
    blend_attachments[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    blend_attachments[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    blend_attachments[i].alphaBlendOp = VK_BLEND_OP_ADD;
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
    .depthCompareOp = RHI_VK_GetVkFromCompareOperation(pipeline_info->depth_stencil_state.depth_compare_operation),
  };

  VkPipelineMultisampleStateCreateInfo multisample = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
  };

  VkPipelineDynamicStateCreateInfo dynamic = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    .dynamicStateCount = ArrayLength(dynamic_states),
    .pDynamicStates = dynamic_states
  };

  VkShaderModule vertex_module;
  {
    VkShaderModuleCreateInfo module_info = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = pipeline_info->vertex_shader->code_size,
      .pCode = (U32*)pipeline_info->vertex_shader->code
    };
    VK_CHECK(vkCreateShaderModule(_rhi_vk_state.device.logical, &module_info, 0, &vertex_module));
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
      .codeSize = pipeline_info->fragment_shader->code_size,
      .pCode = (U32*)pipeline_info->fragment_shader->code
    };
    VK_CHECK(vkCreateShaderModule(_rhi_vk_state.device.logical, &module_info, 0, &fragment_module));
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
    color_attachment_formats[i] = RHI_VK_GetVkFormat(pipeline_info->color_target_infos[i].format);
  }
  VkPipelineRenderingCreateInfo rendering_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
    .colorAttachmentCount = pipeline_info->color_targets_count,
    .pColorAttachmentFormats = color_attachment_formats,
    .depthAttachmentFormat = RHI_VK_GetVkFormat(pipeline_info->depth_stencil_state.depth_target_format),
  };

  VkRenderPass tmp_render_pass = RHI_VK_CreateTmpVkRenderPass(pipeline_info);

  VkGraphicsPipelineCreateInfo vk_pipeline_info = {
    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .pNext = &rendering_info,
    .stageCount = ArrayLength(shaders),
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
    .renderPass = tmp_render_pass,
    .subpass = 0,
  };
  VK_CHECK(vkCreateGraphicsPipelines(_rhi_vk_state.device.logical, 0, 1, &vk_pipeline_info, 0, &pipeline.vk));

  vkDestroyRenderPass(_rhi_vk_state.device.logical, tmp_render_pass, 0);
  vkDestroyShaderModule(_rhi_vk_state.device.logical, vertex_module, 0);
  vkDestroyShaderModule(_rhi_vk_state.device.logical, fragment_module, 0);

	return RHI_VK_GraphicsPipelineArrayAdd(&_rhi_vk_state.graphics_pipelines, pipeline);
}

func void
RHI_VK_DestroyGraphicsPipeline(RHI_GraphicsPipeline pipeline) {
#if 0
  RHI_VK_GraphicsPipeline* vk_pipeline = RHI_VK_GraphicsPipelineFromHandle(pipeline);
  vkDestroyDescriptorSetLayout(_rhi_vk_state.device.logical, vk_pipeline->vertex_global_set_layout, 0);
  vkDestroyDescriptorSetLayout(_rhi_vk_state.device.logical, vk_pipeline->fragment_global_set_layout, 0);
  vkDestroyPipelineLayout(_rhi_vk_state.device.logical, vk_pipeline->layout, 0);
  vkDestroyPipeline(_rhi_vk_state.device.logical, vk_pipeline->vk, 0);
#endif
}

func void
RHI_VK_BindGraphicsPipeline(RHI_CommandBuffer command_buffer, RHI_GraphicsPipeline pipeline) {
	RHI_VK_CommandBuffer* vk_command_buffer = RHI_VK_CommandBufferFromHandle(command_buffer);
  RHI_VK_GraphicsPipeline* vk_pipeline = RHI_VK_GraphicsPipelineFromHandle(pipeline);

	vkCmdBindPipeline(vk_command_buffer->vk[_rhi_vk_state.current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline->vk);
	vk_command_buffer->binded_graphics_pipeline = vk_pipeline;
}

// -------------------------------------------------------------------
// -- Render Pass ----------------------------------------------------
func B32
RHI_RenderPassEqual(RHI_RenderPass a, RHI_RenderPass b) {
  B32 result = 1;

  if (a.color_targets_count != b.color_targets_count) result = 0;

  for (I32 i = 0; i < a.color_targets_count; i += 1) {
    RHI_VK_Texture* vk_texture_a = RHI_VK_TextureFromHandle(a.color_targets[i].texture);
    RHI_VK_Texture* vk_texture_b = RHI_VK_TextureFromHandle(b.color_targets[i].texture);

    if (vk_texture_a->format != vk_texture_b->format) {
      result = 0;
      break;
    }
    else if (a.color_targets[i].load_operation != b.color_targets[i].load_operation) {
      result = 0;
      break;
    }
    else if (a.color_targets[i].store_operation != b.color_targets[i].store_operation) {
      result = 0;
      break;
    }
    else if (!EqualVec4F32(a.color_targets[i].clear_color, b.color_targets[i].clear_color)) {
      result = 0;
      break;
    }
  }
  
  RHI_VK_Texture* vk_texture_depth_stencil_a = RHI_VK_TextureFromHandle(a.depth_stencil_target.texture);
  RHI_VK_Texture* vk_texture_depth_stencil_b = RHI_VK_TextureFromHandle(b.depth_stencil_target.texture);
  if (vk_texture_depth_stencil_a->format != vk_texture_depth_stencil_b->format) {
    result = 0;
  }
  else if (a.depth_stencil_target.load_operation != b.depth_stencil_target.load_operation) {
    result = 0;
  }
  else if (a.depth_stencil_target.store_operation != b.depth_stencil_target.store_operation) {
    result = 0;
  }
  else if (a.depth_stencil_target.clear_depth != b.depth_stencil_target.clear_depth) {
    result = 0;
  }

  return result;
}

func RHI_VK_RenderPass*
RHI_VK_CreateRenderPass(U32 color_targets_count, RHI_ColorTarget* color_targets, RHI_DepthStencilTarget* depth_stencil_target) {
  RHI_VK_RenderPass* result = 0;

  RHI_VK_RenderPass render_pass = ZeroStruct();
  VkAttachmentDescription attachments[RHI_MAX_COLOR_ATTACHMENTS + 1] = ZeroStruct();
  VkAttachmentReference references[RHI_MAX_COLOR_ATTACHMENTS + 1] = ZeroStruct();

  render_pass.header.color_targets_count = color_targets_count;
  for (I32 i = 0; i < color_targets_count; i += 1) {
    RHI_ColorTarget* target = color_targets + i;
    RHI_VK_Texture* vk_texture = RHI_VK_TextureFromHandle(target->texture);
    attachments[i].flags = 0;
    attachments[i].format = RHI_VK_GetVkFormat(vk_texture->format);
    attachments[i].samples = VK_SAMPLE_COUNT_1_BIT,
    attachments[i].loadOp = RHI_VK_GetVkAttachmentLoadOperation(target->load_operation);
    attachments[i].storeOp = RHI_VK_GetVkAttachmentStoreOperation(target->store_operation);
    attachments[i].stencilLoadOp = 0;
    attachments[i].stencilStoreOp = 0;
    attachments[i].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachments[i].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    references[i].attachment = i;
    references[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    render_pass.header.color_targets[i] = *target;
  }

  if (depth_stencil_target != 0) {
    RHI_VK_Texture* vk_texture = RHI_VK_TextureFromHandle(depth_stencil_target->texture);
    attachments[color_targets_count] = (VkAttachmentDescription){
      .flags = 0,
      .format = RHI_VK_GetVkFormat(vk_texture->format),
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = RHI_VK_GetVkAttachmentLoadOperation(depth_stencil_target->load_operation),
      .storeOp = RHI_VK_GetVkAttachmentStoreOperation(depth_stencil_target->store_operation),
      .stencilLoadOp = 0,
      .stencilStoreOp = 0,
      .initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
      .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    references[color_targets_count] = (VkAttachmentReference){
      .attachment = color_targets_count,
      .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    render_pass.header.depth_stencil_target = *depth_stencil_target;
  }

  B32 found = 0;
  for (I32 i = 0; i < _rhi_vk_state.render_passes.length; i += 1) {
    RHI_VK_RenderPass* vk_render_pass = RHI_VK_RenderPassArrayGetPointer(&_rhi_vk_state.render_passes, i);
    if (RHI_RenderPassEqual(render_pass.header, vk_render_pass->header)) {
      found = 1;
      result = vk_render_pass;
      break;
    }
  }

  if (!found) {
    VkSubpassDescription subpass = {
      .flags = 0,
      .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .inputAttachmentCount = 0,
      .pInputAttachments = 0,
      .colorAttachmentCount = color_targets_count,
      .pColorAttachments = references,
      .pResolveAttachments = 0,
      .pDepthStencilAttachment = (depth_stencil_target != 0) ? references + color_targets_count : 0,
      .preserveAttachmentCount = 0,
      .pPreserveAttachments = 0,
    };

    VkRenderPassCreateInfo render_pass_info = {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .attachmentCount = color_targets_count + (U32)(depth_stencil_target != 0),
      .pAttachments = attachments,
      .subpassCount = 1,
      .pSubpasses = &subpass,
      .dependencyCount = 0,
      .pDependencies = 0,
    };

    vkCreateRenderPass(_rhi_vk_state.device.logical, &render_pass_info, 0, &render_pass.vk);
    I32 array_slot = RHI_VK_RenderPassArrayAdd(&_rhi_vk_state.render_passes, render_pass);
    result = RHI_VK_RenderPassArrayGetPointer(&_rhi_vk_state.render_passes, array_slot);
  }

  result->framebuffer = RHI_VK_CreateFramebuffer(result, color_targets, color_targets_count, depth_stencil_target);

  return result;
}

func VkRenderPass RHI_VK_CreateTmpVkRenderPass(RHI_GraphicsPipelineCreateInfo* pipeline_info) {
  VkRenderPass result = 0;

  VkAttachmentDescription attachments[RHI_MAX_COLOR_ATTACHMENTS + 1] = ZeroStruct();
  VkAttachmentReference references[RHI_MAX_COLOR_ATTACHMENTS + 1] = ZeroStruct();

  for (I32 i = 0; i < pipeline_info->color_targets_count; i += 1) {
    RHI_GraphicsPipelineColorTargetInfo* target_info = pipeline_info->color_target_infos + i;
    attachments[i].flags = 0;
    attachments[i].format = RHI_VK_GetVkFormat(target_info->format);
    attachments[i].samples = VK_SAMPLE_COUNT_1_BIT,
    attachments[i].loadOp = 0,
    attachments[i].storeOp = 0,
    attachments[i].stencilLoadOp = 0;
    attachments[i].stencilStoreOp = 0;
    attachments[i].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachments[i].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    references[i].attachment = i;
    references[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  }

  RHI_PipelineDepthStencilState* depth_stencil_info = &pipeline_info->depth_stencil_state;
  B32 has_depth_stencil = depth_stencil_info->depth_test_enable || depth_stencil_info->depth_write_enable;
  if (has_depth_stencil) {
    attachments[pipeline_info->color_targets_count] = (VkAttachmentDescription){
      .flags = 0,
      .format = RHI_VK_GetVkFormat(depth_stencil_info->depth_target_format),
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = 0,
      .storeOp = 0,
      .stencilLoadOp = 0,
      .stencilStoreOp = 0,
      .initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
      .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    references[pipeline_info->color_targets_count] = (VkAttachmentReference){
      .attachment = pipeline_info->color_targets_count,
      .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };
  }

  VkSubpassDescription subpass = {
    .flags = 0,
    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
    .inputAttachmentCount = 0,
    .pInputAttachments = 0,
    .colorAttachmentCount = pipeline_info->color_targets_count,
    .pColorAttachments = references,
    .pResolveAttachments = 0,
    .pDepthStencilAttachment = (has_depth_stencil) ? references + pipeline_info->color_targets_count : 0,
    .preserveAttachmentCount = 0,
    .pPreserveAttachments = 0,
  };

  VkRenderPassCreateInfo render_pass_info = {
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    .attachmentCount = pipeline_info->color_targets_count + has_depth_stencil,
    .pAttachments = attachments,
    .subpassCount = 1,
    .pSubpasses = &subpass,
    .dependencyCount = 0,
    .pDependencies = 0,
  };

  vkCreateRenderPass(_rhi_vk_state.device.logical, &render_pass_info, 0, &result);

  return result;
}

func RHI_RenderPass*
RHI_VK_BeginRenderPass(RHI_CommandBuffer command_buffer, U32 color_targets_count, RHI_ColorTarget* color_targets, RHI_DepthStencilTarget* depth_stencil_target, RHI_Resource* resources, I32 resources_count) {
	RHI_VK_CommandBuffer* vk_command_buffer = RHI_VK_CommandBufferFromHandle(command_buffer);

  RHI_VK_RenderPass* render_pass = RHI_VK_CreateRenderPass(color_targets_count, color_targets, depth_stencil_target);

  if (render_pass->framebuffer != 0) {
    VkClearValue clear_values[RHI_MAX_COLOR_ATTACHMENTS + 1] = ZeroStruct();
    for (I32 i = 0; i < color_targets_count; i += 1) {
      clear_values[i].color.float32[0] = color_targets[i].clear_color.r;
      clear_values[i].color.float32[1] = color_targets[i].clear_color.g;
      clear_values[i].color.float32[2] = color_targets[i].clear_color.b;
      clear_values[i].color.float32[3] = color_targets[i].clear_color.a;

      RHI_VK_Texture* vk_attachment_texture = RHI_VK_TextureFromHandle(color_targets[i].texture);
      RHI_VK_ChangeTextureLayout(vk_command_buffer->vk[_rhi_vk_state.current_frame], vk_attachment_texture, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    }

    if (depth_stencil_target != 0) {
      clear_values[color_targets_count].depthStencil.depth = depth_stencil_target->clear_depth;

      RHI_VK_Texture* vk_depth_texture = RHI_VK_TextureFromHandle(depth_stencil_target->texture);
      RHI_VK_ChangeTextureLayout(vk_command_buffer->vk[_rhi_vk_state.current_frame], vk_depth_texture, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    }

    VkRenderPassBeginInfo begin_info = {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .renderPass = render_pass->vk,
      .framebuffer = render_pass->framebuffer->vk,
      .renderArea = (VkRect2D){
        .offset.x = 0,
        .offset.y = 0,
        .extent.width = render_pass->framebuffer->size.x,
        .extent.height = render_pass->framebuffer->size.y,
      },
      .clearValueCount = color_targets_count + (U32)(depth_stencil_target != 0),
      .pClearValues = clear_values,
    };

    vkCmdBeginRenderPass(vk_command_buffer->vk[_rhi_vk_state.current_frame], &begin_info, VK_SUBPASS_CONTENTS_INLINE);
    vk_command_buffer->active_render_pass = render_pass;
  }

	return 0; // @TODO
}

func void
RHI_VK_EndRenderPass(RHI_CommandBuffer command_buffer, RHI_RenderPass* render_pass) {
	RHI_VK_CommandBuffer* vk_command_buffer = RHI_VK_CommandBufferFromHandle(command_buffer);

  if (vk_command_buffer->active_render_pass != 0) {
    vkCmdEndRenderPass(vk_command_buffer->vk[_rhi_vk_state.current_frame]);
    vk_command_buffer->active_render_pass = 0;
  }
}

func void
RHI_VK_EndRenderPassOld(RHI_CommandBuffer command_buffer, RHI_RenderPass* render_pass) {
	RHI_VK_CommandBuffer* vk_command_buffer = RHI_VK_CommandBufferFromHandle(command_buffer);

  vkCmdEndRendering(vk_command_buffer->vk[_rhi_vk_state.current_frame]);
}

// -------------------------------------------------------------------
// -- Framebuffer ----------------------------------------------------
func B32
RHI_VK_EqualFramebuffer(RHI_VK_Framebuffer a, RHI_VK_Framebuffer b) {
  B32 result = 1;

  if (a.color_attachments_count != b.color_attachments_count) {
    result = 0;
  }
  else if (a.depth_stencil_attachment != b.depth_stencil_attachment) {
    result = 0;
  }
  else if (a.size.x != b.size.x) {
    result = 0;
  }
  else if (a.size.y != b.size.y) {
    result = 0;
  }

  for (I32 i = 0; i < a.color_attachments_count; i += 1) {
    if (a.color_attachments[i] != b.color_attachments[i]) {
      result = 0;
      break;
    }
  }

  return result;
}

func RHI_VK_Framebuffer*
RHI_VK_CreateFramebuffer(RHI_VK_RenderPass* render_pass, RHI_ColorTarget* color_targets, I32 color_targets_count, RHI_DepthStencilTarget* depth_stencil_target) {
  RHI_VK_Framebuffer* result = 0;
  RHI_VK_Framebuffer framebuffer = ZeroStruct();

  VkImageView attachments[RHI_MAX_COLOR_ATTACHMENTS] = ZeroStruct();

  framebuffer.color_attachments_count = color_targets_count;
  framebuffer.size = MakeVec2I32(I32_MAX, I32_MAX);

  for (I32 i = 0; i < color_targets_count; i += 1) {
    RHI_ColorTarget* target = color_targets + i;
    RHI_VK_Texture* vk_texture = RHI_VK_TextureFromHandle(target->texture);
    attachments[i] = vk_texture->view;
    framebuffer.color_attachments[i] = vk_texture->view;
    framebuffer.size.x = Min(framebuffer.size.x, vk_texture->size.x);
    framebuffer.size.y = Min(framebuffer.size.y, vk_texture->size.y);
  }

  if (depth_stencil_target != 0) {
    RHI_VK_Texture* vk_texture = RHI_VK_TextureFromHandle(depth_stencil_target->texture);
    attachments[color_targets_count] = vk_texture->view;
    framebuffer.depth_stencil_attachment = vk_texture->view;
    framebuffer.size.x = Min(framebuffer.size.x, vk_texture->size.x);
    framebuffer.size.y = Min(framebuffer.size.y, vk_texture->size.y);
  }

  if (framebuffer.size.x != 0 && framebuffer.size.y != 0) {
    B32 found = 0;
    for (I32 i = 0; i < _rhi_vk_state.framebuffers.length; i += 1) {
      RHI_VK_Framebuffer* compare = RHI_VK_FramebufferArrayGetPointer(&_rhi_vk_state.framebuffers, i);
      if (RHI_VK_EqualFramebuffer(framebuffer, *compare)) {
        found = 1;
        result = compare;
      }
    }

    if (!found) {
      VkFramebufferCreateInfo framebuffer_info = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .flags = 0,
        .renderPass = render_pass->vk,
        .attachmentCount = color_targets_count + (U32)(depth_stencil_target != 0),
        .pAttachments = attachments,
        .width = framebuffer.size.x,
        .height = framebuffer.size.y,
        .layers = 1,
      };

      vkCreateFramebuffer(_rhi_vk_state.device.logical, &framebuffer_info, 0, &framebuffer.vk);
      I32 array_slot = RHI_VK_FramebufferArrayAdd(&_rhi_vk_state.framebuffers, framebuffer);
      result = RHI_VK_FramebufferArrayGetPointer(&_rhi_vk_state.framebuffers, array_slot);
    }
  }

  return result;
}

// -------------------------------------------------------------------
// -- Draw -----------------------------------------------------------
func void
RHI_VK_SetViewport(RHI_CommandBuffer command_buffer, RectI32 viewport) {
	RHI_VK_CommandBuffer* vk_command_buffer = RHI_VK_CommandBufferFromHandle(command_buffer);

	VkViewport vk_viewport = {
		.x = viewport.x,
		.y = viewport.h,
		.width = viewport.w,
		.height = -viewport.h,
    .minDepth = 0.0f,
    .maxDepth = 1.0f,
	};
	vkCmdSetViewport(vk_command_buffer->vk[_rhi_vk_state.current_frame], 0, 1, &vk_viewport);
  vk_command_buffer->current_viewport = vk_viewport;
}

func void
RHI_VK_SetScissor(RHI_CommandBuffer command_buffer, RectI32 scissor) {
	RHI_VK_CommandBuffer* vk_command_buffer = RHI_VK_CommandBufferFromHandle(command_buffer);
	VkRect2D vk_scissor = {
		.offset.x = scissor.x,
		.offset.y = scissor.y,
		.extent.width = scissor.w,
		.extent.height = scissor.h,
	};
	vkCmdSetScissor(vk_command_buffer->vk[_rhi_vk_state.current_frame], 0, 1, &vk_scissor);
}

func void
RHI_VK_DrawPrimitives(RHI_CommandBuffer command_buffer, U32 vertex_count, U32 instance_count, U32 first_vertex, U32 first_instance) {
	RHI_VK_CommandBuffer* vk_command_buffer = RHI_VK_CommandBufferFromHandle(command_buffer);

  if (vk_command_buffer->active_render_pass != 0) {
    vkCmdDraw(vk_command_buffer->vk[_rhi_vk_state.current_frame], vertex_count, instance_count, first_vertex, first_instance);
  }
}

func void
RHI_VK_DrawIndexedPrimitives(RHI_CommandBuffer command_buffer, U32 index_count, U32 instance_count, U32 first_index, I32 vertex_offset, U32 first_instance) {
	RHI_VK_CommandBuffer* vk_command_buffer = RHI_VK_CommandBufferFromHandle(command_buffer);

  if (vk_command_buffer->active_render_pass != 0) {
    vkCmdDrawIndexed(vk_command_buffer->vk[_rhi_vk_state.current_frame], index_count, instance_count, first_index, vertex_offset, first_instance);
  }
}

func void
RHI_VK_PresentTexture(RHI_CommandBuffer command_buffer, RHI_Texture texture) {
#if 0
  RHI_VK_CommandBuffer* vk_command_buffer = RHI_VK_CommandBufferFromHandle(command_buffer);
  RHI_VK_Texture* vk_texture = RHI_VK_TextureFromHandle(texture);

  if (vk_texture->from_swapchain) {
    VkPresentInfoKHR present_info = {
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &_rhi_vk_state.swapchain.frame_resources[_rhi_vk_state.current_frame].release_semaphore, // @TODO change location of release_semaphore
      .swapchainCount = 1,
      .pSwapchains = &_rhi_vk_state.swapchain.vk,
      .pImageIndices = &_rhi_vk_state.current_target,
    };

    vkQueuePresentKHR(_rhi_vk_state.device.graphics_queue, &present_info);
  }
  else {
    LogWarning("Trying to present image not from swapchain\n");
    return;
  }
#endif
}

// -------------------------------------------------------------------
// -- Texture --------------------------------------------------------
func RHI_VK_Texture*
RHI_VK_TextureFromHandle(RHI_Texture vk) {
  return RHI_VK_TextureArrayGetPointer(&_rhi_vk_state.textures, vk);
}

func RHI_Texture
RHI_VK_CreateTexture(RHI_TextureCreateInfo* info) {
  RHI_VK_Texture texture = {0};
  texture.format = info->format;
  texture.size.x = info->width;
  texture.size.y = info->height;

  VkImageCreateInfo image_info = {0};
  image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image_info.samples = VK_SAMPLE_COUNT_1_BIT;
  image_info.imageType = RHI_VK_GetVkImageType(info->kind);
  image_info.extent.width = info->width;
  image_info.extent.height= info->height;
  image_info.extent.depth = info->depth;
  image_info.mipLevels = 1;
  image_info.arrayLayers = 1;
  image_info.format = RHI_VK_GetVkFormat(info->format);
  image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
  image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_info.usage = RHI_VK_GetVkImageUsageFlags(info->usage_flags);
  image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  image_info.samples = VK_SAMPLE_COUNT_1_BIT;
  if (vkCreateImage(_rhi_vk_state.device.logical, &image_info, 0, &texture.image) != VK_SUCCESS) {
    LogError("Cannot create Image for Texture.\n");
    return RHI_nil;
  }

  VkMemoryRequirements mem_requirements = {0};
  vkGetImageMemoryRequirements(_rhi_vk_state.device.logical, texture.image, &mem_requirements);

  VkMemoryAllocateInfo mem_info = {0};
  mem_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  mem_info.allocationSize  = mem_requirements.size;
  mem_info.memoryTypeIndex = RHI_VK_FindMemoryTypeIndex(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  VK_CHECK(vkAllocateMemory(_rhi_vk_state.device.logical, &mem_info, 0, &texture.memory));

  VK_CHECK(vkBindImageMemory(_rhi_vk_state.device.logical, texture.image, texture.memory, 0));

  VkImageAspectFlags texture_aspect = 0;
  if((info->usage_flags & RHI_TEXTURE_USAGE_FLAG_COLOR_ATTACHMENT) == RHI_TEXTURE_USAGE_FLAG_COLOR_ATTACHMENT) {
    texture_aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    texture.aspect_mask = texture_aspect;
  }
  else if ((info->usage_flags & RHI_TEXTURE_USAGE_FLAG_DEPTH_STENCIL_ATTACHMENT) == RHI_TEXTURE_USAGE_FLAG_DEPTH_STENCIL_ATTACHMENT) {
    texture_aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
    texture.aspect_mask = texture_aspect;
  }
  else {
    texture_aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    texture.aspect_mask = texture_aspect;
  }
  
  VkImageViewCreateInfo view_info = {0};
  view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  view_info.image = texture.image;
  view_info.viewType = VK_IMAGE_VIEW_TYPE_2D; // @TODO
  view_info.format = RHI_VK_GetVkFormat(info->format);
  view_info.subresourceRange.aspectMask = texture_aspect;
  view_info.subresourceRange.baseMipLevel = 0;
  view_info.subresourceRange.levelCount = 1;
  view_info.subresourceRange.baseArrayLayer = 0;
  view_info.subresourceRange.layerCount = 1;

  VK_CHECK(vkCreateImageView(_rhi_vk_state.device.logical, &view_info, 0, &texture.view));

  if (_rhi_vk_state.textures_free_list.length > 0) {
    I32 index = RHI_VK_TextureFreeListRemoveSwapback(&_rhi_vk_state.textures_free_list, 0);
    RHI_VK_TextureArraySet(&_rhi_vk_state.textures, index, texture);
    return index;
  }
  else {
    return RHI_VK_TextureArrayAdd(&_rhi_vk_state.textures, texture);  
  }
}

func B32
RHI_VK_DestroyTexture(RHI_Texture texture) {
  RHI_VK_Texture* vk_texture = RHI_VK_TextureFromHandle(texture);

  vkDestroyImageView(_rhi_vk_state.device.logical, vk_texture->view, 0);
  if (!vk_texture->from_swapchain) {
    vkFreeMemory(_rhi_vk_state.device.logical, vk_texture->memory, 0);
    vkDestroyImage(_rhi_vk_state.device.logical, vk_texture->image, 0);
  }

  RHI_VK_TextureArraySet(&_rhi_vk_state.textures, texture, RHI_VK_TextureNil);
  RHI_VK_TextureFreeListAdd(&_rhi_vk_state.textures_free_list, texture);

  return 1;
}

func void
RHI_VK_LoadDataToTexture(U8* data, U64 data_size, RHI_Texture texture) {
  // --AlNov: @TODO Empty
}

func void
RHI_VK_CopyTexture(RHI_CommandBuffer command_buffer, RHI_Texture source, RHI_Texture destination) {
  RHI_VK_CommandBuffer* vk_command_buffer = RHI_VK_CommandBufferFromHandle(command_buffer);
  RHI_VK_Texture* vk_source = RHI_VK_TextureFromHandle(source);
  RHI_VK_Texture* vk_destination = RHI_VK_TextureFromHandle(destination);

  RHI_VK_ChangeTextureLayout(vk_command_buffer->vk[_rhi_vk_state.current_frame], vk_source, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  RHI_VK_ChangeTextureLayout(vk_command_buffer->vk[_rhi_vk_state.current_frame], vk_destination, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

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
    vk_command_buffer->vk[_rhi_vk_state.current_frame],
    vk_source->image,
    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    vk_destination->image,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    1, &blit_info, VK_FILTER_LINEAR
  );
}

func U64
RHI_VK_CopyTextureToBuffer(RHI_CommandBuffer command_buffer, RHI_Texture texture, RHI_Buffer buffer) {
  RHI_VK_CommandBuffer* vk_command_buffer = RHI_VK_CommandBufferFromHandle(command_buffer);
  RHI_VK_Texture* vk_texture = RHI_VK_TextureFromHandle(texture);
  RHI_VK_Buffer* vk_buffer = RHI_VK_BufferFromHandle(buffer);

  RHI_VK_ChangeTextureLayout(vk_command_buffer->vk[_rhi_vk_state.current_frame], vk_texture, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

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
    vk_command_buffer->vk[_rhi_vk_state.current_frame],
    vk_texture->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    vk_buffer->vk, 1, &copy_info
  );

  return vk_buffer->size;
}

func void
RHI_VK_CopyBufferToTexture(RHI_CommandBuffer command_buffer, RHI_Buffer buffer, U64 offset, U64 size, RHI_Texture texture) {
  RHI_VK_Buffer* vk_buffer = RHI_VK_BufferFromHandle(buffer);
  RHI_VK_Texture* vk_texture = RHI_VK_TextureFromHandle(texture);

  VkCommandBuffer single_cmd = RHI_VK_BeginSingleCmd();
  {
    RHI_VK_ChangeTextureLayout(single_cmd, vk_texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

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
    vkCmdCopyBufferToImage(single_cmd, vk_buffer->vk, vk_texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_info);
    RHI_VK_ChangeTextureLayout(single_cmd, vk_texture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  }
  RHI_VK_EndSingleCmd(single_cmd);
}

func RHI_TextureFormat
RHI_VK_GetTextureFormat(RHI_Texture texture) {
  return RHI_VK_TextureFromHandle(texture)->format;
}

func Vec2I32
RHI_VK_GetTextureDimension(RHI_Texture texture) {
  return RHI_VK_TextureFromHandle(texture)->size;
}

func void
RHI_VK_ChangeTextureLayout(VkCommandBuffer cmd, RHI_VK_Texture* texture, VkImageLayout new_layout) {
  if (texture->layout == new_layout) return;

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

  if (texture->layout == VK_IMAGE_LAYOUT_UNDEFINED) {
    source_stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    image_barrier.srcAccessMask = 0;
    image_barrier.oldLayout = texture->layout;
  }
  else if (texture->layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
    source_stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    image_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    image_barrier.oldLayout = texture->layout;
  }
  else if (texture->layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    source_stages = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    image_barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    image_barrier.oldLayout = texture->layout;
  }
  else if (texture->layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    source_stages = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    image_barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    image_barrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  }
  else if (texture->layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
    source_stages = VK_PIPELINE_STAGE_TRANSFER_BIT;
    image_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    image_barrier.oldLayout = texture->layout;
  }
  else if (texture->layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    source_stages = VK_PIPELINE_STAGE_TRANSFER_BIT;
    image_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    image_barrier.oldLayout = texture->layout;
  }
  else if(texture->layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
    source_stages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    image_barrier.srcAccessMask = 0;
    image_barrier.oldLayout = texture->layout;
  }
  else {
    AssertMessage(0, "Image Layout is not supproted");
  }

  if (new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
    destination_stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    image_barrier.newLayout = new_layout;
  }
  else if (new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    destination_stages = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    image_barrier.newLayout = new_layout;
  }
  else if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    destination_stages = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    image_barrier.newLayout = new_layout;
  }
  else if (new_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
    destination_stages = VK_PIPELINE_STAGE_TRANSFER_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    image_barrier.newLayout = new_layout;
  }
  else if (new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    destination_stages = VK_PIPELINE_STAGE_TRANSFER_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    image_barrier.newLayout = new_layout;
  }
  else if (new_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
    destination_stages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    image_barrier.dstAccessMask = 0;
    image_barrier.newLayout = new_layout;
  }
  else {
    AssertMessage(0, "Image Layout is not supproted");
  }

  vkCmdPipelineBarrier(cmd, source_stages, destination_stages, 0, 0, 0, 0, 0, 1, &image_barrier);

  texture->layout = new_layout;
}

func RHI_VK_TextureSampler*
RHI_VK_TextureSamplerFromHandle(RHI_TextureSampler sampler) {
  return RHI_VK_TextureSamplerArrayGetPointer(&_rhi_vk_state.samplers, sampler);
}

func RHI_TextureSampler
RHI_VK_CreateTextureSampler(RHI_TextureSamplerCreateInfo* info) {
  RHI_VK_TextureSampler sampler = {0};

  VkSamplerCreateInfo sampler_info = {
    .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
    .magFilter = RHI_VK_GetVkFilter(info->mag_filter),
    .minFilter = RHI_VK_GetVkFilter(info->min_filter),
    .mipmapMode = RHI_VK_GetVkSamplerMipmapMode(info->mipmap_mode),
    .addressModeU = RHI_VK_GetVkSamplerAddressMode(info->address_mode_u),
    .addressModeV = RHI_VK_GetVkSamplerAddressMode(info->address_mode_v),
    .addressModeW = RHI_VK_GetVkSamplerAddressMode(info->address_mode_w),
    .mipLodBias = info->mip_lod_bias,
    .anisotropyEnable = info->anisotropy_enable,
    .maxAnisotropy = info->max_anisotropy,
    .compareEnable = info->compare_enable,
    .compareOp = RHI_VK_GetVkFromCompareOperation(info->compare_operation),
    .minLod = info->min_lod,
    .maxLod = info->max_lod,
  };
  VK_CHECK(vkCreateSampler(_rhi_vk_state.device.logical, &sampler_info, 0, &sampler.vk));

  return RHI_VK_TextureSamplerArrayAdd(&_rhi_vk_state.samplers, sampler);
}

// -------------------------------------------------------------------
// -- Command Buffer -------------------------------------------------
func VkCommandBuffer
RHI_VK_BeginSingleCmd(void) {
  VkCommandBufferAllocateInfo allocate_info = {0};
  allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocate_info.commandPool = _rhi_vk_state.command_pool;
  allocate_info.commandBufferCount = 1;

  VkCommandBuffer command_buffer;
  VK_CHECK(vkAllocateCommandBuffers(_rhi_vk_state.device.logical, &allocate_info, &command_buffer));

  VkCommandBufferBeginInfo begin_info = {0};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  VK_CHECK(vkBeginCommandBuffer(command_buffer, &begin_info));

  return command_buffer; 
}

func void
RHI_VK_EndSingleCmd(VkCommandBuffer cmd) {
  VK_CHECK(vkEndCommandBuffer(cmd));

  VkSubmitInfo submit_info = {0};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &cmd;

  VK_CHECK(vkQueueSubmit(_rhi_vk_state.device.graphics_queue, 1, &submit_info, 0));
  VK_CHECK(vkQueueWaitIdle(_rhi_vk_state.device.graphics_queue));

  vkFreeCommandBuffers(_rhi_vk_state.device.logical, _rhi_vk_state.command_pool, 1, &cmd);
}


// -------------------------------------------------------------------
// -- Global State ---------------------------------------------------
func B32
RHI_VK_Init(OS_Window* window) {
  _rhi_vk_state.arena = AllocateArena(Gigabytes(32), Kilobytes(64));
  _rhi_vk_state.buffers = RHI_VK_BufferArrayAllocate(_rhi_vk_state.arena, 32);
  _rhi_vk_state.buffers.elements[0] = (RHI_VK_Buffer){0};
  _rhi_vk_state.render_passes = RHI_VK_RenderPassArrayAllocate(_rhi_vk_state.arena, 32);
  // --AlNov 11 January 2026: @TODO Doesn't delete framebuffers when texture is deleted (Swapchain Recreation)
  _rhi_vk_state.framebuffers = RHI_VK_FramebufferArrayAllocate(_rhi_vk_state.arena, 256);
  _rhi_vk_state.graphics_pipelines = RHI_VK_GraphicsPipelineArrayAllocate(_rhi_vk_state.arena, 32);
  _rhi_vk_state.graphics_pipelines.elements[0] = (RHI_VK_GraphicsPipeline){0};
  _rhi_vk_state.command_buffers = RHI_VK_CommandBufferArrayAllocate(_rhi_vk_state.arena, 16);
  _rhi_vk_state.command_buffers.elements[0] = (RHI_VK_CommandBuffer){0};
  _rhi_vk_state.samplers = RHI_VK_TextureSamplerArrayAllocate(_rhi_vk_state.arena, 32);
  _rhi_vk_state.samplers.elements[0] = (RHI_VK_TextureSampler){0};
  _rhi_vk_state.textures = RHI_VK_TextureArrayAllocate(_rhi_vk_state.arena, 32);
  _rhi_vk_state.textures.elements[0] = (RHI_VK_Texture){0};
  _rhi_vk_state.textures_free_list = RHI_VK_TextureFreeListAllocate(_rhi_vk_state.arena, 32);
  _rhi_vk_state.textures_free_list.elements[0] = 1;

  VkApplicationInfo app_info = ZeroStruct();
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "Ignis_Vulkan_RHI";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "Ignis";
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

  const char* validation_layers[] = {
#if IGNIS_VULKAN_DEBUG
    "VK_LAYER_KHRONOS_validation",
#endif
  };

  VkInstanceCreateInfo instance_info = ZeroStruct();
  instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_info.pApplicationInfo = &app_info;
  instance_info.enabledLayerCount = ArrayLength(validation_layers);
  instance_info.ppEnabledLayerNames = validation_layers;
  instance_info.enabledExtensionCount = ArrayLength(extension_names);
  instance_info.ppEnabledExtensionNames = extension_names;

#if IGNIS_VULKAN_DEBUG
  VkDebugUtilsMessengerCreateInfoEXT messenger_info = RHI_VK_PopulateDebugMessengerCreateInfo();
  instance_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&messenger_info;
#endif // IGNIS_VULKAN_DEBUG

  VK_CHECK(vkCreateInstance(&instance_info, 0, &_rhi_vk_state.instance));

#if IGNIS_VULKAN_DEBUG
  VK_CHECK(RHI_VK_CreateDebugMessenger(_rhi_vk_state.instance, &_rhi_vk_state.debug_messenger));
#endif // IGNIS_VULKAN_DEBUG

  RHI_VK_CreateDevice();
  RHI_VK_CreateSwapchain(window);

  VkCommandPoolCreateInfo command_pool_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
    .queueFamilyIndex = _rhi_vk_state.device.graphics_queue_index
  };
  VK_CHECK(vkCreateCommandPool(_rhi_vk_state.device.logical, &command_pool_info, 0, &_rhi_vk_state.command_pool));

  LogInfo("Rendered is initialized\n");
	return 0;
}

func B32
RHI_VK_Shutdown(void) {
  vkDeviceWaitIdle(_rhi_vk_state.device.logical);

  for (I32 i = 0; i < _rhi_vk_state.textures.length; i += 1) {
    RHI_VK_DestroyTexture(i);
  }

  for (I32 i = 0; i < _rhi_vk_state.command_buffers.length; i += 1) {
    RHI_VK_ReleaseCommandBuffer(i);
  }
  
  for (I32 i = 0; i < _rhi_vk_state.graphics_pipelines.length; i += 1) {
    RHI_VK_DestroyGraphicsPipeline(i);
  }

  vkDestroyCommandPool(_rhi_vk_state.device.logical, _rhi_vk_state.command_pool, 0);

  RHI_VK_DestroySwapchain();

  vkDestroyDevice(_rhi_vk_state.device.logical, 0);
#if IGNIS_VULKAN_DEBUG
  RHI_VK_DestroyDebugUtilsMessenger(_rhi_vk_state.instance, _rhi_vk_state.debug_messenger, 0);
#endif // IGNIS_VULKAN_DEBUG
  vkDestroyInstance(_rhi_vk_state.instance, 0);

  FreeArena(_rhi_vk_state.arena);

	return 1;
}

func void
RHI_VK_HandleResize(OS_Window* window) {
  RHI_VK_RecreateSwapchain(window);
}

// -------------------------------------------------------------------
// Debug Tools
#if IGNIS_VULKAN_DEBUG
VKAPI_ATTR VkBool32 VKAPI_CALL
RHI_VK_DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
  LogWarning("VK_VALIDATION: %s\n", pCallbackData->pMessage);
  return VK_FALSE;
}

func VkDebugUtilsMessengerCreateInfoEXT
RHI_VK_PopulateDebugMessengerCreateInfo(void) {
	VkDebugUtilsMessengerCreateInfoEXT messenger_info = {0};
  messenger_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  messenger_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  messenger_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
  messenger_info.pfnUserCallback = RHI_VK_DebugCallback;
  messenger_info.pUserData = 0;

	return messenger_info;
}

func VkResult
RHI_VK_CreateDebugUtilsMessenger(VkInstance instance, VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMesseneger) {
  PFN_vkCreateDebugUtilsMessengerEXT f = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

  if (f != 0) {
    return f(instance, pCreateInfo, pAllocator, pDebugMesseneger);
  }
  else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

func void
RHI_VK_DestroyDebugUtilsMessenger(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, VkAllocationCallbacks* pAllocator) {
  PFN_vkDestroyDebugUtilsMessengerEXT f = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

  if (f != 0) {
    f(instance, debugMessenger, pAllocator);
  }
}

func VkResult
RHI_VK_CreateDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT* debugMessenger) {
  VkDebugUtilsMessengerCreateInfoEXT messengerInfo = RHI_VK_PopulateDebugMessengerCreateInfo();
  return RHI_VK_CreateDebugUtilsMessenger(instance, &messengerInfo, 0, debugMessenger);
}
#endif // IGNIS_VULKAN_DEBUG
