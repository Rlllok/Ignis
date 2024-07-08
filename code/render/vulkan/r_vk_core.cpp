#include "r_vk_core.h"
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

func b8 R_VK_Init(OS_Window* window)
{
  r_vk_state = {};
  r_vk_state.arena = AllocateArena(Megabytes(128));

  R_VK_CreateInstance();
  R_VK_CreateDevice();
  R_VK_CreateSurface(window);
  R_VK_CreateSwapchain();
  R_VK_CreateDescriptorPool();
  R_VK_CreateMvpSetLayout();
  R_VK_CreateDepthImage();
  Rect2f render_area = {};
  render_area.x0 = 0.0f;
  render_area.y0 = 0.0f;
  render_area.x1 = r_vk_state.window_resources.size.width;
  render_area.y1 = r_vk_state.window_resources.size.height;
  R_VK_CreateRenderPass(&r_vk_state, &r_vk_state.render_pass, render_area, MakeVec4f(0.05f, 0.05f, 0.05f, 1.0f), 1.0f, 0);
  // R_VK_CreateMeshPipeline();
  // R_VK_CreateLinePipeline();
  R_VK_CreateSpherePipeline();
  R_VK_CreateFramebuffers();
  R_VK_CreateCommandPool();
  r_vk_state.texture = R_VK_CreateTexture("data/uv_checker.png");
  R_VK_AllocateCommandBuffers();
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

func void R_VK_CreateSurface(OS_Window* window)
{
  VkWin32SurfaceCreateInfoKHR surface_info = {};
  surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  surface_info.hinstance = window->instance;
  surface_info.hwnd = window->handle;

  VK_CHECK(vkCreateWin32SurfaceKHR(r_vk_state.instance, &surface_info, 0, &r_vk_state.window_resources.surface));

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

  VK_CHECK(vkCreateSwapchainKHR(r_vk_state.device.logical, &swapchain_info, 0, &r_vk_state.window_resources.swapchain));

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

    VK_CHECK(vkCreateImageView(r_vk_state.device.logical, &image_view_info, 0, &r_vk_state.window_resources.image_views[i]));
  }
}

func void R_VK_CreateCommandPool()
{
  VkCommandPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  pool_info.queueFamilyIndex = r_vk_state.device.queue_index;

  VK_CHECK(vkCreateCommandPool(r_vk_state.device.logical, &pool_info, 0, &r_vk_state.cmd_pool.pool));
}

func void R_VK_CreateDescriptorPool()
{
  u32 descriptor_count = 100;

  VkDescriptorPoolSize pool_size = {};
  pool_size.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  pool_size.descriptorCount = 50;

  VkDescriptorPoolSize sampler_pool_size = {};
  sampler_pool_size.type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  sampler_pool_size.descriptorCount = 5;

  VkDescriptorPoolSize pool_sizes[2] = { pool_size, sampler_pool_size };

  VkDescriptorPoolCreateInfo pool_info = {};
  pool_info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.maxSets       = descriptor_count;
  pool_info.poolSizeCount = 2;
  pool_info.pPoolSizes    = pool_sizes;

  VK_CHECK(vkCreateDescriptorPool(r_vk_state.device.logical, &pool_info, 0, &r_vk_state.descriptor_pool.pool));
}

func void R_VK_CreateMvpSetLayout()
{
  VkDescriptorSetLayoutBinding mvp_binding_info = {};
  mvp_binding_info.binding         = 0;
  mvp_binding_info.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  mvp_binding_info.descriptorCount = 1;
  mvp_binding_info.stageFlags      = VK_SHADER_STAGE_VERTEX_BIT;

  VkDescriptorSetLayoutBinding sampler_binding_info = {};
  sampler_binding_info.binding         = 1;
  sampler_binding_info.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  sampler_binding_info.descriptorCount = 1;
  sampler_binding_info.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorSetLayoutBinding bindings[2] = { mvp_binding_info, sampler_binding_info };

  VkDescriptorSetLayoutCreateInfo layout_info = {};
  layout_info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layout_info.bindingCount = 2;
  layout_info.pBindings    = bindings;

  VK_CHECK(vkCreateDescriptorSetLayout(r_vk_state.device.logical, &layout_info, 0, &r_vk_state.mvp_layout));
}

