#include "r_init_vk.h"

#pragma comment(lib, "third_party/vulkan/lib/vulkan-1.lib")

// --AlNov: Debug Layer Staff ----------------------------------------
#include <stdio.h>

global VkDebugUtilsMessengerEXT R_VK_DebugMessenger;

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
  printf("VK_VALIDATION: %s\n\n", pCallbackData->pMessage);

  return VK_FALSE;
}

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& messengerInfo)
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
  messengerInfo.pUserData = nullptr;
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

func void R_VK_Init(OS_Window* window)
{
  r_vk_state = {};
  r_vk_state.arena = AllocateArena(Megabytes(128));

  R_VK_CreateInstance();
  R_VK_CreateDevice();
  R_VK_CreateSurface(window);
  R_VK_CreateSwapchain();
  R_VK_CreateDescriptorPool();
  R_VK_CreateMvpSetLayout();
  R_VK_CreateRenderPass();
  R_VK_CreateMeshPipeline();
  R_VK_CreateLinePipeline();
  R_VK_CreateFramebuffers();
  R_VK_CreateCommandPool();
  R_VK_AllocateCommandBuffers();
  R_VK_CreateSyncTools();
  R_VK_CreateVertexBuffer();
  R_VK_CreateIndexBuffer();
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

  vkCreateInstance(&instance_info, 0, &r_vk_state.instance);

  createDebugMessenger(r_vk_state.instance, &R_VK_DebugMessenger);
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

    printf("GPU NAME: %s\n", properties.deviceName);

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

    vkCreateDevice(r_vk_state.device.physical, &device_info, 0, &r_vk_state.device.logical);
  }
  FreeArena(tmp_arena);
}

func void R_VK_CreateSurface(OS_Window* window)
{
  VkWin32SurfaceCreateInfoKHR surface_info = {};
  surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  surface_info.hinstance = window->instance;
  surface_info.hwnd = window->handle;

  vkCreateWin32SurfaceKHR(r_vk_state.instance, &surface_info, 0, &r_vk_state.window_resources.surface);

  // Get Surface Capabilities
  VkSurfaceCapabilitiesKHR capabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(r_vk_state.device.physical, r_vk_state.window_resources.surface, &capabilities);

  r_vk_state.window_resources.size.width = capabilities.currentExtent.width;
  r_vk_state.window_resources.size.height = capabilities.currentExtent.height;

  r_vk_state.window_resources.image_count = capabilities.minImageCount + 1;
  if (capabilities.maxImageCount > 0 && r_vk_state.window_resources.image_count > capabilities.maxImageCount) {
    r_vk_state.window_resources.image_count= capabilities.maxImageCount;
  }

  // Get Surface Format
  Arena* tmp_arena = AllocateArena(Kilobytes(64));
  {
    u32 format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(r_vk_state.device.physical, r_vk_state.window_resources.surface, &format_count, 0);
    VkSurfaceFormatKHR* formats = (VkSurfaceFormatKHR*)PushArena(tmp_arena, format_count * sizeof(VkSurfaceFormatKHR));
    vkGetPhysicalDeviceSurfaceFormatsKHR(r_vk_state.device.physical, r_vk_state.window_resources.surface, &format_count, formats);

    for (u32 i = 0; i < format_count; i += 1)
    {
      if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && formats[i].colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
        r_vk_state.window_resources.surface_format = formats[i];
      }
    }
  }
  FreeArena(tmp_arena);
}

