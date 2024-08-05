#include "r_vk_core.h"
#include "r_vk_types.h"
#pragma comment(lib, "third_party/vulkan/lib/vulkan-1.lib")

#define STB_IMAGE_IMPLEMENTATION
#include "../../third_party/stb_image.h"

global VkDebugUtilsMessengerEXT R_VK_DebugMessenger;

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
  LOG_INFO("VK_VALIDATION: %s\n", pCallbackData->pMessage);

  return VK_FALSE;
}

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& messengerInfo)
{
  messengerInfo = {};

  messengerInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  messengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  messengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
  messengerInfo.pfnUserCallback = debugCallback;
  messengerInfo.pUserData       = nullptr;
}

VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMesseneger)
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

void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, VkAllocationCallbacks* pAllocator)
{
  auto f = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

  if (f != nullptr)
  {
    f(instance, debugMessenger, pAllocator);
  }
}

VkResult createDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT* debugMessenger)
{
  VkDebugUtilsMessengerCreateInfoEXT messengerInfo = {};
  populateDebugMessengerCreateInfo(messengerInfo);

  return createDebugUtilsMessengerEXT(instance, &messengerInfo, nullptr, debugMessenger);
}
// End Debug Layer Staff ---------------------------------------------

func b8 R_VK_Init(OS_Window* window)
{
  r_vk_state = {};
  r_vk_state.arena = AllocateArena(Megabytes(128));

  R_VK_CreateInstance();
  R_VK_CreateDevice();
  R_VK_CreateSurface(&r_vk_state, window, &r_vk_state.swapchain);
  R_VK_CreateSwapchain();
  R_VK_CreateDescriptorPool();
  R_VK_CreateDepthImage();
  Rect2f render_area = {};
  render_area.x0 = 0.0f;
  render_area.y0 = 0.0f;
  render_area.x1 = r_vk_state.swapchain.size.width;
  render_area.y1 = r_vk_state.swapchain.size.height;
  R_VK_CreateRenderPass(&r_vk_state, &r_vk_state.render_pass, render_area, MakeVec4f(0.05f, 0.05f, 0.05f, 1.0f), 1.0f, 0);
  // --AlNov: Create Framebuffers
  {
    u32 image_count = r_vk_state.swapchain.image_count;
    r_vk_state.swapchain.framebuffers = (R_VK_Framebuffer*)PushArena(r_vk_state.arena, image_count * sizeof(R_VK_Framebuffer));

    for (u32 i = 0; i < image_count; i += 1)
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

    for (u32 i = 0; i < NUM_FRAMES_IN_FLIGHT; i += 1)
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

func void R_VK_CreateInstance()
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

func void R_VK_CreateDevice()
{
  Arena* tmp_arena = AllocateArena(Kilobytes(10));
  {
    u32 device_count = 0;
    vkEnumeratePhysicalDevices(r_vk_state.instance, &device_count, 0);
    VkPhysicalDevice* devices = (VkPhysicalDevice*)PushArena(tmp_arena, device_count * sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(r_vk_state.instance, &device_count, devices);

    for (u32 i = 0; i < device_count; i += 1)
    {
      VkPhysicalDeviceProperties properties;
      vkGetPhysicalDeviceProperties(devices[i], &properties);

      if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
      {
        r_vk_state.device.physical = devices[i];
      }
    }

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(r_vk_state.device.physical, &properties);

    LOG_INFO("GPU NAME: %s\n", properties.deviceName);

    u32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(r_vk_state.device.physical, &queue_family_count, 0);
    VkQueueFamilyProperties* queue_family_properties = (VkQueueFamilyProperties*)PushArena(tmp_arena, queue_family_count * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(r_vk_state.device.physical, &queue_family_count, queue_family_properties);

    for (u32 i = 0; i < queue_family_count; i += 1)
    {
      if (queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
      {
        r_vk_state.device.queue_index = i;
        break;
      }
    }

    VkDeviceQueueCreateInfo graphics_queue_info = {};
    graphics_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    graphics_queue_info.queueFamilyIndex = r_vk_state.device.queue_index;
    graphics_queue_info.queueCount = 1;
    const f32 priority = 1.0f;
    graphics_queue_info.pQueuePriorities = &priority;

    const char* extension_names[] = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkDeviceCreateInfo device_info = {};
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.queueCreateInfoCount = 1;
    device_info.pQueueCreateInfos = &graphics_queue_info;
    device_info.enabledExtensionCount = CountArrayElements(extension_names);
    device_info.ppEnabledExtensionNames = extension_names;
    device_info.pEnabledFeatures = 0;

    VK_CHECK(vkCreateDevice(r_vk_state.device.physical, &device_info, 0, &r_vk_state.device.logical));
  }
  FreeArena(tmp_arena);
}

func void R_VK_CreateSurface(R_VK_State* vk_state, OS_Window* window, R_VK_Swapchain* swapchain)
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
    u32 format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(vk_state->device.physical, vk_state->swapchain.surface, &format_count, 0);
    VkSurfaceFormatKHR* formats = (VkSurfaceFormatKHR*)PushArena(tmp_arena, format_count * sizeof(VkSurfaceFormatKHR));
    vkGetPhysicalDeviceSurfaceFormatsKHR(vk_state->device.physical, vk_state->swapchain.surface, &format_count, formats);

    for (u32 i = 0; i < format_count; i += 1)
    {
      if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && formats[i].colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
        vk_state->swapchain.surface_format = formats[i];
      }
    }
  }
  FreeArena(tmp_arena);
}

func void R_VK_CreateSwapchain()
{
  VkSwapchainCreateInfoKHR swapchain_info = {};
  swapchain_info.sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchain_info.surface               = r_vk_state.swapchain.surface;
  swapchain_info.minImageCount         = r_vk_state.swapchain.image_count;
  swapchain_info.imageFormat           = r_vk_state.swapchain.surface_format.format;
  swapchain_info.imageColorSpace       = r_vk_state.swapchain.surface_format.colorSpace;
  swapchain_info.imageExtent.width     = r_vk_state.swapchain.size.width;
  swapchain_info.imageExtent.height    = r_vk_state.swapchain.size.height;
  swapchain_info.imageArrayLayers      = 1;
  swapchain_info.imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swapchain_info.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
  swapchain_info.queueFamilyIndexCount = 0;
  swapchain_info.pQueueFamilyIndices   = 0;
  swapchain_info.preTransform          = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  swapchain_info.compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchain_info.presentMode           = VK_PRESENT_MODE_IMMEDIATE_KHR;
  swapchain_info.clipped               = VK_TRUE;
  swapchain_info.oldSwapchain          = VK_NULL_HANDLE;

  VK_CHECK(vkCreateSwapchainKHR(r_vk_state.device.logical, &swapchain_info, 0, &r_vk_state.swapchain.handle));

  vkGetSwapchainImagesKHR(r_vk_state.device.logical, r_vk_state.swapchain.handle, &r_vk_state.swapchain.image_count, 0);
  // --AlNov: @TODO Images doesnt deleted on swapchain recreation
  r_vk_state.swapchain.images = (VkImage*)PushArena(r_vk_state.arena, r_vk_state.swapchain.image_count * sizeof(VkImage));
  vkGetSwapchainImagesKHR(r_vk_state.device.logical, r_vk_state.swapchain.handle, &r_vk_state.swapchain.image_count, r_vk_state.swapchain.images);

  r_vk_state.swapchain.image_views = (VkImageView*)PushArena(r_vk_state.arena, r_vk_state.swapchain.image_count * sizeof(VkImageView));
  for (u32 i = 0; i < r_vk_state.swapchain.image_count; i += 1)
  {
    VkImageViewCreateInfo image_view_info = {};
    image_view_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_info.image                           = r_vk_state.swapchain.images[i];
    image_view_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
    image_view_info.format                          = r_vk_state.swapchain.surface_format.format;
    image_view_info.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_info.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_info.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_info.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_info.subresourceRange.baseMipLevel   = 0;
    image_view_info.subresourceRange.levelCount     = 1;
    image_view_info.subresourceRange.baseArrayLayer = 0;
    image_view_info.subresourceRange.layerCount     = 1;

    VK_CHECK(vkCreateImageView(r_vk_state.device.logical, &image_view_info, 0, &r_vk_state.swapchain.image_views[i]));
  }
}

func void R_VK_CreateDescriptorPool()
{
  u32 descriptor_count = 100;

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

func void R_VK_CreateSyncTools()
{
  for (i32 i = 0; i < NUM_FRAMES_IN_FLIGHT; i += 1)
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

func void R_VK_CreateDepthImage()
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
// --AlNov: Render Pass ----------------------------------------------
func void R_VK_CreateRenderPass(
  R_VK_State* vk_state, R_VK_RenderPass* out_render_pass,
  Rect2f render_area, Vec4f clear_color,
  f32 clear_depth, u32 clear_stencil
)
{
  VkAttachmentDescription color_attachment = {};
  color_attachment.format         = vk_state->swapchain.surface_format.format;
  color_attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
  color_attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  color_attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
  color_attachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference color_attachment_reference = {};
  color_attachment_reference.attachment = 0;
  color_attachment_reference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentDescription depth_attachment = {};
  depth_attachment.format         = VK_FORMAT_D32_SFLOAT;
  depth_attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
  depth_attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depth_attachment.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depth_attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depth_attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
  depth_attachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depth_attachment_reference = {};
  depth_attachment_reference.attachment = 1;
  depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.inputAttachmentCount    = 0;
  subpass.pInputAttachments       = 0;
  subpass.colorAttachmentCount    = 1;
  subpass.pColorAttachments       = &color_attachment_reference;
  subpass.pResolveAttachments     = 0;
  subpass.pDepthStencilAttachment = &depth_attachment_reference;
  subpass.preserveAttachmentCount = 0;
  subpass.pPreserveAttachments    = 0;

  VkSubpassDependency subpass_dependency = {};
  subpass_dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
  subpass_dependency.dstSubpass    = 0;
  subpass_dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  subpass_dependency.srcAccessMask = 0;
  subpass_dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  VkAttachmentDescription attachments[2] = { color_attachment, depth_attachment };

  VkRenderPassCreateInfo render_pass_info = {};
  render_pass_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_info.attachmentCount = 2;
  render_pass_info.pAttachments    = attachments;
  render_pass_info.subpassCount    = 1;
  render_pass_info.pSubpasses      = &subpass;
  render_pass_info.dependencyCount = 1;
  render_pass_info.pDependencies   = &subpass_dependency;

  VK_CHECK(vkCreateRenderPass(vk_state->device.logical, &render_pass_info, 0, &out_render_pass->handle));

  out_render_pass->render_area   = render_area;
  out_render_pass->clear_color   = clear_color;
  out_render_pass->clear_depth   = clear_depth;
  out_render_pass->clear_stencil = clear_stencil;
}

func void R_VK_DestroyRenderPass(R_VK_State* vk_state, R_VK_RenderPass* render_pass)
{
  if (!render_pass || render_pass->handle) { return; }

  vkDestroyRenderPass(vk_state->device.logical, render_pass->handle, 0);
  
  render_pass->handle = 0;
}

func void R_VK_BeginRenderPass(R_VK_CommandBuffer* command_buffer, R_VK_RenderPass* render_pass, R_VK_Framebuffer* framebuffer)
{
}

func void R_VK_EndRenderPass(R_VK_CommandBuffer* command_buffer, R_VK_RenderPass* render_pass)
{
  vkCmdEndRenderPass(command_buffer->handle);
}

func void R_VK_BeginFrame()
{
  r_vk_state.current_frame %= NUM_FRAMES_IN_FLIGHT;

  vkWaitForFences(r_vk_state.device.logical, 1, &r_vk_state.sync_tools.fences[r_vk_state.current_frame], VK_TRUE, U64_MAX);

  // --AlNov: @TODO Read more about vkAcquireNextImageKHR in terms of synchonization
  u32 image_index;
  VkResult image_acquire_result = vkAcquireNextImageKHR(
      r_vk_state.device.logical, r_vk_state.swapchain.handle,
      U64_MAX, r_vk_state.sync_tools.image_available_semaphores[r_vk_state.current_frame],
      0, &image_index
      );

  vkResetFences(r_vk_state.device.logical, 1, &r_vk_state.sync_tools.fences[r_vk_state.current_frame]);

  R_VK_CommandBuffer* command_buffer = &r_vk_state.command_buffers[r_vk_state.current_frame];
  vkResetCommandBuffer(command_buffer->handle, 0);

  r_vk_state.current_command_buffer = command_buffer;
  r_vk_state.current_image_index    = image_index;
  r_vk_state.current_framebuffer    = &r_vk_state.swapchain.framebuffers[image_index];

  R_VK_BeginCommandBuffer(r_vk_state.current_command_buffer);
}

func void R_VK_EndFrame()
{
  R_VK_EndCommandBuffer(r_vk_state.current_command_buffer);

  VkPipelineStageFlags wait_stage = {
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
  };

  VkSubmitInfo submit_info = {};
  submit_info.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.waitSemaphoreCount   = 1;
  submit_info.pWaitSemaphores      = &r_vk_state.sync_tools.image_available_semaphores[r_vk_state.current_image_index];
  submit_info.pWaitDstStageMask    = &wait_stage;
  submit_info.commandBufferCount   = 1;
  submit_info.pCommandBuffers      = &r_vk_state.command_buffers[r_vk_state.current_frame].handle;
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores    = &r_vk_state.sync_tools.image_ready_semaphores[r_vk_state.current_frame];

  VkQueue queue;
  vkGetDeviceQueue(r_vk_state.device.logical, r_vk_state.device.queue_index, 0, &queue);

  vkQueueSubmit(queue, 1, &submit_info, r_vk_state.sync_tools.fences[r_vk_state.current_frame]);

  VkPresentInfoKHR present_info = {};
  present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores    = &r_vk_state.sync_tools.image_ready_semaphores[r_vk_state.current_frame];
  present_info.swapchainCount     = 1;
  present_info.pSwapchains        = &r_vk_state.swapchain.handle;
  present_info.pImageIndices      = &r_vk_state.current_image_index;
  present_info.pResults           = 0;

  VkResult present_result = vkQueuePresentKHR(queue, &present_info);

  r_vk_state.current_frame += 1;

  r_vk_state.big_buffer.current_position = 0;

  vkDeviceWaitIdle(r_vk_state.device.logical);
  vkResetDescriptorPool(r_vk_state.device.logical, r_vk_state.descriptor_pool.pool, 0);
}

func void R_VK_BeginRenderPass()
{
  R_VK_CommandBuffer* command_buffer  = r_vk_state.current_command_buffer;
  R_VK_Framebuffer*   framebuffer     = r_vk_state.current_framebuffer;
  R_VK_RenderPass*    render_pass     = &r_vk_state.render_pass;

  VkRenderPassBeginInfo begin_info = {};
  begin_info.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  begin_info.renderPass               = render_pass->handle;
  begin_info.framebuffer              = framebuffer->handle;
  begin_info.renderArea.offset.x      = render_pass->render_area.x0;
  begin_info.renderArea.offset.y      = render_pass->render_area.y0;
  begin_info.renderArea.extent.width  = render_pass->render_area.x1 - render_pass->render_area.x0;
  begin_info.renderArea.extent.height = render_pass->render_area.y1 - render_pass->render_area.y0;

  VkClearValue color_clear_value = {};
  color_clear_value.color.float32[0] = render_pass->clear_color.r;
  color_clear_value.color.float32[1] = render_pass->clear_color.g;
  color_clear_value.color.float32[2] = render_pass->clear_color.b;
  color_clear_value.color.float32[3] = render_pass->clear_color.a;

  VkClearValue depth_stencil_clear_value = {};
  depth_stencil_clear_value.depthStencil.depth   = render_pass->clear_depth;
  depth_stencil_clear_value.depthStencil.stencil = render_pass->clear_stencil;

  VkClearValue clear_values[2] = { color_clear_value, depth_stencil_clear_value };

  begin_info.clearValueCount = CountArrayElements(clear_values);
  begin_info.pClearValues    = clear_values;

  vkCmdBeginRenderPass(command_buffer->handle, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

func void R_VK_EndRenderPass()
{
  vkCmdEndRenderPass(r_vk_state.current_command_buffer->handle);
}

func void R_VK_Draw(R_DrawInfo* info)
{
  R_VK_BindPipeline(info->pipeline);

  VkDeviceSize vertex_offset = r_vk_state.big_buffer.current_position;
  memcpy((u8*)r_vk_state.big_buffer.mapped_memory + r_vk_state.big_buffer.current_position, info->vertecies, info->vertex_count * info->vertex_size);
  r_vk_state.big_buffer.current_position += info->vertex_count * info->vertex_size;

  VkDeviceSize index_offset = r_vk_state.big_buffer.current_position;
  memcpy((u8*)r_vk_state.big_buffer.mapped_memory + r_vk_state.big_buffer.current_position, info->indecies, info->index_count * info->index_size);
  r_vk_state.big_buffer.current_position += info->index_count * info->index_size;

  u32 alligment = 64 - (r_vk_state.big_buffer.current_position % 64);
  r_vk_state.big_buffer.current_position += alligment;
  VkDeviceSize set_offset = r_vk_state.big_buffer.current_position;
  memcpy((u8*)r_vk_state.big_buffer.mapped_memory + r_vk_state.big_buffer.current_position, info->uniform_data, info->uniform_data_size);
  r_vk_state.big_buffer.current_position += info->uniform_data_size;

  VkDescriptorSetAllocateInfo set_info = {};
  set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  set_info.descriptorPool = r_vk_state.descriptor_pool.pool;
  set_info.descriptorSetCount = 1;
  set_info.pSetLayouts = &r_vk_state.pipelines[r_vk_state.active_pipeline_index].set_layout;

  VkDescriptorSet current_set;
  VK_CHECK(vkAllocateDescriptorSets(r_vk_state.device.logical, &set_info, &current_set));

  VkDescriptorBufferInfo buffer_info = {};
  buffer_info.buffer = r_vk_state.big_buffer.buffer;
  buffer_info.offset = set_offset;
  buffer_info.range  = info->uniform_data_size;

  VkDescriptorImageInfo image_info = {};
  image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  image_info.imageView   = r_vk_state.texture.vk_view;
  image_info.sampler     = r_vk_state.sampler;

  R_Pipeline* active_pipeline = r_vk_state.pipelines[r_vk_state.active_pipeline_index].r_pipeline;

  VkWriteDescriptorSet write_sets[5] = {};
  for (u32 i = 0; i < active_pipeline->bindings_count; i += 1)
  {
    write_sets[i].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_sets[i].dstSet          = current_set;
    write_sets[i].dstBinding      = i;
    write_sets[i].dstArrayElement = 0;
    write_sets[i].descriptorType  = R_VK_DescriptorTypeFromBindingType(active_pipeline->bindings[i].type);
    write_sets[i].descriptorCount = 1;
    write_sets[i].pBufferInfo     = &buffer_info;
    write_sets[i].pImageInfo      = &image_info;
  }

  vkUpdateDescriptorSets(r_vk_state.device.logical, active_pipeline->bindings_count, write_sets, 0, 0);

  vkCmdBindDescriptorSets(
    r_vk_state.current_command_buffer->handle, VK_PIPELINE_BIND_POINT_GRAPHICS, r_vk_state.pipelines[r_vk_state.active_pipeline_index].layout,
    0, 1, &current_set, 0, 0
  );

  vkCmdBindVertexBuffers(r_vk_state.current_command_buffer->handle, 0, 1, &r_vk_state.big_buffer.buffer, &vertex_offset);
  vkCmdBindIndexBuffer(r_vk_state.current_command_buffer->handle, r_vk_state.big_buffer.buffer, index_offset, VK_INDEX_TYPE_UINT32);

  vkCmdDrawIndexed(r_vk_state.current_command_buffer->handle, info->index_count, 1, 0, 0, 0);
}

// -------------------------------------------------------------------
// --AlNov: Command Buffer -------------------------------------------
func void R_VK_CreateCommandPool(R_VK_State* vk_state)
{
  VkCommandPoolCreateInfo pool_info = {};
  pool_info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_info.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  pool_info.queueFamilyIndex = vk_state->device.queue_index;

  VK_CHECK(vkCreateCommandPool(vk_state->device.logical, &pool_info, 0, &vk_state->command_pool));
}

func void R_VK_AllocateCommandBuffer(R_VK_State* vk_state, VkCommandPool pool, R_VK_CommandBuffer* out_command_buffer)
{
  VkCommandBufferAllocateInfo command_buffer_info = {};
  command_buffer_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  command_buffer_info.commandPool        = pool;
  command_buffer_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  command_buffer_info.commandBufferCount = 1;

  out_command_buffer->state = R_VK_COMMAND_BUFFER_STATE_NOT_ALLOCATED;
  VK_CHECK(vkAllocateCommandBuffers(vk_state->device.logical, &command_buffer_info, &out_command_buffer->handle));
  out_command_buffer->state = R_VK_COMMAND_BUFFER_STATE_READY;
}

func void R_VK_FreeCommandBuffer(R_VK_State* vk_state, VkCommandPool pool, R_VK_CommandBuffer* command_buffer)
{
  vkFreeCommandBuffers(vk_state->device.logical, pool, 1, &command_buffer->handle);

  command_buffer->handle = 0;
  command_buffer->state  = R_VK_COMMAND_BUFFER_STATE_NOT_ALLOCATED;
}

func void R_VK_BeginCommandBuffer(R_VK_CommandBuffer* command_buffer)
{
  VkCommandBufferBeginInfo begin_info = {};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  VK_CHECK(vkBeginCommandBuffer(command_buffer->handle, &begin_info));
  command_buffer->state = R_VK_COMMAND_BUFFER_STATE_RECORDING;
}

func void R_VK_EndCommandBuffer(R_VK_CommandBuffer* command_buffer)
{
  VK_CHECK(vkEndCommandBuffer(command_buffer->handle));
  command_buffer->state = R_VK_COMMAND_BUFFER_STATE_RECORDING_ENDED;
}

func void R_VK_SubmitComandBuffer(R_VK_CommandBuffer* command_buffer)
{
  command_buffer->state = R_VK_COMMAND_BUFFER_STATE_SUBMITTED;
}

func void R_VK_ResetCommandBuffer(R_VK_CommandBuffer* command_buffer)
{
  command_buffer->state= R_VK_COMMAND_BUFFER_STATE_READY;
}

func void R_VK_BeginSingleUseCommandBuffer(R_VK_State* vk_state, VkCommandPool pool, R_VK_CommandBuffer* out_command_buffer)
{
  R_VK_AllocateCommandBuffer(vk_state, pool, out_command_buffer);

  VkCommandBufferBeginInfo begin_info = {};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  VK_CHECK(vkBeginCommandBuffer(out_command_buffer->handle, &begin_info));
  out_command_buffer->state = R_VK_COMMAND_BUFFER_STATE_RECORDING;
}

func void R_VK_EndSingleUseCommandBuffer(R_VK_State* vk_state, VkCommandPool pool, R_VK_CommandBuffer* command_buffer, VkQueue queue)
{
  R_VK_EndCommandBuffer(command_buffer);

  VkSubmitInfo submit_info = {};
  submit_info.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers    = &command_buffer->handle;

  VK_CHECK(vkQueueSubmit(queue, 1, &submit_info, 0));

  VK_CHECK(vkQueueWaitIdle(queue));

  R_VK_FreeCommandBuffer(vk_state, pool, command_buffer);
}
// --AlNov: Command Buffer @END --------------------------------------

// -------------------------------------------------------------------
// --AlNov: Framebuffer ----------------------------------------------
func void R_VK_CreateFramebuffer(
  R_VK_State* vk_state, R_VK_RenderPass* render_pass, Vec2u size,
  u32 attachment_count, VkImageView* attachments, R_VK_Framebuffer* out_framebuffer
)
{
  out_framebuffer->attachments = (VkImageView*)PushArena(vk_state->arena, sizeof(VkImageView) * attachment_count);
  for (u32 i = 0; i < attachment_count; i += 1)
  {
    out_framebuffer->attachments[i] = attachments[i];
  }
  out_framebuffer->render_pass      = render_pass;
  out_framebuffer->attachment_count = attachment_count;

  VkFramebufferCreateInfo framebuffer_info = {};
  framebuffer_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  framebuffer_info.renderPass      = r_vk_state.render_pass.handle;
  framebuffer_info.attachmentCount = attachment_count;
  framebuffer_info.pAttachments    = out_framebuffer->attachments;
  framebuffer_info.width           = size.width;
  framebuffer_info.height          = size.height;
  framebuffer_info.layers          = 1;

  VK_CHECK(vkCreateFramebuffer(vk_state->device.logical, &framebuffer_info, 0, &out_framebuffer->handle));
}

func void R_VK_DestroyFramebuffer(R_VK_State* vk_state, R_VK_Framebuffer* framebuffer)
{
  vkDestroyFramebuffer(vk_state->device.logical, framebuffer->handle, 0);
  
  // --AlNov: @TODO @EROR There is memory leak. Don't free attachments
  *framebuffer = {};
}
// --AlNov: Framebuffer @END -----------------------------------------

// -------------------------------------------------------------------
// --AlNov: Shader ---------------------------------------------------
func void R_VK_BindShaderProgram(R_VK_CommandBuffer* command_buffer, R_VK_ShaderProgram* program)
{
  vkCmdBindPipeline(command_buffer->handle, VK_PIPELINE_BIND_POINT_GRAPHICS, program->pipeline.handle);
}
// --AlNov: Shader @END ----------------------------------------------

// -------------------------------------------------------------------
// --AlNov: Pipeline -------------------------------------------------

func b8 R_VK_CreatePipeline(R_Pipeline* pipeline)
{
  // --AlNov: Vertex Shader
  VkShaderModule vertex_shader_module;
  R_Shader* vertex_shader = &pipeline->shaders[R_SHADER_TYPE_VERTEX];
  {
    VkShaderModuleCreateInfo module_info = {};
    module_info.sType     = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    module_info.codeSize  = vertex_shader->code_size;
    module_info.pCode     = (u32*)vertex_shader->code;
    VK_CHECK(vkCreateShaderModule(r_vk_state.device.logical, &module_info, 0, &vertex_shader_module));
  }
  VkPipelineShaderStageCreateInfo vertex_shader_stage = {};
  vertex_shader_stage.sType   = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertex_shader_stage.stage   = R_VK_ShaderStageFromShaderType(vertex_shader->type);
  vertex_shader_stage.module  = vertex_shader_module;
  vertex_shader_stage.pName   = vertex_shader->entry_point;

  // --AlNov: Fragment Shader
  VkShaderModule fragment_shader_module;
  R_Shader* fragment_shader = &pipeline->shaders[R_SHADER_TYPE_FRAGMENT];
  {
    VkShaderModuleCreateInfo module_info = {};
    module_info.sType     = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    module_info.codeSize  = fragment_shader->code_size;
    module_info.pCode     = (u32*)fragment_shader->code;
    VK_CHECK(vkCreateShaderModule(r_vk_state.device.logical, &module_info, 0, &fragment_shader_module));
  }
  VkPipelineShaderStageCreateInfo fragment_shader_stage = {};
  fragment_shader_stage.sType   = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragment_shader_stage.stage   = R_VK_ShaderStageFromShaderType(fragment_shader->type);
  fragment_shader_stage.module  = fragment_shader_module;
  fragment_shader_stage.pName   = fragment_shader->entry_point;

  VkPipelineShaderStageCreateInfo stages[] = {
    vertex_shader_stage,
    fragment_shader_stage
  };

  VkDescriptorSetLayoutBinding bindings[10] = {};

  for (u32 i = 0; i < pipeline->bindings_count; i += 1)
  {
    bindings[i].binding         = i;
    bindings[i].descriptorType  = R_VK_DescriptorTypeFromBindingType(pipeline->bindings[i].type);
    bindings[i].descriptorCount = 1;
    bindings[i].stageFlags      = R_VK_ShaderStageFromShaderType(pipeline->bindings[i].shader_type);
  }

  VkDescriptorSetLayoutCreateInfo layout_info = {};
  layout_info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layout_info.bindingCount = pipeline->bindings_count;
  layout_info.pBindings    = bindings;

  VK_CHECK(vkCreateDescriptorSetLayout(r_vk_state.device.logical, &layout_info, 0, &r_vk_state.pipelines[r_vk_state.pipelines_count].set_layout));

  // --AlNov: @TODO Get rid of hardcoded size of array
  VkVertexInputAttributeDescription vertex_attributes[10] = {};

  u32 offset = 0;
  for (u32 i = 0; i < pipeline->attributes_count; i++)
  {
    vertex_attributes[i].location = i;
    vertex_attributes[i].binding  = 0;
    vertex_attributes[i].format   = R_VK_VkFormatFromAttributeFormat(pipeline->attributes[i]);
    vertex_attributes[i].offset   = offset;

    offset += R_H_OffsetFromAttributeFormat(pipeline->attributes[i]);
  }

  VkVertexInputBindingDescription vertex_description = {};
  vertex_description.binding   = 0;
  vertex_description.stride    = offset;
  vertex_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  VkPipelineVertexInputStateCreateInfo vertex_input_state_info = {};
  vertex_input_state_info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertex_input_state_info.vertexBindingDescriptionCount   = 1;
  vertex_input_state_info.pVertexBindingDescriptions      = &vertex_description;
  vertex_input_state_info.vertexAttributeDescriptionCount = pipeline->attributes_count;
  vertex_input_state_info.pVertexAttributeDescriptions    = vertex_attributes;

  VkPipelineInputAssemblyStateCreateInfo input_assembly_state_info = {};
  input_assembly_state_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  input_assembly_state_info.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  input_assembly_state_info.primitiveRestartEnable = VK_FALSE;

  // --AlNov: Viewport State
  VkViewport viewport = {};
  viewport.x        = 0;
  viewport.y        = 0;
  viewport.height   = r_vk_state.swapchain.size.height;
  viewport.width    = r_vk_state.swapchain.size.width;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor = {};
  scissor.offset        = {0, 0};
  scissor.extent.height = r_vk_state.swapchain.size.height;
  scissor.extent.width  = r_vk_state.swapchain.size.width;

  VkPipelineViewportStateCreateInfo viewport_state_info = {};
  viewport_state_info.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_state_info.viewportCount = 1;
  viewport_state_info.pViewports    = &viewport;
  viewport_state_info.scissorCount  = 1;
  viewport_state_info.pScissors     = &scissor;

  VkPipelineRasterizationStateCreateInfo rasterization_state_info = {};
  rasterization_state_info.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterization_state_info.depthClampEnable        = VK_FALSE;
  rasterization_state_info.rasterizerDiscardEnable = VK_FALSE;
  rasterization_state_info.polygonMode             = VK_POLYGON_MODE_FILL;
  rasterization_state_info.cullMode                = VK_CULL_MODE_BACK_BIT;
  rasterization_state_info.frontFace               = VK_FRONT_FACE_CLOCKWISE;
  rasterization_state_info.depthBiasEnable         = VK_FALSE;
  rasterization_state_info.lineWidth               = 1.0f;

  VkPipelineMultisampleStateCreateInfo multisample_state_info = {};
  multisample_state_info.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisample_state_info.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
  multisample_state_info.sampleShadingEnable   = VK_FALSE;
  multisample_state_info.minSampleShading      = 0.0f;
  multisample_state_info.pSampleMask           = nullptr;
  multisample_state_info.alphaToCoverageEnable = VK_FALSE;
  multisample_state_info.alphaToOneEnable      = VK_FALSE;

  VkPipelineDepthStencilStateCreateInfo depth_stencil_state_info = {};
  depth_stencil_state_info.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depth_stencil_state_info.depthTestEnable       = VK_TRUE;
  depth_stencil_state_info.depthWriteEnable      = VK_TRUE;
  depth_stencil_state_info.depthCompareOp        = VK_COMPARE_OP_LESS;
  depth_stencil_state_info.depthBoundsTestEnable = VK_FALSE;
  depth_stencil_state_info.stencilTestEnable     = VK_FALSE;

  VkPipelineColorBlendAttachmentState color_blend_attachment = {};
  color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT| VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
    | VK_COLOR_COMPONENT_A_BIT;
  color_blend_attachment.blendEnable    = VK_FALSE;

  VkPipelineColorBlendStateCreateInfo color_blend_state_info = {};
  color_blend_state_info.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  color_blend_state_info.logicOpEnable   = VK_FALSE;
  color_blend_state_info.attachmentCount = 1;
  color_blend_state_info.pAttachments    = &color_blend_attachment;

  {
    VkPipelineLayoutCreateInfo layout_info = {};
    layout_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_info.setLayoutCount         = 1;
    layout_info.pSetLayouts            = &r_vk_state.pipelines[r_vk_state.pipelines_count].set_layout;
    layout_info.pushConstantRangeCount = 0;
    layout_info.pPushConstantRanges    = 0;

    VK_CHECK(vkCreatePipelineLayout(r_vk_state.device.logical, &layout_info, 0, &r_vk_state.pipelines[r_vk_state.pipelines_count].layout));
  }

  VkGraphicsPipelineCreateInfo pipeline_info = {};
  pipeline_info.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_info.stageCount          = 2;
  pipeline_info.pStages             = stages;
  pipeline_info.pVertexInputState   = &vertex_input_state_info;
  pipeline_info.pInputAssemblyState = &input_assembly_state_info;
  pipeline_info.pViewportState      = &viewport_state_info;
  pipeline_info.pRasterizationState = &rasterization_state_info;
  pipeline_info.pMultisampleState   = &multisample_state_info;
  pipeline_info.pDepthStencilState  = &depth_stencil_state_info;
  pipeline_info.pColorBlendState    = &color_blend_state_info;
  pipeline_info.pDynamicState       = 0;
  pipeline_info.layout              = r_vk_state.pipelines[r_vk_state.pipelines_count].layout;
  pipeline_info.renderPass          = r_vk_state.render_pass.handle;
  pipeline_info.subpass             = 0;

  VK_CHECK(vkCreateGraphicsPipelines(r_vk_state.device.logical, 0, 1, &pipeline_info, nullptr, &r_vk_state.pipelines[r_vk_state.pipelines_count].handle));

  pipeline->backend_handle = r_vk_state.pipelines_count;
  r_vk_state.pipelines[r_vk_state.pipelines_count].r_pipeline = pipeline;

  r_vk_state.pipelines_count += 1;

  return true;
}

func void R_VK_BindPipeline(R_Pipeline* pipeline)
{
  r_vk_state.active_pipeline_index = pipeline->backend_handle;
  vkCmdBindPipeline(r_vk_state.current_command_buffer->handle, VK_PIPELINE_BIND_POINT_GRAPHICS, r_vk_state.pipelines[r_vk_state.active_pipeline_index].handle);
}
// --AlNov: Pipeline @END --------------------------------------------

// -------------------------------------------------------------------
// --AlNov: Helpers --------------------------------------------------
func u32 R_VK_FindMemoryType(u32 filter, VkMemoryPropertyFlags flags)
{
  VkPhysicalDeviceMemoryProperties mem_properties = {};
  vkGetPhysicalDeviceMemoryProperties(r_vk_state.device.physical, &mem_properties);

  for (u32 type_index = 0; type_index < mem_properties.memoryTypeCount; type_index += 1)
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

func void R_VK_CreateBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags property_flags, u32 size, VkBuffer* out_buffer, VkDeviceMemory* out_memory)
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

func void R_VK_MemCopy(VkDeviceMemory memory, void* data, u64 size)
{
  void* mapped_memory;
  vkMapMemory(r_vk_state.device.logical, memory, 0, size, 0, &mapped_memory);
  {
    memcpy(mapped_memory, data, size);
  }
  vkUnmapMemory(r_vk_state.device.logical, memory);
}

func VkShaderStageFlagBits R_VK_ShaderStageFromShaderType(R_ShaderType type)
{
  switch (type)
  {
    case R_SHADER_TYPE_VERTEX   : return VK_SHADER_STAGE_VERTEX_BIT;
    case R_SHADER_TYPE_FRAGMENT : return VK_SHADER_STAGE_FRAGMENT_BIT;

    default: ASSERT(1); return VK_SHADER_STAGE_FRAGMENT_BIT; // --AlNov: type is not supported by Vulkan Layer
  }
}

func VkFormat R_VK_VkFormatFromAttributeFormat(R_VertexAttributeFormat format)
{
  switch (format)
  {
    case R_VERTEX_ATTRIBUTE_FORMAT_VEC3F  : return VK_FORMAT_R32G32B32_SFLOAT;
    case R_VERTEX_ATTRIBUTE_FORMAT_VEC2F  : return VK_FORMAT_R32G32_SFLOAT;

    default: ASSERT(1); return VK_FORMAT_R32_SFLOAT; // --AlNov: format is not supported by Vulkan Layer
  }
}

func VkDescriptorType R_VK_DescriptorTypeFromBindingType(R_BindingType type)
{
  switch (type)
  {
    case R_BINDING_TYPE_UNIFORM_BUFFER: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case R_BINDING_TYPE_TEXTURE_2D:     return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    default: ASSERT(1); return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // --AlNov: Binding type not supported by Vulkan Layer
  }
}

func VkCommandBuffer R_VK_BeginSingleCommands()
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

func void R_VK_EndSingleCommands(VkCommandBuffer command_buffer)
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

func void R_VK_CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size)
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

func void R_VK_CopyBufferToImage(VkBuffer buffer, VkImage image, Vec2u image_dimensions)
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

func void R_VK_TransitImageLayout(VkImage image, VkFormat format, u32 layer_count, VkImageLayout old_layout, VkImageLayout new_layout)
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

func bool LoadTexture(const char* path, u8** out_data, i32* out_width, i32* out_height, i32* out_channels)
{
  *out_data = stbi_load(path, out_width, out_height, out_channels, STBI_rgb_alpha);

  if (!out_data)
  {
    LOG_ERROR("Cannot load texture %s\n", path);
    return false;
  }

  return true;
}

func R_Texture R_VK_CreateTexture(const char* path)
{
  R_Texture texture = {};

  i32 tex_width    = 0;
  i32 tex_height   = 0;
  i32 tex_channels = 0;
  u8* tex_pixels   = 0;
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

func void R_VK_CreateCubeMap(const char* folder_path, R_VK_CubeMap* out_cubemap)
{
  i32 width = 0;
  i32 height = 0;
  i32 channels = 0;
  u8* datas[R_CUBE_MAP_SIDE_TYPE_COUNT] = {};
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
    for (u32 i = 0; i < R_CUBE_MAP_SIDE_TYPE_COUNT; i += 1)
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
        copy_info.imageExtent = { (u32)width, (u32)height, 1 };

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
func R_View R_CreateView(Vec3f position, f32 fov, Vec2f size)
{
  R_View view = {};
  view.size               = size;
  view.position           = position;
  view.fov                = fov;
  view.uniform.projection = MakePerspective4x4f(fov, size.x / size.y, 0.1f, 100.0f);

  return view;
}

func void R_VK_BindView(R_View view)
{
  r_vk_state.view = view;
}
