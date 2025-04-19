#include "r_vk_swapchain.h"
#include "render/vulkan/r_vk_utils.h"
#include "third_party/vulkan/include/vulkan_core.h"

func void
R_VK_CreateFrameResources(R_VK_State* state, FrameResources* resources)
{
  VkFenceCreateInfo fence_info = {
    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    .flags = VK_FENCE_CREATE_SIGNALED_BIT
  };
  VK_CHECK(vkCreateFence(state->device.logical, &fence_info, 0, &resources->submit_fence));

  VkCommandPoolCreateInfo cmd_pool_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
    .queueFamilyIndex = state->device.graphics_queue_index
  };
  VK_CHECK(vkCreateCommandPool(state->device.logical, &cmd_pool_info, 0, &resources->cmd_pool));

  VkCommandBufferAllocateInfo cmd_buffer = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool = resources->cmd_pool,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = 1
  };
  VK_CHECK(vkAllocateCommandBuffers(state->device.logical, &cmd_buffer, &resources->cmd_buffer));

  VkSemaphoreCreateInfo semaphore_info = {
    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
  };
  VK_CHECK(vkCreateSemaphore(state->device.logical, &semaphore_info, 0, &resources->acquire_semaphore));
  VK_CHECK(vkCreateSemaphore(state->device.logical, &semaphore_info, 0, &resources->release_semaphore));
}

func void
R_VK_DestroyFrameResources(R_VK_State* state, FrameResources* resources)
{
  vkDestroySemaphore(state->device.logical, resources->release_semaphore, 0);
  vkDestroySemaphore(state->device.logical, resources->acquire_semaphore, 0);
  vkDestroyCommandPool(state->device.logical, resources->cmd_pool, 0);
  vkDestroyFence(state->device.logical, resources->submit_fence, 0);
}

func void
R_VK_CreateSwapchain(R_VK_State* state)
{
  R_VK_Swapchain swapchain = {};
  
  Arena* tmp_arena = AllocateArena(Kilobytes(64));
  {
    U32 format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(state->device.physical, state->surface.handle, &format_count, 0);
    VkSurfaceFormatKHR* formats = (VkSurfaceFormatKHR*)PushArena(tmp_arena, format_count * sizeof(VkSurfaceFormatKHR));
    vkGetPhysicalDeviceSurfaceFormatsKHR(state->device.physical, state->surface.handle, &format_count, formats);

    for (U32 i = 0; i < format_count; i += 1)
    {
      if (formats[i].format == VK_FORMAT_B8G8R8A8_UNORM && formats[i].colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
        swapchain.surface_format = formats[i];
      }
    }
  }
  FreeArena(tmp_arena);
  
  VkSurfaceCapabilitiesKHR capabilities;
  VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(state->device.physical,
                                                     state->surface.handle,
                                                     &capabilities));
  if (capabilities.currentExtent.width == U32_MAX)
  {
    swapchain.size = MakeVec2u(1280, 720);
  }
  else
  {
    swapchain.size.width = capabilities.currentExtent.width;
    swapchain.size.height = capabilities.currentExtent.height;
  }

  VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;

  U32 image_count = capabilities.minImageCount + 1;
  if ((capabilities.maxImageCount > 0) && (image_count > capabilities.maxImageCount))
  {
    image_count = capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR swapchain_info = {
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .surface = state->surface.handle,
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

  VK_CHECK(vkCreateSwapchainKHR(state->device.logical, &swapchain_info, 0, &swapchain.handle));

  VK_CHECK(vkGetSwapchainImagesKHR(state->device.logical, swapchain.handle, &swapchain.image_count, 0));
  swapchain.image_arena = AllocateArena(Megabytes(8));
  swapchain.images = (VkImage*)PushArena(swapchain.image_arena, swapchain.image_count * sizeof(VkImage));
  VK_CHECK(vkGetSwapchainImagesKHR(state->device.logical, swapchain.handle, &swapchain.image_count, swapchain.images));

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

    VK_CHECK(vkCreateImageView(state->device.logical, &view_info, 0, &swapchain.image_views[i]));
    
    R_VK_CreateFrameResources(state, &swapchain.frame_resources[i]);
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
      .initialLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
    };

    VK_CHECK(vkCreateImage(state->device.logical, &image_info, 0, &swapchain.depth_image));

    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(state->device.logical, swapchain.depth_image, &memory_requirements);

    VkMemoryAllocateInfo allocate_info = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = memory_requirements.size,
      .memoryTypeIndex = R_VK_FindMemoryTypeIndex(memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
    };
    VK_CHECK(vkAllocateMemory(state->device.logical, &allocate_info, 0, &swapchain.depth_image_memory));
    
    vkBindImageMemory(state->device.logical, swapchain.depth_image, swapchain.depth_image_memory, 0);

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

    VK_CHECK(vkCreateImageView(state->device.logical, &view_info, 0, &swapchain.depth_image_view));
  }
  
  state->swapchain = swapchain;
}

func void
R_VK_RecreateSwapchain(R_VK_State* state)
{
  LOG_INFO("Recreate Swapchain\n");
  R_VK_DestroySwapchain(state);
  R_VK_CreateSwapchain(state);
}

func void
R_VK_DestroySwapchain(R_VK_State* state)
{
  vkDeviceWaitIdle(state->device.logical);
  
  for (I32 i = 0; i < state->swapchain.image_count; i += 1)
  {
    R_VK_DestroyFrameResources(state, &state->swapchain.frame_resources[i]);
    vkDestroyImageView(state->device.logical, state->swapchain.image_views[i], 0);
  }

  vkDestroySwapchainKHR(state->device.logical, state->swapchain.handle, 0);

  FreeArena(state->swapchain.image_arena);
}

func B32
R_VK_AcquireNextImage(R_VK_State* state, U32 *image_index)
{
  state->swapchain.current_index += 1;
  state->swapchain.current_index %= state->swapchain.image_count;
  U32 current_index = state->swapchain.current_index;

  VkResult acquire_result = vkAcquireNextImageKHR(
    state->device.logical, state->swapchain.handle, U64_MAX,
    state->swapchain.frame_resources[current_index].acquire_semaphore, 0,
    image_index
  );
  
  if (acquire_result != VK_SUCCESS) {
    return false;
  }

  vkWaitForFences(state->device.logical, 1, &state->swapchain.frame_resources[*image_index].submit_fence, true, U64_MAX);
  vkResetFences(state->device.logical, 1, &state->swapchain.frame_resources[*image_index].submit_fence);

  vkResetCommandPool(state->device.logical, state->swapchain.frame_resources[*image_index].cmd_pool, 0);

  return true;
}