func void R_VK_CreateSwapchain()
{
  VkSwapchainCreateInfoKHR swapchain_info = {};
  swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchain_info.surface = r_vk_state.window_resources.surface;
  swapchain_info.minImageCount = r_vk_state.window_resources.image_count;
  swapchain_info.imageFormat = r_vk_state.window_resources.surface_format.format;
  swapchain_info.imageColorSpace = r_vk_state.window_resources.surface_format.colorSpace;
  swapchain_info.imageExtent.width = r_vk_state.window_resources.size.width;
  swapchain_info.imageExtent.height = r_vk_state.window_resources.size.height;
  swapchain_info.imageArrayLayers = 1;
  swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  swapchain_info.queueFamilyIndexCount = 0;
  swapchain_info.pQueueFamilyIndices = 0;
  swapchain_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchain_info.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
  swapchain_info.clipped = VK_TRUE;
  // --AlNov: @TODO add oldSwapchain
  swapchain_info.oldSwapchain = VK_NULL_HANDLE;

  vkCreateSwapchainKHR(r_vk_state.device.logical, &swapchain_info, 0, &r_vk_state.window_resources.swapchain);

  vkGetSwapchainImagesKHR(r_vk_state.device.logical, r_vk_state.window_resources.swapchain, &r_vk_state.window_resources.image_count, 0);
  // --AlNov: @TODO Images doesnt deleted on swapchain recreation
  r_vk_state.window_resources.images = (VkImage*)PushArena(r_vk_state.arena, r_vk_state.window_resources.image_count * sizeof(VkImage));
  vkGetSwapchainImagesKHR(r_vk_state.device.logical, r_vk_state.window_resources.swapchain, &r_vk_state.window_resources.image_count, r_vk_state.window_resources.images);

  r_vk_state.window_resources.image_views = (VkImageView*)PushArena(r_vk_state.arena, r_vk_state.window_resources.image_count * sizeof(VkImageView));
  for (u32 i = 0; i < r_vk_state.window_resources.image_count; i += 1)
  {
    VkImageViewCreateInfo image_view_info = {};
    image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_info.image = r_vk_state.window_resources.images[i];
    image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_info.format = r_vk_state.window_resources.surface_format.format;
    image_view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_info.subresourceRange.baseMipLevel = 0;
    image_view_info.subresourceRange.levelCount = 1;
    image_view_info.subresourceRange.baseArrayLayer = 0;
    image_view_info.subresourceRange.layerCount = 1;

    vkCreateImageView(r_vk_state.device.logical, &image_view_info, 0, &r_vk_state.window_resources.image_views[i]);
  }
}

func void R_VK_CreateCommandPool()
{
  VkCommandPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  pool_info.queueFamilyIndex = r_vk_state.device.queue_index;

  vkCreateCommandPool(r_vk_state.device.logical, &pool_info, 0, &r_vk_state.cmd_pool.pool);
}

func void R_VK_CreateDescriptorPool()
{
  u32 descriptor_count = 100;

  VkDescriptorPoolSize pool_size = {};
  pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  pool_size.descriptorCount = descriptor_count;

  VkDescriptorPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.maxSets = descriptor_count;
  pool_info.poolSizeCount = 1;
  pool_info.pPoolSizes = &pool_size;

  vkCreateDescriptorPool(r_vk_state.device.logical, &pool_info, 0, &r_vk_state.descriptor_pool.pool);
}

func void R_VK_CreateMvpSetLayout()
{
  VkDescriptorSetLayoutBinding binding_info = {};
  binding_info.binding = 0;
  binding_info.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  binding_info.descriptorCount = 1;
  binding_info.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  VkDescriptorSetLayoutCreateInfo layout_info = {};
  layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layout_info.bindingCount = 1;
  layout_info.pBindings = &binding_info;

  vkCreateDescriptorSetLayout(r_vk_state.device.logical, &layout_info, 0, &r_vk_state.mvp_layout);
}

func void R_VK_CreateRenderPass()
{
  VkAttachmentDescription color_attachment = {};
  color_attachment.format = r_vk_state.window_resources.surface_format.format;
  color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference color_attachment_reference = {};
  color_attachment_reference.attachment = 0;
  color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.inputAttachmentCount = 0;
  subpass.pInputAttachments = 0;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_attachment_reference;
  subpass.pResolveAttachments = 0;
  subpass.pDepthStencilAttachment = 0;
  subpass.preserveAttachmentCount = 0;
  subpass.pPreserveAttachments = 0;

  VkSubpassDependency subpass_dependency = {};
  subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  subpass_dependency.dstSubpass = 0;
  subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpass_dependency.srcAccessMask = 0;
  subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  VkRenderPassCreateInfo render_pass_info = {};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_info.attachmentCount = 1;
  render_pass_info.pAttachments = &color_attachment;
  render_pass_info.subpassCount = 1;
  render_pass_info.pSubpasses = &subpass;
  render_pass_info.dependencyCount = 1;
  render_pass_info.pDependencies = &subpass_dependency;

  vkCreateRenderPass(r_vk_state.device.logical, &render_pass_info, 0, &r_vk_state.render_pass);
}

