#include "render/vulkan/r_vk_core.h"
#include "third_party/vulkan/include/vulkan_core.h"
#pragma comment(lib, "third_party/vulkan/lib/vulkan-1.lib")

#define STB_IMAGE_IMPLEMENTATION
#include "../../third_party/stb_image.h"

#include "r_vk_device.cpp"

global VkDebugUtilsMessengerEXT R_VK_DebugMessenger;

VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
  LOG_INFO("VK_VALIDATION: %s\n", pCallbackData->pMessage);

  return VK_FALSE;
}

func void
populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& messengerInfo)
{
  messengerInfo = {};

  messengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  messengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  messengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
  messengerInfo.pfnUserCallback = debugCallback;
  messengerInfo.pUserData       = nullptr;
}

func VkResult
createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMesseneger)
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
destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, VkAllocationCallbacks* pAllocator)
{
  auto f = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

  if (f != nullptr)
  {
    f(instance, debugMessenger, pAllocator);
  }
}

func VkResult
createDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT* debugMessenger)
{
  VkDebugUtilsMessengerCreateInfoEXT messengerInfo = {};
  populateDebugMessengerCreateInfo(messengerInfo);

  return createDebugUtilsMessengerEXT(instance, &messengerInfo, nullptr, debugMessenger);
}
// End Debug Layer Staff ---------------------------------------------

func B32
R_VK_Init(OS_Window* window)
{
  r_vk_state = {};
  r_vk_state.arena = AllocateArena(Megabytes(128));

  R_VK_CreateInstance();
  R_VK_CreateDevice();
  R_VK_CreateSurface(&r_vk_state, window, &r_vk_state.swapchain);
  R_VK_CreateDescriptorPool();
  R_VK_CreateDepthImage();
  Rect2f render_area = {};
  render_area.x = 0.0f;
  render_area.y = 0.0f;
  render_area.w = r_vk_state.swapchain.size.width;
  render_area.h = r_vk_state.swapchain.size.height;
  R_VK_CreateRenderPass(&r_vk_state, &r_vk_state.render_pass, render_area);
  R_VK_CreateSwapchain(500, 500);
  // --AlNov: Create Framebuffers
  {
    U32 image_count = r_vk_state.swapchain.image_count;
    r_vk_state.swapchain.framebuffers = (R_VK_Framebuffer*)PushArena(r_vk_state.arena, image_count * sizeof(R_VK_Framebuffer));

    for (U32 i = 0; i < image_count; i += 1)
    {
      VkImageView attachments[2] = { r_vk_state.swapchain.image_views[i], r_vk_state.depth_view };

      R_VK_CreateFramebuffer(
        &r_vk_state, &r_vk_state.render_pass, r_vk_state.swapchain.size,
        CountArrayElements(attachments), attachments, &r_vk_state.swapchain.framebuffers[i]
      );
    }
  }
  R_VK_CreateCommandPool(&r_vk_state);
  // --AlNov: Create Command Buffers
  {
    r_vk_state.command_buffers = (R_VK_CommandBuffer*)PushArena(r_vk_state.arena, NUM_FRAMES_IN_FLIGHT);

    for (U32 i = 0; i < NUM_FRAMES_IN_FLIGHT; i += 1)
    {
      R_VK_AllocateCommandBuffer(&r_vk_state, r_vk_state.command_pool, &r_vk_state.command_buffers[i]);
    }
  }
  r_vk_state.texture = R_VK_CreateTexture("data/uv_checker.png");
  R_VK_CreateCubeMap("", &r_vk_state.cubemap);

  R_VK_CreateSyncTools();

  r_vk_state.big_buffer = {};
  r_vk_state.big_buffer.size = Megabytes(128);
  R_VK_CreateBuffer(
    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT|VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
    r_vk_state.big_buffer.size,
    &r_vk_state.big_buffer.buffer,
    &r_vk_state.big_buffer.memory
  );
  vkMapMemory(r_vk_state.device.logical, r_vk_state.big_buffer.memory, 0, r_vk_state.big_buffer.size, 0, &r_vk_state.big_buffer.mapped_memory);

  return true;
}

func void
R_VK_CreateInstance()
{
  VkApplicationInfo app_info = {};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "VulkanRenderingFramework";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "RenderingEngine";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_API_VERSION_1_3;

  const char* extension_names[] = {
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
    "VK_KHR_win32_surface",
    "VK_KHR_surface",
  };

  const char* validation_layers[] = {
    "VK_LAYER_KHRONOS_validation",
    "VK_LAYER_LUNARG_monitor",
  };

  VkInstanceCreateInfo instance_info = {};
  instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_info.pApplicationInfo = &app_info;
  instance_info.enabledLayerCount = CountArrayElements(validation_layers);
  instance_info.ppEnabledLayerNames = validation_layers;
  instance_info.enabledExtensionCount = CountArrayElements(extension_names);
  instance_info.ppEnabledExtensionNames = extension_names;

  VkDebugUtilsMessengerCreateInfoEXT messenger_info;
  populateDebugMessengerCreateInfo(messenger_info);
  instance_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&messenger_info;

  VK_CHECK(vkCreateInstance(&instance_info, 0, &r_vk_state.instance));

  VK_CHECK(createDebugMessenger(r_vk_state.instance, &R_VK_DebugMessenger));
}