func void R_VK_CreateMeshPipeline()
{
  Arena* tmp_arena = AllocateArena(Megabytes(8));
  {
    R_VK_ShaderStage vertex_shader_stage   = R_VK_CreateShaderModule(tmp_arena, "data/shaders/default2DVS.spv", "main", R_VK_SHADER_TYPE_VERTEX);
    R_VK_ShaderStage fragment_shader_stage = R_VK_CreateShaderModule(tmp_arena, "data/shaders/default2DFS.spv", "main", R_VK_SHADER_TYPE_FRAGMENT);

    r_vk_state.mesh_pipeline = R_VK_CreatePipeline(&vertex_shader_stage, &fragment_shader_stage);
  }
  FreeArena(tmp_arena);
}

func void R_VK_CreateSpherePipeline()
{
  Arena* tmp_arena = AllocateArena(Megabytes(8));
  {
    R_VK_ShaderStage vertex_shader_stage   = R_VK_CreateShaderModule(tmp_arena, "data/shaders/default3DVS.spv", "main", R_VK_SHADER_TYPE_VERTEX);
    R_VK_ShaderStage fragment_shader_stage = R_VK_CreateShaderModule(tmp_arena, "data/shaders/default3DFS.spv", "main", R_VK_SHADER_TYPE_FRAGMENT);

    r_vk_state.sphere_pipeline = R_VK_CreatePipeline(&vertex_shader_stage, &fragment_shader_stage);
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
      LOG_ERROR("Cannot open file %s\n", vs_path);
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
      LOG_ERROR("Cannot open file %s\n", fs_path);
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
    // rasterization_state_info.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterization_state_info.cullMode = VK_CULL_MODE_NONE;
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
    pipeline_info.renderPass = r_vk_state.render_pass.handle;
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
    VkImageView attachments[2] = { r_vk_state.window_resources.image_views[i], r_vk_state.depth_view };

    VkFramebufferCreateInfo framebuffer_info = {};
    framebuffer_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_info.renderPass      = r_vk_state.render_pass.handle;
    framebuffer_info.attachmentCount = 2;
    framebuffer_info.pAttachments    = attachments;
    framebuffer_info.width           = r_vk_state.window_resources.size.width;
    framebuffer_info.height          = r_vk_state.window_resources.size.height;
    framebuffer_info.layers          = 1;

    VK_CHECK(vkCreateFramebuffer(r_vk_state.device.logical, &framebuffer_info, 0, &r_vk_state.window_resources.framebuffers[i]));
  }
}