func void R_VK_CreateMeshPipeline()
{
  // --AlNov: Vertex Shader
  Arena* tmp_arena = AllocateArena(Kilobytes(4000));
  {
    const char* vs_path = "data/shaders/default2DVS.spv";
    FILE* vs_file = 0;
    vs_file = fopen(vs_path, "rb");
    if (!vs_file)
    {
      printf("Cannot open file %s\n", vs_path);
      return;
    }
    fseek(vs_file, 0L, SEEK_END);
    u32 vs_file_size = ftell(vs_file);
    u8* vs_code = (u8*)PushArena(tmp_arena, vs_file_size * sizeof(u8));
    rewind(vs_file);
    fread(vs_code, vs_file_size * sizeof(u8), 1, vs_file);
    fclose(vs_file);

    VkShaderModuleCreateInfo vs_module_info = {};
    vs_module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vs_module_info.codeSize = vs_file_size;
    vs_module_info.pCode = (u32*)vs_code;

    VkShaderModule vs_module = {};
    vkCreateShaderModule(r_vk_state.device.logical, &vs_module_info, 0, &vs_module);

    // --AlNov: Fragment Shader
    const char* fs_path = "data/shaders/default2DFS.spv";
    FILE* fs_file = fopen(fs_path, "rb");
    if (!fs_file)
    {
      printf("Cannot open file %s\n", fs_path);
      return;
    }
    fseek(fs_file, 0L, SEEK_END);
    u32 fs_file_size = ftell(fs_file);
    u8* fs_code = (u8*)PushArena(tmp_arena, fs_file_size * sizeof(u8));
    rewind(fs_file);
    fread(fs_code, fs_file_size * sizeof(u8), 1, fs_file);
    fclose(fs_file);

    VkShaderModuleCreateInfo fs_module_info = {};
    fs_module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    fs_module_info.codeSize = fs_file_size;
    fs_module_info.pCode = (u32*)fs_code;

    VkShaderModule fs_module = {};
    vkCreateShaderModule(r_vk_state.device.logical, &fs_module_info, 0, &fs_module);

    VkPipelineShaderStageCreateInfo vertex_shader_info = {};
    vertex_shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex_shader_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertex_shader_info.module = vs_module;
    vertex_shader_info.pName = "main";

    VkPipelineShaderStageCreateInfo fragment_shader_info = {};
    fragment_shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragment_shader_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragment_shader_info.module = fs_module;
    fragment_shader_info.pName = "main";

    VkPipelineShaderStageCreateInfo stages[] = {
      vertex_shader_info,
      fragment_shader_info,
    };

    VkVertexInputBindingDescription vertex_description = {};
    vertex_description.binding = 0;
    vertex_description.stride = sizeof(Vec3f);
    vertex_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription vertex_attribute_description = {};
    vertex_attribute_description.location = 0;
    vertex_attribute_description.binding = 0;
    vertex_attribute_description.format = VK_FORMAT_R32G32B32_SFLOAT;
    vertex_attribute_description.offset = 0;

    VkPipelineVertexInputStateCreateInfo vertex_input_state_info = {};
    vertex_input_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_state_info.vertexBindingDescriptionCount = 1;
    vertex_input_state_info.pVertexBindingDescriptions = &vertex_description;
    vertex_input_state_info.vertexAttributeDescriptionCount = 1;
    vertex_input_state_info.pVertexAttributeDescriptions = &vertex_attribute_description;

    VkPipelineInputAssemblyStateCreateInfo input_assembly_state_info = {};
    input_assembly_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_state_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_state_info.primitiveRestartEnable = VK_FALSE;

    // --AlNov: Viewport State
    VkViewport viewport = {};
    viewport.x = 0;
    viewport.y = 0;
    viewport.height = r_vk_state.window_resources.size.height;
    viewport.width = r_vk_state.window_resources.size.width;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent.height = r_vk_state.window_resources.size.height;
    scissor.extent.width = r_vk_state.window_resources.size.width;

    VkPipelineViewportStateCreateInfo viewport_state_info = {};
    viewport_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state_info.viewportCount = 1;
    viewport_state_info.pViewports = &viewport;
    viewport_state_info.scissorCount = 1;
    viewport_state_info.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterization_state_info = {};
    rasterization_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization_state_info.depthClampEnable = VK_FALSE;
    rasterization_state_info.rasterizerDiscardEnable = VK_FALSE;
    rasterization_state_info.polygonMode = VK_POLYGON_MODE_LINE;
    // rasterization_state_info.polygonMode = VK_POLYGON_MODE_FILL;
    rasterization_state_info.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterization_state_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterization_state_info.depthBiasEnable = VK_FALSE;
    rasterization_state_info.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisample_state_info = {};
    multisample_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_state_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisample_state_info.sampleShadingEnable = VK_FALSE;
    multisample_state_info.minSampleShading = 0.0f;
    multisample_state_info.pSampleMask = nullptr;
    multisample_state_info.alphaToCoverageEnable = VK_FALSE;
    multisample_state_info.alphaToOneEnable = VK_FALSE;

    VkPipelineDepthStencilStateCreateInfo depth_stencil_state_info = {};
    depth_stencil_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil_state_info.depthTestEnable = VK_FALSE;
    depth_stencil_state_info.depthWriteEnable = VK_FALSE;
    depth_stencil_state_info.depthBoundsTestEnable = VK_FALSE;
    depth_stencil_state_info.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState color_blend_attachment = {};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT| VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
      | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo color_blend_state_info = {};
    color_blend_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_state_info.logicOpEnable = VK_FALSE;
    color_blend_state_info.attachmentCount = 1;
    color_blend_state_info.pAttachments = &color_blend_attachment;

    VkPipelineLayoutCreateInfo layout_info = {};
    layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_info.setLayoutCount = 1;
    layout_info.pSetLayouts = &r_vk_state.mvp_layout;
    layout_info.pushConstantRangeCount = 0;
    layout_info.pPushConstantRanges = nullptr;

    vkCreatePipelineLayout(r_vk_state.device.logical, &layout_info, 0, &r_vk_state.mesh_pipeline.layout);

    VkGraphicsPipelineCreateInfo pipeline_info = {};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = stages;
    pipeline_info.pVertexInputState = &vertex_input_state_info;
    pipeline_info.pInputAssemblyState = &input_assembly_state_info;
    pipeline_info.pViewportState = &viewport_state_info;
    pipeline_info.pRasterizationState = &rasterization_state_info;
    pipeline_info.pMultisampleState = &multisample_state_info;
    pipeline_info.pDepthStencilState = &depth_stencil_state_info;
    pipeline_info.pColorBlendState = &color_blend_state_info;
    pipeline_info.pDynamicState = 0;
    pipeline_info.layout = r_vk_state.mesh_pipeline.layout;
    pipeline_info.renderPass = r_vk_state.render_pass;
    pipeline_info.subpass = 0;

    vkCreateGraphicsPipelines(r_vk_state.device.logical, 0, 1, &pipeline_info, nullptr, &r_vk_state.mesh_pipeline.pipeline);
  }
  FreeArena(tmp_arena);
}