func void
R_VK_CreateSurface(R_VK_State* vk_state, OS_Window* window, R_VK_Swapchain* swapchain)
{
  VkWin32SurfaceCreateInfoKHR surface_info = {};
  surface_info.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  surface_info.hinstance = window->instance;
  surface_info.hwnd      = window->handle;

  VK_CHECK(vkCreateWin32SurfaceKHR(vk_state->instance, &surface_info, 0, &vk_state->swapchain.surface));

  // Get Surface Capabilities
  VkSurfaceCapabilitiesKHR capabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_state->device.physical, vk_state->swapchain.surface, &capabilities);

  vk_state->swapchain.size.width = capabilities.currentExtent.width;
  vk_state->swapchain.size.height = capabilities.currentExtent.height;

  vk_state->swapchain.image_count = capabilities.minImageCount + 1;
  if (capabilities.maxImageCount > 0 && vk_state->swapchain.image_count > capabilities.maxImageCount) {
    vk_state->swapchain.image_count= capabilities.maxImageCount;
  }

  // Get Surface Format
  Arena* tmp_arena = AllocateArena(Kilobytes(64));
  {
    U32 format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(vk_state->device.physical, vk_state->swapchain.surface, &format_count, 0);
    VkSurfaceFormatKHR* formats = (VkSurfaceFormatKHR*)PushArena(tmp_arena, format_count * sizeof(VkSurfaceFormatKHR));
    vkGetPhysicalDeviceSurfaceFormatsKHR(vk_state->device.physical, vk_state->swapchain.surface, &format_count, formats);

    for (U32 i = 0; i < format_count; i += 1)
    {
      if (formats[i].format == VK_FORMAT_B8G8R8A8_UNORM && formats[i].colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
        vk_state->swapchain.surface_format = formats[i];
      }
    }
  }
  FreeArena(tmp_arena);
}

func void
R_VK_CreateDescriptorPool()
{
  U32 descriptor_count = 10000;

  VkDescriptorPoolSize pool_size = {};
  pool_size.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  pool_size.descriptorCount = 1024;

  VkDescriptorPoolSize sampler_pool_size = {};
  sampler_pool_size.type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  sampler_pool_size.descriptorCount = 1024;

  VkDescriptorPoolSize pool_sizes[2] = { pool_size, sampler_pool_size };

  VkDescriptorPoolCreateInfo pool_info = {};
  pool_info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.maxSets       = descriptor_count;
  pool_info.poolSizeCount = 2;
  pool_info.pPoolSizes    = pool_sizes;

  VK_CHECK(vkCreateDescriptorPool(r_vk_state.device.logical, &pool_info, 0, &r_vk_state.descriptor_pool.pool));
}