func void R_VK_AllocateCommandBuffers()
{
  VkCommandBufferAllocateInfo cmd_buffer_info = {};
  cmd_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  cmd_buffer_info.commandPool = r_vk_state.cmd_pool.pool;
  cmd_buffer_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  cmd_buffer_info.commandBufferCount = CountArrayElements(r_vk_state.cmd_pool.buffers);

  VK_CHECK(vkAllocateCommandBuffers(r_vk_state.device.logical, &cmd_buffer_info, r_vk_state.cmd_pool.buffers));
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
  image_info.extent.width  = r_vk_state.window_resources.size.x;
  image_info.extent.height = r_vk_state.window_resources.size.y;
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
  color_attachment.format         = vk_state->window_resources.surface_format.format;
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

func void R_VK_BeginRenderPass(VkCommandBuffer* command_buffer, R_VK_RenderPass* render_pass, VkFramebuffer framebuffer)
{
  VkRenderPassBeginInfo begin_info = {};
  begin_info.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  begin_info.renderPass               = render_pass->handle;
  begin_info.framebuffer              = framebuffer;
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

  vkCmdBeginRenderPass(*command_buffer, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

func void R_VK_EndRenderPass(VkCommandBuffer* command_buffer, R_VK_RenderPass* render_pass)
{
  vkCmdEndRenderPass(*command_buffer);
}

// -------------------------------------------------------------------
// --AlNov: Pipeline Functions ---------------------------------------
func R_VK_ShaderStage R_VK_CreateShaderModule(Arena* arena, const char* path, const char* enter_point, R_VK_ShaderType type)
{
  // --AlNov: @TODO Remove Arena allocation
  R_VK_ShaderStage shader_stage = {};
  shader_stage.enter_point      = enter_point;
  shader_stage.type             = type;

  FILE* file = 0;

  file = fopen(path, "rb");
  if (!file)
  {
    LOG_ERROR("Cannot open file %s\n", path);
    return shader_stage;
  }

  fseek(file, 0L, SEEK_END);
  u32 file_size     = ftell(file);
  shader_stage.code = (u8*)PushArena(arena, file_size * sizeof(u8));
  rewind(file);
  fread(shader_stage.code, file_size * sizeof(u8), 1, file);
  fclose(file);

  VkShaderModuleCreateInfo module_info = {};
  module_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  module_info.codeSize = file_size;
  module_info.pCode    = (u32*)shader_stage.code;

  VK_CHECK(vkCreateShaderModule(r_vk_state.device.logical, &module_info, 0, &shader_stage.vk_handle));

  shader_stage.vk_info = {};
  shader_stage.vk_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shader_stage.vk_info.stage  = (type == R_VK_SHADER_TYPE_VERTEX) ? VK_SHADER_STAGE_VERTEX_BIT : VK_SHADER_STAGE_FRAGMENT_BIT;
  shader_stage.vk_info.module = shader_stage.vk_handle;
  shader_stage.vk_info.pName = shader_stage.enter_point;

  return shader_stage;
}

func R_VK_Pipeline R_VK_CreatePipeline(R_VK_ShaderStage* vertex_shader_stage, R_VK_ShaderStage* fragment_shader_stage)
{
  R_VK_Pipeline result = {};

  VkPipelineShaderStageCreateInfo stages[] = {
    vertex_shader_stage->vk_info,
    fragment_shader_stage->vk_info,
  };

  VkVertexInputBindingDescription vertex_description = {};
  vertex_description.binding   = 0;
  vertex_description.stride    = sizeof(R_MeshVertex);
  vertex_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  VkVertexInputAttributeDescription vertex_position_description = {};
  vertex_position_description.location = 0;
  vertex_position_description.binding  = 0;
  vertex_position_description.format   = VK_FORMAT_R32G32B32_SFLOAT;
  vertex_position_description.offset   = offsetof(R_MeshVertex, position);

  VkVertexInputAttributeDescription vertex_normal_description = {};
  vertex_normal_description.location = 1;
  vertex_normal_description.binding  = 0;
  vertex_normal_description.format   = VK_FORMAT_R32G32B32_SFLOAT;
  vertex_normal_description.offset   = offsetof(R_MeshVertex, normal);

  VkVertexInputAttributeDescription vertex_uv_description = {};
  vertex_uv_description.location = 2;
  vertex_uv_description.binding  = 0;
  vertex_uv_description.format   = VK_FORMAT_R32G32_SFLOAT;
  vertex_uv_description.offset   = offsetof(R_MeshVertex, uv);

  VkVertexInputAttributeDescription vertex_attributes[3] = {
    vertex_position_description,
    vertex_normal_description,
    vertex_uv_description
  };

  VkPipelineVertexInputStateCreateInfo vertex_input_state_info = {};
  vertex_input_state_info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertex_input_state_info.vertexBindingDescriptionCount   = 1;
  vertex_input_state_info.pVertexBindingDescriptions      = &vertex_description;
  vertex_input_state_info.vertexAttributeDescriptionCount = 3;
  vertex_input_state_info.pVertexAttributeDescriptions    = vertex_attributes;

  VkPipelineInputAssemblyStateCreateInfo input_assembly_state_info = {};
  input_assembly_state_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  input_assembly_state_info.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  input_assembly_state_info.primitiveRestartEnable = VK_FALSE;

  // --AlNov: Viewport State
  VkViewport viewport = {};
  viewport.x        = 0;
  viewport.y        = 0;
  viewport.height   = r_vk_state.window_resources.size.height;
  viewport.width    = r_vk_state.window_resources.size.width;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor = {};
  scissor.offset        = {0, 0};
  scissor.extent.height = r_vk_state.window_resources.size.height;
  scissor.extent.width  = r_vk_state.window_resources.size.width;

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

  VkPipelineLayoutCreateInfo layout_info = {};
  layout_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  layout_info.setLayoutCount         = 1;
  layout_info.pSetLayouts            = &r_vk_state.mvp_layout;
  layout_info.pushConstantRangeCount = 0;
  layout_info.pPushConstantRanges    = 0;

  VK_CHECK(vkCreatePipelineLayout(r_vk_state.device.logical, &layout_info, 0, &result.layout));

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
  pipeline_info.layout              = result.layout;
  pipeline_info.renderPass          = r_vk_state.render_pass.handle;
  pipeline_info.subpass             = 0;

  VK_CHECK(vkCreateGraphicsPipelines(r_vk_state.device.logical, 0, 1, &pipeline_info, nullptr, &result.pipeline));

  return result;
}

// -------------------------------------------------------------------
// --AlNov: Draw Functions -------------------------------------------
func b8 R_VK_DrawFrame()
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
    R_VK_BeginRenderPass(&cmd_buffer, &r_vk_state.render_pass, r_vk_state.window_resources.framebuffers[image_index]);
    {
      vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, r_vk_state.sphere_pipeline.pipeline);

      // --AlNov: Draw Meshes
      for (R_Mesh* mesh_to_draw = r_vk_state.mesh_list.first; mesh_to_draw; mesh_to_draw = mesh_to_draw->next)
      {
        R_VK_PushMeshToBuffer(mesh_to_draw);

        VkDescriptorSetAllocateInfo set_info = {};
        set_info.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        set_info.descriptorPool     = r_vk_state.descriptor_pool.pool;
        set_info.descriptorSetCount = 1;
        set_info.pSetLayouts        = &r_vk_state.mvp_layout;

        VK_CHECK(vkAllocateDescriptorSets(r_vk_state.device.logical, &set_info, &mesh_to_draw->mvp_set));

        VkDescriptorBufferInfo buffer_info = {};
        buffer_info.buffer = r_vk_state.big_buffer.buffer;
        buffer_info.offset = mesh_to_draw->mvp_offset;
        buffer_info.range  = sizeof(R_VK_MVP);

        VkDescriptorImageInfo image_info = {};
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info.imageView   = r_vk_state.texture.vk_view;
        image_info.sampler     = r_vk_state.sampler;

        VkWriteDescriptorSet buffer_write_set = {};
        buffer_write_set.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        buffer_write_set.dstSet          = mesh_to_draw->mvp_set;
        buffer_write_set.dstBinding      = 0;
        buffer_write_set.dstArrayElement = 0;
        buffer_write_set.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        buffer_write_set.descriptorCount = 1;
        buffer_write_set.pBufferInfo     = &buffer_info;
        
        VkWriteDescriptorSet image_write_set = {};
        image_write_set.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        image_write_set.dstSet          = mesh_to_draw->mvp_set;
        image_write_set.dstBinding      = 1;
        image_write_set.dstArrayElement = 0;
        image_write_set.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        image_write_set.descriptorCount = 1;
        image_write_set.pImageInfo      = &image_info;

        VkWriteDescriptorSet write_sets[2] = { buffer_write_set, image_write_set };

        vkUpdateDescriptorSets(r_vk_state.device.logical, 2, write_sets, 0, 0);

        vkCmdBindDescriptorSets(
          cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, r_vk_state.sphere_pipeline.layout,
          0, 1, &mesh_to_draw->mvp_set, 0, 0
        );

        vkCmdBindVertexBuffers(cmd_buffer, 0, 1, &r_vk_state.big_buffer.buffer, &mesh_to_draw->vertex_offset);
        vkCmdBindIndexBuffer(cmd_buffer, r_vk_state.big_buffer.buffer, mesh_to_draw->index_offset, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(cmd_buffer, mesh_to_draw->index_count, 1, 0, 0, 0);
      }

      // --AlNov: Draw Lines
      /*
      vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, r_vk_state.line_pipeline.pipeline);
      for (R_Line* line = r_vk_state.line_list.first; line; line = line->next)
      {
        VkDeviceSize vertex_buffer_offsets[] = { r_vk_state.vertex_buffer.current_position };
        R_VK_PushVertexBuffer(&r_vk_state.vertex_buffer, line->vertecies, 2 * sizeof(R_LineVertex));

        vkCmdBindVertexBuffers(cmd_buffer, 0, 1, &r_vk_state.vertex_buffer.buffer, vertex_buffer_offsets);

        vkCmdDraw(cmd_buffer, 2, 1, 0, 0);
      }
      */
    }
    R_VK_EndRenderPass(&cmd_buffer, &r_vk_state.render_pass);
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

  return true;
}

func b8 R_VK_EndFrame()
{
  r_vk_state.big_buffer.current_position = 0;

  for (R_Mesh* mesh_to_draw = r_vk_state.mesh_list.first; mesh_to_draw; mesh_to_draw = mesh_to_draw->next)
  {
    // --AlNov: @NOTE It is bad to recreate and delete.
    // But it is how it is now
    vkDeviceWaitIdle(r_vk_state.device.logical);

    vkResetDescriptorPool(r_vk_state.device.logical, r_vk_state.descriptor_pool.pool, 0);
  }

  r_vk_state.mesh_list = {};
  r_vk_state.line_list = {};

  return true;
}

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

func void R_VK_PushMeshToBuffer(R_Mesh* mesh)
{
  // --AlNov: Add Vertecies information
  mesh->vertex_offset = r_vk_state.big_buffer.current_position;
  memcpy((u8*)r_vk_state.big_buffer.mapped_memory + r_vk_state.big_buffer.current_position, mesh->vertecies, mesh->vertex_count * sizeof(R_MeshVertex));
  r_vk_state.big_buffer.current_position += mesh->vertex_count * sizeof(R_MeshVertex);

  // --AlNov: Add Indecies information
  mesh->index_offset = r_vk_state.big_buffer.current_position;
  memcpy((u8*)r_vk_state.big_buffer.mapped_memory + r_vk_state.big_buffer.current_position, mesh->indecies, mesh->index_count * sizeof(u32));
  r_vk_state.big_buffer.current_position += mesh->index_count * sizeof(u32);


  // --AlNov: Add Uniform information
  // --AlNov: (https://vulkan.lunarg.com/doc/view/1.3.268.0/windows/1.3-extensions/vkspec.html#VUID-VkWriteDescriptorSet-descriptorType-00327
  u64 alligment = 64 - (r_vk_state.big_buffer.current_position % 64);
  r_vk_state.big_buffer.current_position += alligment;
  mesh->mvp_offset = r_vk_state.big_buffer.current_position;
  memcpy((u8*)r_vk_state.big_buffer.mapped_memory + r_vk_state.big_buffer.current_position, &mesh->mvp, sizeof(R_VK_MVP));
  r_vk_state.big_buffer.current_position += sizeof(R_VK_MVP);
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

func VkCommandBuffer R_VK_BeginSingleCommands()
{
  VkCommandBufferAllocateInfo allocate_info = {};
  allocate_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocate_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocate_info.commandPool        = r_vk_state.cmd_pool.pool;
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

  vkFreeCommandBuffers(r_vk_state.device.logical, r_vk_state.cmd_pool.pool, 1, &command_buffer);
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

func void R_VK_TransitImageLayout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout)
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
    image_barrier.subresourceRange.layerCount     = 1;

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

func R_Texture R_VK_CreateTexture(const char* path)
{
  R_Texture texture = {};

  i32 tex_width;
  i32 tex_height;
  i32 tex_channels;

  stbi_uc* tex_pixels = stbi_load(path, &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);

  texture.size = tex_width * tex_height * 4;

  if (!tex_pixels)
  {
    LOG_ERROR("Cannot load texture %s\n", path);
    return texture;
  }

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

  R_VK_TransitImageLayout(texture.vk_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  {
    R_VK_CopyBufferToImage(r_vk_state.staging_buffer.buffer, texture.vk_image, MakeVec2u(tex_width, tex_height));
  }
  R_VK_TransitImageLayout(texture.vk_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

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