func void R_VK_CreateLinePipeline()
{
  // --AlNov: Vertex Shader
  Arena* tmp_arena = AllocateArena(Kilobytes(4000));
  {
    const char* vs_path = "data/shaders/lineVS.spv";
    FILE* vs_file = 0;
    vs_file = fopen(vs_path, "rb");
    if (!vs_file)
    {
      printf("Cannot open file %s\n", vs_path);
      return;
    }
    fseek(vs_file, 0L, SEEK_END);
    u32 vs_file_size = ftell(vs_file);
    u8* vs_code = (u8*)PushArena(tmp_arena, vs_file_size * sizeof(u8));
    rewind(vs_file);
    fread(vs_code, vs_file_size * sizeof(u8), 1, vs_file);
    fclose(vs_file);

    VkShaderModuleCreateInfo vs_module_info = {};
    vs_module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vs_module_info.codeSize = vs_file_size;
    vs_module_info.pCode = (u32*)vs_code;

    VkShaderModule vs_module = {};
    vkCreateShaderModule(r_vk_state.device.logical, &vs_module_info, 0, &vs_module);

    // --AlNov: Fragment Shader
    const char* fs_path = "data/shaders/lineFS.spv";
    FILE* fs_file = fopen(fs_path, "rb");
    if (!fs_file)
    {
      printf("Cannot open file %s\n", fs_path);
      return;
    }
    fseek(fs_file, 0L, SEEK_END);
    u32 fs_file_size = ftell(fs_file);
    u8* fs_code = (u8*)PushArena(tmp_arena, fs_file_size * sizeof(u8));
    rewind(fs_file);
    fread(fs_code, fs_file_size * sizeof(u8), 1, fs_file);
    fclose(fs_file);

    VkShaderModuleCreateInfo fs_module_info = {};
    fs_module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    fs_module_info.codeSize = fs_file_size;
    fs_module_info.pCode = (u32*)fs_code;

    VkShaderModule fs_module = {};
    vkCreateShaderModule(r_vk_state.device.logical, &fs_module_info, 0, &fs_module);

    VkPipelineShaderStageCreateInfo vertex_shader_info = {};
    vertex_shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex_shader_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertex_shader_info.module = vs_module;
    vertex_shader_info.pName = "main";

    VkPipelineShaderStageCreateInfo fragment_shader_info = {};
    fragment_shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragment_shader_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragment_shader_info.module = fs_module;
    fragment_shader_info.pName = "main";

    VkPipelineShaderStageCreateInfo stages[] = {
      vertex_shader_info,
      fragment_shader_info,
    };

    VkVertexInputBindingDescription vertex_description = {};
    vertex_description.binding = 0;
    vertex_description.stride = sizeof(Vec3f);
    vertex_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription vertex_attribute_description = {};
    vertex_attribute_description.location = 0;
    vertex_attribute_description.binding = 0;
    vertex_attribute_description.format = VK_FORMAT_R32G32B32_SFLOAT;
    vertex_attribute_description.offset = 0;

    VkPipelineVertexInputStateCreateInfo vertex_input_state_info = {};
    vertex_input_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_state_info.vertexBindingDescriptionCount = 1;
    vertex_input_state_info.pVertexBindingDescriptions = &vertex_description;
    vertex_input_state_info.vertexAttributeDescriptionCount = 1;
    vertex_input_state_info.pVertexAttributeDescriptions = &vertex_attribute_description;

    VkPipelineInputAssemblyStateCreateInfo input_assembly_state_info = {};
    input_assembly_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_state_info.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    input_assembly_state_info.primitiveRestartEnable = VK_FALSE;

    // --AlNov: Viewport State
    VkViewport viewport = {};
    viewport.x = 0;
    viewport.y = 0;
    viewport.height = r_vk_state.window_resources.size.height;
    viewport.width = r_vk_state.window_resources.size.width;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent.height = r_vk_state.window_resources.size.height;
    scissor.extent.width = r_vk_state.window_resources.size.width;

    VkPipelineViewportStateCreateInfo viewport_state_info = {};
    viewport_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state_info.viewportCount = 1;
    viewport_state_info.pViewports = &viewport;
    viewport_state_info.scissorCount = 1;
    viewport_state_info.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterization_state_info = {};
    rasterization_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization_state_info.depthClampEnable = VK_FALSE;
    rasterization_state_info.rasterizerDiscardEnable = VK_FALSE;
    rasterization_state_info.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterization_state_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterization_state_info.depthBiasEnable = VK_FALSE;
    rasterization_state_info.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisample_state_info = {};
    multisample_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_state_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisample_state_info.sampleShadingEnable = VK_FALSE;
    multisample_state_info.minSampleShading = 0.0f;
    multisample_state_info.pSampleMask = nullptr;
    multisample_state_info.alphaToCoverageEnable = VK_FALSE;
    multisample_state_info.alphaToOneEnable = VK_FALSE;

    VkPipelineDepthStencilStateCreateInfo depth_stencil_state_info = {};
    depth_stencil_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil_state_info.depthTestEnable = VK_FALSE;
    depth_stencil_state_info.depthWriteEnable = VK_FALSE;
    depth_stencil_state_info.depthBoundsTestEnable = VK_FALSE;
    depth_stencil_state_info.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState color_blend_attachment = {};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT| VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
      | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo color_blend_state_info = {};
    color_blend_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_state_info.logicOpEnable = VK_FALSE;
    color_blend_state_info.attachmentCount = 1;
    color_blend_state_info.pAttachments = &color_blend_attachment;

    VkPipelineLayoutCreateInfo layout_info = {};
    layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_info.setLayoutCount = 0;
    layout_info.pSetLayouts = 0;
    layout_info.pushConstantRangeCount = 0;
    layout_info.pPushConstantRanges = 0;

    vkCreatePipelineLayout(r_vk_state.device.logical, &layout_info, 0, &r_vk_state.line_pipeline.layout);

    VkGraphicsPipelineCreateInfo pipeline_info = {};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = stages;
    pipeline_info.pVertexInputState = &vertex_input_state_info;
    pipeline_info.pInputAssemblyState = &input_assembly_state_info;
    pipeline_info.pViewportState = &viewport_state_info;
    pipeline_info.pRasterizationState = &rasterization_state_info;
    pipeline_info.pMultisampleState = &multisample_state_info;
    pipeline_info.pDepthStencilState = &depth_stencil_state_info;
    pipeline_info.pColorBlendState = &color_blend_state_info;
    pipeline_info.pDynamicState = 0;
    pipeline_info.layout = r_vk_state.mesh_pipeline.layout;
    pipeline_info.renderPass = r_vk_state.render_pass;
    pipeline_info.subpass = 0;

    vkCreateGraphicsPipelines(r_vk_state.device.logical, 0, 1, &pipeline_info, 0, &r_vk_state.line_pipeline.pipeline);
  }
  FreeArena(tmp_arena);
}