func void
R_VK_CreateSyncTools()
{
  for (I32 i = 0; i < NUM_FRAMES_IN_FLIGHT; i += 1)
  {
    VkSemaphoreCreateInfo semaphore_info = {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VK_CHECK(vkCreateSemaphore(r_vk_state.device.logical, &semaphore_info, 0, &r_vk_state.sync_tools.image_available_semaphores[i]));
    VK_CHECK(vkCreateSemaphore(r_vk_state.device.logical, &semaphore_info, 0, &r_vk_state.sync_tools.image_ready_semaphores[i]));

    VkFenceCreateInfo fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VK_CHECK(vkCreateFence(r_vk_state.device.logical, &fence_info, 0, &r_vk_state.sync_tools.fences[i]));
  }
}

func B32
R_VK_BeginFrame()
{
  vkWaitForFences(r_vk_state.device.logical, 1, &r_vk_state.sync_tools.fences[r_vk_state.current_frame], VK_TRUE, U64_MAX);

  // --AlNov: @TODO Read more about vkAcquireNextImageKHR in terms of synchonization
  U32 image_index;
  VkResult acquire_result = vkAcquireNextImageKHR(
    r_vk_state.device.logical, r_vk_state.swapchain.handle,
    U64_MAX, r_vk_state.sync_tools.image_available_semaphores[r_vk_state.current_frame],
    0, &image_index);

  if (acquire_result == VK_ERROR_OUT_OF_DATE_KHR)
  {
    R_VK_RecreateSwapchain();
    return false;
  }
  else
  {
    VK_CHECK(acquire_result);
  }

  vkResetFences(r_vk_state.device.logical, 1, &r_vk_state.sync_tools.fences[r_vk_state.current_frame]);

  R_VK_CommandBuffer* command_buffer = &r_vk_state.command_buffers[r_vk_state.current_frame];
  vkResetCommandBuffer(command_buffer->handle, 0);

  r_vk_state.current_command_buffer = command_buffer;
  r_vk_state.current_image_index = image_index;
  r_vk_state.current_framebuffer = &r_vk_state.swapchain.framebuffers[image_index];

  R_VK_BeginCommandBuffer(r_vk_state.current_command_buffer);

  return true;
}

func void
R_VK_EndFrame()
{
  R_VK_EndCommandBuffer(r_vk_state.current_command_buffer);

  VkPipelineStageFlags wait_stage = {
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
  };

  VkSubmitInfo submit_info = {};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = &r_vk_state.sync_tools.image_available_semaphores[r_vk_state.current_image_index];
  submit_info.pWaitDstStageMask = &wait_stage;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &r_vk_state.command_buffers[r_vk_state.current_frame].handle;
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = &r_vk_state.sync_tools.image_ready_semaphores[r_vk_state.current_frame];

  VkQueue queue;
  vkGetDeviceQueue(r_vk_state.device.logical, r_vk_state.device.queue_index, 0, &queue);

  VK_CHECK(vkQueueSubmit(queue, 1, &submit_info, r_vk_state.sync_tools.fences[r_vk_state.current_frame]))

  vkDeviceWaitIdle(r_vk_state.device.logical);
  vkResetDescriptorPool(r_vk_state.device.logical, r_vk_state.descriptor_pool.pool, 0);

  r_vk_state.big_buffer.current_position = 0;
  r_vk_state.current_viewport = {};
}

func void
R_VK_PresentFrame()
{
  VkPresentInfoKHR present_info = {};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores = &r_vk_state.sync_tools.image_ready_semaphores[r_vk_state.current_frame];
  present_info.swapchainCount = 1;
  present_info.pSwapchains = &r_vk_state.swapchain.handle;
  present_info.pImageIndices = &r_vk_state.current_image_index;
  present_info.pResults = 0;

  VkQueue queue;
  vkGetDeviceQueue(r_vk_state.device.logical, r_vk_state.device.queue_index, 0, &queue);
  VkResult present_result = vkQueuePresentKHR(queue, &present_info);

  if (present_result == VK_ERROR_OUT_OF_DATE_KHR || present_result == VK_SUBOPTIMAL_KHR)
  {
    R_VK_RecreateSwapchain();
  }
  else
  {
    VK_CHECK(present_result);
  }

  r_vk_state.current_frame += 1;
  r_vk_state.current_frame %= NUM_FRAMES_IN_FLIGHT;
}

func void
R_VK_Draw(R_DrawInfo* info)
{
  R_VK_BindPipeline(info->pipeline);

  U32 alligment = 64 - (r_vk_state.big_buffer.current_position % 64);
  r_vk_state.big_buffer.current_position += alligment;
  VkDeviceSize scene_set_offset = r_vk_state.big_buffer.current_position;
  memcpy((U8*)r_vk_state.big_buffer.mapped_memory + r_vk_state.big_buffer.current_position, info->scene_group.data, info->scene_group.data_size);
  r_vk_state.big_buffer.current_position += info->scene_group.data_size;

  alligment = 64 - (r_vk_state.big_buffer.current_position % 64);
  r_vk_state.big_buffer.current_position += alligment;
  VkDeviceSize draw_vs_set_offset = r_vk_state.big_buffer.current_position;
  memcpy((U8*)r_vk_state.big_buffer.mapped_memory + r_vk_state.big_buffer.current_position, info->instance_group.data, info->instance_group.data_size);
  r_vk_state.big_buffer.current_position += info->instance_group.data_size;

  VkDescriptorSetAllocateInfo scene_descriptor = {};
  scene_descriptor.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  scene_descriptor.descriptorPool = r_vk_state.descriptor_pool.pool;
  scene_descriptor.descriptorSetCount = 1;
  scene_descriptor.pSetLayouts = &scene_descriptor_layout;

  VkDescriptorSetAllocateInfo draw_descriptor = {};
  draw_descriptor.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  draw_descriptor.descriptorPool = r_vk_state.descriptor_pool.pool;
  draw_descriptor.descriptorSetCount = 1;
  draw_descriptor.pSetLayouts = &instance_descriptor_layout;

  VkDescriptorSet scene_set;
  VK_CHECK(vkAllocateDescriptorSets(r_vk_state.device.logical, &scene_descriptor, &scene_set));
  VkDescriptorSet draw_set;
  VK_CHECK(vkAllocateDescriptorSets(r_vk_state.device.logical, &draw_descriptor, &draw_set));

  VkDescriptorBufferInfo scene_buffer_info = {};
  scene_buffer_info.buffer = r_vk_state.big_buffer.buffer;
  scene_buffer_info.offset = scene_set_offset;
  scene_buffer_info.range = info->scene_group.data_size;

  {
    VkWriteDescriptorSet write_set = {};
    write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_set.dstSet = scene_set;
    write_set.dstBinding = 0;
    write_set.dstArrayElement = 0;
    write_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write_set.descriptorCount = 1;
    write_set.pBufferInfo = &scene_buffer_info;

    vkUpdateDescriptorSets(
      r_vk_state.device.logical,
      1, &write_set, 0, 0
    );
  }

  VkDescriptorBufferInfo draw_vs_buffer_info = {};
  draw_vs_buffer_info.buffer = r_vk_state.big_buffer.buffer;
  draw_vs_buffer_info.offset = draw_vs_set_offset;
  draw_vs_buffer_info.range = info->instance_group.data_size;

  {
    VkWriteDescriptorSet write_set = {};
    write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_set.dstSet = draw_set;
    write_set.dstBinding = 0;
    write_set.dstArrayElement = 0;
    write_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write_set.descriptorCount = 1;
    write_set.pBufferInfo = &draw_vs_buffer_info;

    vkUpdateDescriptorSets(
      r_vk_state.device.logical,
      1, &write_set, 0, 0
    );
  }

  vkCmdBindDescriptorSets(
    r_vk_state.current_command_buffer->handle,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    r_vk_state.pipelines[r_vk_state.active_pipeline_index].layout,
    0, 1, &scene_set, 0, 0
  );
  vkCmdBindDescriptorSets(
    r_vk_state.current_command_buffer->handle,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    r_vk_state.pipelines[r_vk_state.active_pipeline_index].layout,
    1, 1, &draw_set, 0, 0
  );

  VkRect2D scissor = {};
  scissor.offset.x = info->scissor.x;
  scissor.offset.y = info->scissor.y;
  scissor.extent.width = info->scissor.w;
  scissor.extent.height = info->scissor.h;
  vkCmdSetScissor(
    r_vk_state.current_command_buffer->handle,
    0, 1, &scissor);

  VkBuffer vk_vertex_buffer = r_vk_state.buffers[info->vertex_buffer->buffer.handle].buffer;
  VkDeviceSize test_offset = 0;
  vkCmdBindVertexBuffers(r_vk_state.current_command_buffer->handle, 0, 1, &vk_vertex_buffer, &test_offset);

  VkBuffer vk_index_buffer = r_vk_state.buffers[info->index_buffer->buffer.handle].buffer;
  vkCmdBindIndexBuffer(r_vk_state.current_command_buffer->handle, vk_index_buffer, (VkDeviceSize)0, VK_INDEX_TYPE_UINT32);

  vkCmdDrawIndexed(r_vk_state.current_command_buffer->handle, info->index_buffer->index_count, 1, 0, 0, 0);
}

func void
R_VK_CreateDepthImage()
{
  VkImageCreateInfo image_info = {};
  image_info.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image_info.imageType     = VK_IMAGE_TYPE_2D;
  image_info.extent.width  = r_vk_state.swapchain.size.x;
  image_info.extent.height = r_vk_state.swapchain.size.y;
  image_info.extent.depth  = 1;
  image_info.mipLevels     = 1;
  image_info.arrayLayers   = 1;
  image_info.format        = VK_FORMAT_D32_SFLOAT;
  image_info.tiling        = VK_IMAGE_TILING_OPTIMAL;
  image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_info.usage         = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  image_info.samples       = VK_SAMPLE_COUNT_1_BIT;
  image_info.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;

  VK_CHECK(vkCreateImage(r_vk_state.device.logical, &image_info, 0, &r_vk_state.depth_image));

  VkMemoryRequirements memory_requirements;
  vkGetImageMemoryRequirements(r_vk_state.device.logical, r_vk_state.depth_image, &memory_requirements);

  VkMemoryAllocateInfo allocate_info = {};
  allocate_info.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocate_info.allocationSize  = memory_requirements.size;
  allocate_info.memoryTypeIndex = R_VK_FindMemoryType(memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  VK_CHECK(vkAllocateMemory(r_vk_state.device.logical, &allocate_info, 0, &r_vk_state.depth_memory));

  VK_CHECK(vkBindImageMemory(r_vk_state.device.logical, r_vk_state.depth_image, r_vk_state.depth_memory, 0));

  VkImageViewCreateInfo view_info = {};
  view_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  view_info.image                           = r_vk_state.depth_image;
  view_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
  view_info.format                          = VK_FORMAT_D32_SFLOAT;
  view_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
  view_info.subresourceRange.baseMipLevel   = 0;
  view_info.subresourceRange.levelCount     = 1;
  view_info.subresourceRange.baseArrayLayer = 0;
  view_info.subresourceRange.layerCount     = 1;

  VK_CHECK(vkCreateImageView(r_vk_state.device.logical, &view_info, 0, &r_vk_state.depth_view));
}

// -------------------------------------------------------------------
// --AlNov: Helpers --------------------------------------------------
func U32
R_VK_FindMemoryType(U32 filter, VkMemoryPropertyFlags flags)
{
  VkPhysicalDeviceMemoryProperties mem_properties = {};
  vkGetPhysicalDeviceMemoryProperties(r_vk_state.device.physical, &mem_properties);

  for (U32 type_index = 0; type_index < mem_properties.memoryTypeCount; type_index += 1)
  {
    if (filter & (1 << type_index) && ((mem_properties.memoryTypes[type_index].propertyFlags & flags) == flags))
    {
      return type_index;
      break;
    }
  }

  LOG_ERROR("Cannot foind suitable memory type.\n");
  return -1;
}

func void
R_VK_CreateBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags property_flags, U32 size, VkBuffer* out_buffer, VkDeviceMemory* out_memory)
{
  VkBufferCreateInfo buffer_info = {};
  buffer_info.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_info.size        = size;
  buffer_info.usage       = usage;
  buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  VK_CHECK(vkCreateBuffer(r_vk_state.device.logical, &buffer_info, 0, out_buffer));

  VkMemoryRequirements memory_requirements = {};
  vkGetBufferMemoryRequirements(r_vk_state.device.logical, *out_buffer, &memory_requirements);

  VkMemoryAllocateInfo allocate_info = {};
  allocate_info.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocate_info.allocationSize  = memory_requirements.size;
  allocate_info.memoryTypeIndex = R_VK_FindMemoryType(memory_requirements.memoryTypeBits, property_flags);
  VK_CHECK(vkAllocateMemory(r_vk_state.device.logical, &allocate_info, 0, out_memory));

  VK_CHECK(vkBindBufferMemory(r_vk_state.device.logical, *out_buffer, *out_memory, 0));
}

func void
R_VK_MemCopy(VkDeviceMemory memory, void* data, U64 size)
{
  void* mapped_memory;
  vkMapMemory(r_vk_state.device.logical, memory, 0, size, 0, &mapped_memory);
  {
    memcpy(mapped_memory, data, size);
  }
  vkUnmapMemory(r_vk_state.device.logical, memory);
}

func VkShaderStageFlagBits
R_VK_ShaderStageFromShaderType(R_ShaderType type)
{
  switch (type)
  {
    case R_SHADER_TYPE_VERTEX   : return VK_SHADER_STAGE_VERTEX_BIT;
    case R_SHADER_TYPE_FRAGMENT : return VK_SHADER_STAGE_FRAGMENT_BIT;

    default: Assert(1); return VK_SHADER_STAGE_FRAGMENT_BIT; // --AlNov: type is not supported by Vulkan Layer
  }
}

func VkFormat
R_VK_VkFormatFromAttributeFormat(R_VertexAttributeFormat format)
{
  switch (format)
  {
    case R_VERTEX_ATTRIBUTE_FORMAT_VEC3F  : return VK_FORMAT_R32G32B32_SFLOAT;
    case R_VERTEX_ATTRIBUTE_FORMAT_VEC2F  : return VK_FORMAT_R32G32_SFLOAT;

    default: Assert(1); return VK_FORMAT_R32_SFLOAT; // --AlNov: format is not supported by Vulkan Layer
  }
}

func VkDescriptorType
R_VK_DescriptorTypeFromBindingType(R_BindingType type)
{
  switch (type)
  {
    case R_BINDING_TYPE_UNIFORM_BUFFER: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case R_BINDING_TYPE_TEXTURE_2D:     return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    default: Assert(1); return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // --AlNov: Binding type not supported by Vulkan Layer
  }
}

func VkCommandBuffer
R_VK_BeginSingleCommands()
{
  VkCommandBufferAllocateInfo allocate_info = {};
  allocate_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocate_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocate_info.commandPool        = r_vk_state.command_pool;
  allocate_info.commandBufferCount = 1;

  VkCommandBuffer command_buffer;
  VK_CHECK(vkAllocateCommandBuffers(r_vk_state.device.logical, &allocate_info, &command_buffer));

  VkCommandBufferBeginInfo begin_info = {};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  VK_CHECK(vkBeginCommandBuffer(command_buffer, &begin_info));

  return command_buffer;
}

func void
R_VK_EndSingleCommands(VkCommandBuffer command_buffer)
{
  VK_CHECK(vkEndCommandBuffer(command_buffer));

  VkSubmitInfo submit_info = {};
  submit_info.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers    = &command_buffer;

  VkQueue queue;
  vkGetDeviceQueue(r_vk_state.device.logical, r_vk_state.device.queue_index, 0, &queue);

  VK_CHECK(vkQueueSubmit(queue, 1, &submit_info, 0));
  VK_CHECK(vkQueueWaitIdle(queue));

  vkFreeCommandBuffers(r_vk_state.device.logical, r_vk_state.command_pool, 1, &command_buffer);
}

func void
R_VK_CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size)
{
  VkCommandBuffer command_buffer = R_VK_BeginSingleCommands();
  {
    VkBufferCopy buffer_copy_region = {};
    buffer_copy_region.srcOffset = 0;
    buffer_copy_region.dstOffset = 0;
    buffer_copy_region.size      = 0;

    vkCmdCopyBuffer(command_buffer, src, dst, 1, &buffer_copy_region);
  }
  R_VK_EndSingleCommands(command_buffer);
}