func void R_VK_CreateFramebuffers()
{
  u32 image_count = r_vk_state.window_resources.image_count;
  r_vk_state.window_resources.framebuffers = (VkFramebuffer*)PushArena(r_vk_state.arena, image_count * sizeof(VkFramebuffer));

  for (u32 i = 0; i < image_count; i += 1)
  {
    VkFramebufferCreateInfo framebuffer_info = {};
    framebuffer_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_info.renderPass      = r_vk_state.render_pass;
    framebuffer_info.attachmentCount = 1;
    framebuffer_info.pAttachments    = &r_vk_state.window_resources.image_views[i];
    framebuffer_info.width           = r_vk_state.window_resources.size.width;
    framebuffer_info.height          = r_vk_state.window_resources.size.height;
    framebuffer_info.layers          = 1;

    vkCreateFramebuffer(r_vk_state.device.logical, &framebuffer_info, 0, &r_vk_state.window_resources.framebuffers[i]);
  }
}

func void R_VK_AllocateCommandBuffers()
{
  VkCommandBufferAllocateInfo cmd_buffer_info = {};
  cmd_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  cmd_buffer_info.commandPool = r_vk_state.cmd_pool.pool;
  cmd_buffer_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  cmd_buffer_info.commandBufferCount = CountArrayElements(r_vk_state.cmd_pool.buffers);

  vkAllocateCommandBuffers(r_vk_state.device.logical, &cmd_buffer_info, r_vk_state.cmd_pool.buffers);
}

func void R_VK_CreateSyncTools()
{
  for (i32 i = 0; i < NUM_FRAMES_IN_FLIGHT; i += 1)
  {
    VkSemaphoreCreateInfo semaphore_info = {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    vkCreateSemaphore(r_vk_state.device.logical, &semaphore_info, 0, &r_vk_state.sync_tools.image_available_semaphores[i]);
    vkCreateSemaphore(r_vk_state.device.logical, &semaphore_info, 0, &r_vk_state.sync_tools.image_ready_semaphores[i]);

    VkFenceCreateInfo fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    vkCreateFence(r_vk_state.device.logical, &fence_info, 0, &r_vk_state.sync_tools.fences[i]);
  }
}

func void R_VK_CreateVertexBuffer()
{
  r_vk_state.vertex_buffer = {};
  r_vk_state.vertex_buffer.size = Kilobytes(64);
  R_VK_CreateBuffer(
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT|VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
      r_vk_state.vertex_buffer.size, &r_vk_state.vertex_buffer.buffer, &r_vk_state.vertex_buffer.memory
      );

  vkMapMemory(r_vk_state.device.logical, r_vk_state.vertex_buffer.memory, 0, r_vk_state.vertex_buffer.size, 0, &r_vk_state.vertex_buffer.mapped_memory);  
}

func void R_VK_CreateIndexBuffer()
{
  r_vk_state.index_buffer = {};
  r_vk_state.index_buffer.size = Kilobytes(64);
  R_VK_CreateBuffer(
      VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT|VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
      r_vk_state.index_buffer.size, &r_vk_state.index_buffer.buffer, &r_vk_state.index_buffer.memory
      );

  vkMapMemory(r_vk_state.device.logical, r_vk_state.index_buffer.memory, 0, r_vk_state.index_buffer.size, 0, &r_vk_state.index_buffer.mapped_memory);
}

// -------------------------------------------------------------------
// --AlNov: Draw Functions -------------------------------------------
func void R_DrawFrame()
{
  local_persist u32 current_frame = 0;

  vkWaitForFences(r_vk_state.device.logical, 1, &r_vk_state.sync_tools.fences[current_frame], VK_TRUE, U64_MAX);

  // --AlNov: @TODO Read more about vkAcquireNextImageKHR in terms of synchonization
  u32 image_index;
  VkResult image_acquire_result = vkAcquireNextImageKHR(
      r_vk_state.device.logical, r_vk_state.window_resources.swapchain,
      U64_MAX, r_vk_state.sync_tools.image_available_semaphores[current_frame],
      0, &image_index
      );

  vkResetFences(r_vk_state.device.logical, 1, &r_vk_state.sync_tools.fences[current_frame]);

  // --AlNov: @TODO Return resize. Maybe event fix it
  // if (image_require_result == VK_ERROR_OUT_OF_DATE_KHR)
  // {
  //   R_VK_HandleWindowResize();
  //   return;
  // }

  VkCommandBuffer cmd_buffer = r_vk_state.cmd_pool.buffers[current_frame];
  vkResetCommandBuffer(cmd_buffer, 0);

  VkCommandBufferBeginInfo cmd_begin_info = {};
  cmd_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  vkBeginCommandBuffer(cmd_buffer, &cmd_begin_info);
  {
    VkClearValue color_clear_value = {};
    color_clear_value.color = { {0.05f, 0.05f, 0.05f, 1.0f} };

    VkRenderPassBeginInfo render_pass_begin_info = {};
    render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_info.renderPass = r_vk_state.render_pass;
    render_pass_begin_info.framebuffer = r_vk_state.window_resources.framebuffers[image_index];
    render_pass_begin_info.renderArea.extent.height = r_vk_state.window_resources.size.height;
    render_pass_begin_info.renderArea.extent.width = r_vk_state.window_resources.size.width;
    render_pass_begin_info.renderArea.offset.y = 0;
    render_pass_begin_info.renderArea.offset.y = 0;
    render_pass_begin_info.clearValueCount = 1;
    render_pass_begin_info.pClearValues = &color_clear_value;

    vkCmdBeginRenderPass(cmd_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    {
      vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, r_vk_state.mesh_pipeline.pipeline);

      // --AlNov: Draw Meshes
      for (R_Mesh* mesh_to_draw = r_vk_state.mesh_list.first; mesh_to_draw; mesh_to_draw = mesh_to_draw->next)
      {

        VkDeviceSize vertex_buffer_offsets[] = { r_vk_state.vertex_buffer.current_position };
        R_VK_PushVertexBuffer(&r_vk_state.vertex_buffer, mesh_to_draw->vertecies, mesh_to_draw->vertex_count * sizeof(R_MeshVertex));

        u32 index_buffer_offset = r_vk_state.index_buffer.current_position;
        R_VK_PushIndexBuffer(&r_vk_state.index_buffer, mesh_to_draw->indecies, mesh_to_draw->index_count * sizeof(u32));

        // MVP BUffer
        R_VK_CreateBuffer(
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT|VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            sizeof(mesh_to_draw->mvp), &mesh_to_draw->mvp_buffer, &mesh_to_draw->mvp_memory
            );
        R_VK_MemCopy(mesh_to_draw->mvp_memory, &mesh_to_draw->mvp, sizeof(mesh_to_draw->mvp));

        VkDescriptorSetAllocateInfo set_info = {};
        set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        set_info.descriptorPool = r_vk_state.descriptor_pool.pool;
        set_info.descriptorSetCount = 1;
        set_info.pSetLayouts = &r_vk_state.mvp_layout;

        vkAllocateDescriptorSets(r_vk_state.device.logical, &set_info, &mesh_to_draw->mvp_set);

        VkDescriptorBufferInfo buffer_info = {};
        buffer_info.buffer = mesh_to_draw->mvp_buffer;
        buffer_info.offset = 0;
        buffer_info.range = sizeof(mesh_to_draw->mvp);

        VkWriteDescriptorSet write_set = {};
        write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_set.dstSet = mesh_to_draw->mvp_set;
        write_set.dstBinding = 0;
        write_set.dstArrayElement = 0;
        write_set.descriptorCount = 1;
        write_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write_set.pBufferInfo = &buffer_info;

        vkUpdateDescriptorSets(r_vk_state.device.logical, 1, &write_set, 0, 0);

        vkCmdBindDescriptorSets(
            cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, r_vk_state.mesh_pipeline.layout,
            0, 1, &mesh_to_draw->mvp_set, 0, 0
            );

        vkCmdBindVertexBuffers(cmd_buffer, 0, 1, &r_vk_state.vertex_buffer.buffer, vertex_buffer_offsets);
        vkCmdBindIndexBuffer(cmd_buffer, r_vk_state.index_buffer.buffer, index_buffer_offset, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(cmd_buffer, mesh_to_draw->index_count, 1, 0, 0, 0);
      }

      // --AlNov: Draw Lines
      vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, r_vk_state.line_pipeline.pipeline);
      for (R_Line* line = r_vk_state.line_list.first; line; line = line->next)
      {
        VkDeviceSize vertex_buffer_offsets[] = { r_vk_state.vertex_buffer.current_position };
        R_VK_PushVertexBuffer(&r_vk_state.vertex_buffer, line->vertecies, 2 * sizeof(R_LineVertex));

        vkCmdBindVertexBuffers(cmd_buffer, 0, 1, &r_vk_state.vertex_buffer.buffer, vertex_buffer_offsets);

        vkCmdDraw(cmd_buffer, 2, 1, 0, 0);
      }
    }
    vkCmdEndRenderPass(cmd_buffer);
  }
  vkEndCommandBuffer(cmd_buffer);

  VkPipelineStageFlags wait_stage = {
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
  };

  VkSubmitInfo submit_info = {};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = &r_vk_state.sync_tools.image_available_semaphores[image_index];
  submit_info.pWaitDstStageMask = &wait_stage;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &r_vk_state.cmd_pool.buffers[current_frame];
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = &r_vk_state.sync_tools.image_ready_semaphores[current_frame];

  VkQueue queue;
  vkGetDeviceQueue(r_vk_state.device.logical, r_vk_state.device.queue_index, 0, &queue);

  vkQueueSubmit(queue, 1, &submit_info, r_vk_state.sync_tools.fences[current_frame]);

  VkPresentInfoKHR present_info = {};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores = &r_vk_state.sync_tools.image_ready_semaphores[current_frame];
  present_info.swapchainCount = 1;
  present_info.pSwapchains = &r_vk_state.window_resources.swapchain;
  present_info.pImageIndices = &image_index;
  present_info.pResults = 0;

  VkResult present_result = vkQueuePresentKHR(queue, &present_info);

  // --AlNov: @TODO Handle resize
  // if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR || R_WindowResources.bIsWindowResized)
  // {
  //   R_VK_HandleWindowResize();
  //   R_WindowResources.bIsWindowResized = false;
  //   return;
  // }

  current_frame = (current_frame + 1) % NUM_FRAMES_IN_FLIGHT;
}

func void R_EndFrame()
{
  r_vk_state.vertex_buffer.current_position = 0;
  r_vk_state.index_buffer.current_position = 0;

  for (R_Mesh* mesh_to_draw = r_vk_state.mesh_list.first; mesh_to_draw; mesh_to_draw = mesh_to_draw->next)
  {
    // --AlNov: @NOTE It is bad to recreate and delete.
    // But it is how it is now
    vkDeviceWaitIdle(r_vk_state.device.logical);

    vkDestroyBuffer(r_vk_state.device.logical, mesh_to_draw->mvp_buffer, 0);

    vkFreeMemory(r_vk_state.device.logical, mesh_to_draw->mvp_memory, 0);

    vkResetDescriptorPool(r_vk_state.device.logical, r_vk_state.descriptor_pool.pool, 0);
  }

  r_vk_state.mesh_list = {};
  r_vk_state.line_list = {};
}

// -------------------------------------------------------------------
// --AlNov: Helpers --------------------------------------------------

func void R_VK_CreateBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags property_flags, u32 size, VkBuffer* out_buffer, VkDeviceMemory* out_memory)
{
  VkBufferCreateInfo buffer_info = {};
  buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_info.size = size;
  buffer_info.usage = usage;
  buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  vkCreateBuffer(r_vk_state.device.logical, &buffer_info, 0, out_buffer);

  VkMemoryRequirements memory_requirements = {};
  vkGetBufferMemoryRequirements(r_vk_state.device.logical, *out_buffer, &memory_requirements);

  VkPhysicalDeviceMemoryProperties temp_properties = {};
  vkGetPhysicalDeviceMemoryProperties(r_vk_state.device.physical, &temp_properties);

  i32 memory_type_index = -1;
  for (u32 type_index = 0; type_index < temp_properties.memoryTypeCount; type_index += 1)
  {
    if (memory_requirements.memoryTypeBits & (1 << type_index)
        && ((temp_properties.memoryTypes[type_index].propertyFlags & property_flags) == property_flags)
       )
    {
      memory_type_index = type_index;
      break;
    }
  }

  VkMemoryAllocateInfo allocate_info = {};
  allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocate_info.allocationSize = memory_requirements.size;
  allocate_info.memoryTypeIndex = memory_type_index;
  vkAllocateMemory(r_vk_state.device.logical, &allocate_info, 0, out_memory);

  vkBindBufferMemory(r_vk_state.device.logical, *out_buffer, *out_memory, 0);
}