func void
R_VK_CopyBufferToImage(VkBuffer buffer, VkImage image, Vec2u image_dimensions)
{
  VkCommandBuffer command_buffer = R_VK_BeginSingleCommands();
  {
    VkBufferImageCopy copy_info = {};
    copy_info.bufferOffset                    = 0;
    copy_info.bufferRowLength                 = 0;
    copy_info.bufferImageHeight               = 0;
    copy_info.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_info.imageSubresource.mipLevel       = 0;
    copy_info.imageSubresource.baseArrayLayer = 0;
    copy_info.imageSubresource.layerCount     = 1;
    copy_info.imageOffset                     = { 0, 0, 0 };
    copy_info.imageExtent                     = { image_dimensions.x, image_dimensions.y, 1 };

    vkCmdCopyBufferToImage(command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_info);
  }
  R_VK_EndSingleCommands(command_buffer);
}

func void
R_VK_TransitImageLayout(VkImage image, VkFormat format, U32 layer_count, VkImageLayout old_layout, VkImageLayout new_layout)
{
  VkCommandBuffer command_buffer = R_VK_BeginSingleCommands();
  {
    VkImageMemoryBarrier image_barrier = {};
    image_barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    image_barrier.image                           = image;
    image_barrier.oldLayout                       = old_layout;
    image_barrier.newLayout                       = new_layout;
    image_barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    image_barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    image_barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    image_barrier.subresourceRange.baseMipLevel   = 0;
    image_barrier.subresourceRange.levelCount     = 1;
    image_barrier.subresourceRange.baseArrayLayer = 0;
    image_barrier.subresourceRange.layerCount     = layer_count;

    VkPipelineStageFlags src_stage;
    VkPipelineStageFlags dst_stage;

    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
      image_barrier.srcAccessMask = 0;
      image_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

      src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
      dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
      image_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      image_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

      src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
      dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
      LOG_ERROR("Wrong image layout transition.\n");
      return;
    }

    vkCmdPipelineBarrier(command_buffer, src_stage, dst_stage, 0, 0, 0, 0, 0, 1, &image_barrier);
  }
  R_VK_EndSingleCommands(command_buffer);
}

func bool
LoadTexture(const char* path, U8** out_data, I32* out_width, I32* out_height, I32* out_channels)
{
  *out_data = stbi_load(path, out_width, out_height, out_channels, STBI_rgb_alpha);

  if (!out_data)
  {
    LOG_ERROR("Cannot load texture %s\n", path);
    return false;
  }

  return true;
}

func R_Texture
R_VK_CreateTexture(const char* path)
{
  R_Texture texture = {};

  I32 tex_width    = 0;
  I32 tex_height   = 0;
  I32 tex_channels = 0;
  U8* tex_pixels   = 0;
  LoadTexture(path, &tex_pixels, &tex_width, &tex_height, &tex_channels);

  texture.size = tex_width * tex_height * 4;

  r_vk_state.staging_buffer = {};
  r_vk_state.staging_buffer.size = texture.size;
  R_VK_CreateBuffer(
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
    r_vk_state.staging_buffer.size,
    &r_vk_state.staging_buffer.buffer,
    &r_vk_state.staging_buffer.memory
  );

  void* data;
  vkMapMemory(r_vk_state.device.logical, r_vk_state.staging_buffer.memory, 0, texture.size, 0, &data);
    memcpy(data, tex_pixels, texture.size);
  vkUnmapMemory(r_vk_state.device.logical, r_vk_state.staging_buffer.memory);

  stbi_image_free(tex_pixels);

  VkImageCreateInfo image_info = {};
  image_info.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image_info.samples       = VK_SAMPLE_COUNT_1_BIT;
  image_info.imageType     = VK_IMAGE_TYPE_2D;
  image_info.extent.width  = tex_width;
  image_info.extent.height = tex_width;
  image_info.extent.depth  = 1;
  image_info.mipLevels     = 1;
  image_info.arrayLayers   = 1;
  image_info.format        = VK_FORMAT_R8G8B8A8_SRGB;
  image_info.tiling        = VK_IMAGE_TILING_OPTIMAL;
  image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_info.usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  image_info.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
  image_info.samples       = VK_SAMPLE_COUNT_1_BIT;
  if (vkCreateImage(r_vk_state.device.logical, &image_info, 0, &texture.vk_image) != VK_SUCCESS)
  {
    LOG_ERROR("Cannot create Image for Texture.\n");
    return texture;
  }

  VkMemoryRequirements mem_requirements = {};
  vkGetImageMemoryRequirements(r_vk_state.device.logical, texture.vk_image, &mem_requirements);

  VkMemoryAllocateInfo mem_info = {};
  mem_info.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  mem_info.allocationSize  = mem_requirements.size;
  mem_info.memoryTypeIndex = R_VK_FindMemoryType(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  VK_CHECK(vkAllocateMemory(r_vk_state.device.logical, &mem_info, 0, &texture.vk_memory));

  VK_CHECK(vkBindImageMemory(r_vk_state.device.logical, texture.vk_image, texture.vk_memory, 0));

  R_VK_TransitImageLayout(texture.vk_image, VK_FORMAT_R8G8B8A8_SRGB, 1, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  {
    R_VK_CopyBufferToImage(r_vk_state.staging_buffer.buffer, texture.vk_image, MakeVec2u(tex_width, tex_height));
  }
  R_VK_TransitImageLayout(texture.vk_image, VK_FORMAT_R8G8B8A8_SRGB, 1, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  vkDestroyBuffer(r_vk_state.device.logical, r_vk_state.staging_buffer.buffer, 0);
  vkFreeMemory(r_vk_state.device.logical, r_vk_state.staging_buffer.memory, 0);

  // AlNov: Create Texture Image View
  VkImageViewCreateInfo view_info = {};
  view_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  view_info.image                           = texture.vk_image;
  view_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
  view_info.format                          = VK_FORMAT_R8G8B8A8_SRGB;
  view_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
  view_info.subresourceRange.baseMipLevel   = 0;
  view_info.subresourceRange.levelCount     = 1;
  view_info.subresourceRange.baseArrayLayer = 0;
  view_info.subresourceRange.layerCount     = 1;

  VK_CHECK(vkCreateImageView(r_vk_state.device.logical, &view_info, 0, &texture.vk_view));

  // AlNov: Create Texture Sampler
  VkSamplerCreateInfo sampler_info = {};
  sampler_info.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  sampler_info.magFilter               = VK_FILTER_LINEAR;
  sampler_info.minFilter               = VK_FILTER_LINEAR;
  sampler_info.addressModeU            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
  sampler_info.addressModeV            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
  sampler_info.addressModeW            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
  sampler_info.anisotropyEnable        = VK_FALSE;
  sampler_info.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  sampler_info.unnormalizedCoordinates = VK_FALSE;
  sampler_info.compareEnable           = VK_FALSE;
  sampler_info.compareOp               = VK_COMPARE_OP_ALWAYS;
  sampler_info.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  sampler_info.mipLodBias              = 0.0f;
  sampler_info.minLod                  = 0.0f;
  sampler_info.maxLod                  = 0.0f;

  VK_CHECK(vkCreateSampler(r_vk_state.device.logical, &sampler_info, 0, &r_vk_state.sampler));

  return texture;
}

func void
R_VK_CreateCubeMap(const char* folder_path, R_VK_CubeMap* out_cubemap)
{
  I32 width = 0;
  I32 height = 0;
  I32 channels = 0;
  U8* datas[R_CUBE_MAP_SIDE_TYPE_COUNT] = {};
  LoadTexture("E:/Programming/Ignis/data/skybox/front.jpg", &datas[R_CUBE_MAP_SIDE_TYPE_FRONT], &width, &height, &channels);
  LoadTexture("E:/Programming/Ignis/data/skybox/back.jpg", &datas[R_CUBE_MAP_SIDE_TYPE_BACK], &width, &height, &channels);
  LoadTexture("E:/Programming/Ignis/data/skybox/right.jpg", &datas[R_CUBE_MAP_SIDE_TYPE_RIGHT], &width, &height, &channels);
  LoadTexture("E:/Programming/Ignis/data/skybox/left.jpg", &datas[R_CUBE_MAP_SIDE_TYPE_LEFT], &width, &height, &channels);
  LoadTexture("E:/Programming/Ignis/data/skybox/top.jpg", &datas[R_CUBE_MAP_SIDE_TYPE_TOP], &width, &height, &channels);
  LoadTexture("E:/Programming/Ignis/data/skybox/bottom.jpg", &datas[R_CUBE_MAP_SIDE_TYPE_BOTTOM], &width, &height, &channels);

  VkDeviceSize image_layer_size = width * height * 4;
  VkDeviceSize image_size       = image_layer_size * 6;

  R_VK_Buffer staging_buffer = {};
  staging_buffer.size = image_size;
  R_VK_CreateBuffer(
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
    staging_buffer.size,
    &staging_buffer.buffer,
    &staging_buffer.memory
  );

  VkImageCreateInfo image_info = {};
  image_info.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image_info.samples       = VK_SAMPLE_COUNT_1_BIT;
  image_info.imageType     = VK_IMAGE_TYPE_2D;
  image_info.extent.width  = width;
  image_info.extent.height = height;
  image_info.extent.depth  = 1;
  image_info.mipLevels     = 1;
  image_info.arrayLayers   = 6;
  image_info.format        = VK_FORMAT_R8G8B8A8_SRGB;
  image_info.tiling        = VK_IMAGE_TILING_OPTIMAL;
  image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_info.usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  image_info.flags         = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
  VK_CHECK(vkCreateImage(r_vk_state.device.logical, &image_info, 0, &out_cubemap->image));

  VkMemoryRequirements mem_requirements = {};
  vkGetImageMemoryRequirements(r_vk_state.device.logical, out_cubemap->image, &mem_requirements);

  VkMemoryAllocateInfo memory_info = {};
  memory_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  memory_info.allocationSize = mem_requirements.size;
  memory_info.memoryTypeIndex = R_VK_FindMemoryType(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  VK_CHECK(vkAllocateMemory(r_vk_state.device.logical, &memory_info, 0, &out_cubemap->memory));
  
  VK_CHECK(vkBindImageMemory(r_vk_state.device.logical, out_cubemap->image, out_cubemap->memory, 0));

  R_VK_TransitImageLayout(out_cubemap->image, VK_FORMAT_R8G8B8A8_SRGB, 6, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  {
    for (U32 i = 0; i < R_CUBE_MAP_SIDE_TYPE_COUNT; i += 1)
    {
      VkCommandBuffer command_buffer = R_VK_BeginSingleCommands();
      {
        VkBufferImageCopy copy_info = {};
        copy_info.bufferOffset = image_layer_size * i;
        copy_info.bufferRowLength = 0;
        copy_info.bufferImageHeight = 0;
        copy_info.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy_info.imageSubresource.mipLevel = 0;
        copy_info.imageSubresource.baseArrayLayer = i;
        copy_info.imageSubresource.layerCount = 1;
        copy_info.imageOffset = { 0, 0, 0 };
        copy_info.imageExtent = { (U32)width, (U32)height, 1 };

        vkCmdCopyBufferToImage(command_buffer, staging_buffer.buffer, out_cubemap->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_info);
      }
      R_VK_EndSingleCommands(command_buffer);
    }
  }
  R_VK_TransitImageLayout(out_cubemap->image, VK_FORMAT_R8G8B8A8_SRGB, 6, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  vkDestroyBuffer(r_vk_state.device.logical, staging_buffer.buffer, 0);
  vkFreeMemory(r_vk_state.device.logical, staging_buffer.memory, 0);

  VkImageViewCreateInfo view_info = {};
  view_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  view_info.image                           = out_cubemap->image;
  view_info.viewType                        = VK_IMAGE_VIEW_TYPE_CUBE;
  view_info.format                          = VK_FORMAT_R8G8B8A8_SRGB;
  view_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
  view_info.subresourceRange.baseMipLevel   = 0;
  view_info.subresourceRange.levelCount     = 1;
  view_info.subresourceRange.baseArrayLayer = 0;
  view_info.subresourceRange.layerCount     = 6;
  VK_CHECK(vkCreateImageView(r_vk_state.device.logical, &view_info, 0, &out_cubemap->view));
}

// -------------------------------------------------------------------
// --AlNov: View -----------------------------------------------------
func R_View
R_CreateView(Vec3f position, F32 fov, Vec2f size)
{
  R_View view = {};
  view.size               = size;
  view.position           = position;
  view.fov                = fov;
  view.uniform.projection = MakePerspective4x4f(fov, size.x / size.y, 0.1f, 100.0f);

  return view;
}

func void
R_VK_BindView(R_View view)
{
  r_vk_state.view = view;
}