func void R_VK_PushVertexBuffer(R_VK_VertexBuffer* buffer, void* data, u64 size)
{
  memcpy((u8*)buffer->mapped_memory + buffer->current_position, data, size);
  buffer->current_position += size;
}

func void R_VK_PushIndexBuffer(R_VK_IndexBuffer* buffer, void* data, u64 size)
{
  memcpy((u8*)buffer->mapped_memory + buffer->current_position, data, size);
  buffer->current_position += size;
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

// -------------------------------------------------------------------
// --AlNov: Mesh List Functions
func void R_PushMesh(R_MeshList* list, R_Mesh* mesh)
{
  if (list->count == 0)
  {
    list->first = mesh;
    list->last = mesh;
    list->count += 1;

    mesh->next = 0;
    mesh->previous = 0;
  }
  else
  {
    mesh->previous = list->last;
    mesh->next = 0;
    list->last->next = mesh;
    list->last = mesh;
    list->count += 1;
  }
}

func void R_AddMeshToDrawList(R_Mesh* mesh)
{
  R_PushMesh(&r_vk_state.mesh_list, mesh);
}

func void R_PushLine(R_LineList* list, R_Line* line)
{
  if (list->count == 0)
  {
    list->first = line;
    list->last = line;
    list->count = 1;

    line->next = 0;
    line->previous = 0;
  }
  else
  {
    line->previous = list->last;
    line->next = 0;
    list->last->next = line;
    list->last = line;
    list->count += 1;
  }

}

func void R_AddLineToDrawList(R_Line* line)
{
  R_PushLine(&r_vk_state.line_list, line);
}
