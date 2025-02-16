#include "base/base_core.h"
#include "base/base_include.h"
#include "os/os_include.h"
#include "render/r_include.h"

#include "base/base_include.cpp"
#include "os/os_include.cpp"
#include "render/r_include.cpp"

#include "render/vulkan/r_vk_core.h"
#include "third_party/vulkan/include/vulkan.h"
#include "third_party/vulkan/include/vulkan_core.h"
#include "third_party/vulkan/include/vulkan_win32.h"

#pragma comment(lib, "third_party/vulkan/lib/vulkan-1.lib")

#define VK_CHECK(expression) Assert(expression != VK_SUCCESS);

#if 0 // In Render Layer that used to compile shaders
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
#endif

func void InitVulkan();
func void DestroyVulkan();

func void CreateInstance();
func void DestroyInstance();

struct Device
{
  VkDevice logical;
  VkPhysicalDevice physical;

  U32 graphics_queue_index;
  VkQueue graphics_queue;
};

func void CreateDevice();
func void DestroyDevice();

func void CreateSurface(OS_Window* window);
func void DestroySurface();

#define FRAMES_IN_FLIGHT 3
struct FrameResources
{
  VkFence submit_fence;
  VkCommandPool cmd_pool;
  VkCommandBuffer cmd_buffer;
  VkSemaphore acquire_semaphore;
  VkSemaphore release_semaphore;
};

func void CreateFrameResources(FrameResources* resources);
func void DestroyFrameResources(FrameResources* resources);

struct Swapchain
{
  VkSwapchainKHR handle;
  VkSurfaceFormatKHR surface_format;
  Vec2u size;
  Arena* image_arena;
  U32 image_count;
  VkImage* images;
  VkImageView* image_views;
  FrameResources* frame_resources; // Per Image
  U32 current_index;
};

func void CreateSwapchain();
func void RecreateSwapchain();
func void DestroySwapchain();
func B32 AcquireNextImage(U32 *image_index);

struct Pipeline
{
  VkPipeline handle;
  VkPipelineLayout layout;
};

func void CreatePipeline();
func void DestroyPipeline();

struct Buffer
{
  VkBuffer handle;
  U32 size;
  VkDeviceMemory memory;
};

func void CreateVertexBuffer();
func void DestroyVertexBuffer();

struct Vertex
{
  Vec2f position;
  Vec3f color;
};

struct AppState
{
  Arena* arena;
  OS_Window window;

  VkDebugUtilsMessengerEXT debug_msg;
  VkInstance instance;
  Device device;
  VkSurfaceKHR surface;
  Swapchain swapchain;
  Pipeline pipeline;
  R_Shader vertex_shader;
  R_Shader fragment_shader;
  Buffer vertex_buffer;

  Vertex vertecies[3];     

  B32 is_window_closed;
} app_state;

func void HandleEvents(AppState* state);
func void RenderTriangle(U32 image_index);
func B32 PresentImage(U32 image_index);
func void TransitImageLayout(VkCommandBuffer cmd, VkImage image, VkImageLayout layout,
                             VkImageLayout new_layout, VkAccessFlags srs_access,
                             VkAccessFlags dst_access, VkPipelineStageFlags src_stage,
                             VkPipelineStageFlags dst_stage);

I32 main()
{
  app_state = {};
  app_state.arena = AllocateArena(Megabytes(64));
  app_state.is_window_closed = false;
 
  app_state.window = OS_CreateWindow("Vulkan Triangle", MakeVec2u(1270, 720));
  OS_ShowWindow(&app_state.window);
  
  InitVulkan();

  while (!app_state.is_window_closed)
  {
    HandleEvents(&app_state);

    U32 index = 0;
    if (!AcquireNextImage(&index))
    {
      RecreateSwapchain();
    }
    
    RenderTriangle(index);
    if (!PresentImage(index))
    {
      RecreateSwapchain();
    }
  }

  DestroyVulkan();
  
  return 0;
}

func void
HandleEvents(AppState* state)
{
  OS_EventList event_list = OS_GetEventList(app_state.arena);
  
  for (OS_Event *event = event_list.first; event; event = event->next)
  {
    switch (event->type)
    {
      case OS_EVENT_TYPE_EXIT:
      {
        state->is_window_closed = true;
      } break;
      
      default: break;
    }
  }
}

func void
InitVulkan()
{
  CreateInstance();
  CreateDevice();
  CreateSurface(&app_state.window);
  CreateSwapchain();
  CreatePipeline();

  app_state.vertecies[0].position = {{ 0.5f, -0.5f }};
  app_state.vertecies[0].color = {{ 1.0f, 0.0f, 0.0f }};
  app_state.vertecies[1].position = {{ 0.5f, 0.5f }};
  app_state.vertecies[1].color = {{ 1.0f, 1.0f, 0.0f }};
  app_state.vertecies[2].position = {{ -0.5f, 0.5f }};
  app_state.vertecies[2].color = {{ 0.0f, 0.0f, 1.0f }};
  CreateVertexBuffer();
}

func void
DestroyVulkan()
{
  vkDeviceWaitIdle(app_state.device.logical);
  
  DestroyVertexBuffer();
  DestroyPipeline();
  DestroySwapchain();
  DestroySurface();
  DestroyDevice();
  DestroyInstance();
}

func void
CreateInstance()
{
  LOG_INFO("Create Instance\n")
  
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

  VK_CHECK(vkCreateInstance(&instance_info, 0, &app_state.instance));

  VK_CHECK(createDebugMessenger(app_state.instance, &app_state.debug_msg));
}

func void
DestroyInstance()
{
  destroyDebugUtilsMessengerEXT(app_state.instance, app_state.debug_msg, 0);
  
  vkDestroyInstance(app_state.instance, 0);
}

func void
CreateDevice()
{
  U32 device_count = 0;
  VK_CHECK(vkEnumeratePhysicalDevices(app_state.instance, &device_count, 0));
  
  Arena* tmp_arena = AllocateArena(Kilobytes(16));
  {
    VkPhysicalDevice* devices = (VkPhysicalDevice*)PushArena(tmp_arena, device_count * sizeof(VkPhysicalDevice));
    VK_CHECK(vkEnumeratePhysicalDevices(app_state.instance, &device_count, devices));

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
          app_state.device.graphics_queue_index = i;
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
      queue_info.queueFamilyIndex = app_state.device.graphics_queue_index;
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

      app_state.device.physical = *device;
      VK_CHECK(vkCreateDevice(*device, &device_info, 0, &app_state.device.logical))

      vkGetDeviceQueue(app_state.device.logical, app_state.device.graphics_queue_index, 0, &app_state.device.graphics_queue);

      LOG_INFO("%s\n", properties.deviceName);
    }
  }
  FreeArena(tmp_arena);
}

func void
DestroyDevice()
{
  vkDestroyDevice(app_state.device.logical, 0);
}

func void
CreateSurface(OS_Window* window)
{
  VkWin32SurfaceCreateInfoKHR surface_info = {};
  surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  surface_info.hinstance = window->instance;
  surface_info.hwnd = window->handle;

  VK_CHECK(vkCreateWin32SurfaceKHR(app_state.instance, &surface_info, 0, &app_state.surface));
}

func void
DestroySurface()
{
  vkDestroySurfaceKHR(app_state.instance, app_state.surface, 0);
}

func void
CreateFrameResources(FrameResources* resources)
{
  VkFenceCreateInfo fence_info = {
    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    .flags = VK_FENCE_CREATE_SIGNALED_BIT
  };
  VK_CHECK(vkCreateFence(app_state.device.logical, &fence_info, 0, &resources->submit_fence));

  VkCommandPoolCreateInfo cmd_pool_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
    .queueFamilyIndex = app_state.device.graphics_queue_index
  };
  VK_CHECK(vkCreateCommandPool(app_state.device.logical, &cmd_pool_info, 0, &resources->cmd_pool));

  VkCommandBufferAllocateInfo cmd_buffer = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool = resources->cmd_pool,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = 1
  };
  VK_CHECK(vkAllocateCommandBuffers(app_state.device.logical, &cmd_buffer, &resources->cmd_buffer));

  VkSemaphoreCreateInfo semaphore_info = {
    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
  };
  VK_CHECK(vkCreateSemaphore(app_state.device.logical, &semaphore_info, 0, &resources->acquire_semaphore));
  VK_CHECK(vkCreateSemaphore(app_state.device.logical, &semaphore_info, 0, &resources->release_semaphore));
}

func void
DestroyFrameResources(FrameResources* resources)
{
  vkDestroySemaphore(app_state.device.logical, resources->release_semaphore, 0);
  vkDestroySemaphore(app_state.device.logical, resources->acquire_semaphore, 0);
  vkDestroyCommandPool(app_state.device.logical, resources->cmd_pool, 0);
  vkDestroyFence(app_state.device.logical, resources->submit_fence, 0);
}

func void
CreateSwapchain()
{
  Swapchain swapchain = {};
  
  Arena* tmp_arena = AllocateArena(Kilobytes(64));
  {
    U32 format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(app_state.device.physical, app_state.surface, &format_count, 0);
    VkSurfaceFormatKHR* formats = (VkSurfaceFormatKHR*)PushArena(tmp_arena, format_count * sizeof(VkSurfaceFormatKHR));
    vkGetPhysicalDeviceSurfaceFormatsKHR(app_state.device.physical, app_state.surface, &format_count, formats);

    for (U32 i = 0; i < format_count; i += 1)
    {
      if (formats[i].format == VK_FORMAT_B8G8R8A8_UNORM && formats[i].colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
        swapchain.surface_format = formats[i];
      }
    }
  }
  FreeArena(tmp_arena);
  
  VkSurfaceCapabilitiesKHR capabilities;
  VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(app_state.device.physical,
                                                     app_state.surface,
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
    .surface = app_state.surface,
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

  VK_CHECK(vkCreateSwapchainKHR(app_state.device.logical, &swapchain_info, 0, &swapchain.handle));

  VK_CHECK(vkGetSwapchainImagesKHR(app_state.device.logical, swapchain.handle, &swapchain.image_count, 0));
  swapchain.image_arena = AllocateArena(Megabytes(8));
  swapchain.images = (VkImage*)PushArena(swapchain.image_arena, swapchain.image_count * sizeof(VkImage));
  VK_CHECK(vkGetSwapchainImagesKHR(app_state.device.logical, swapchain.handle, &swapchain.image_count, swapchain.images));

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

    VK_CHECK(vkCreateImageView(app_state.device.logical, &view_info, 0, &swapchain.image_views[i]));
    
    CreateFrameResources(&swapchain.frame_resources[i]);
  }
  
  app_state.swapchain = swapchain;
}

func void
RecreateSwapchain()
{
  LOG_INFO("Recreate Swapchain\n");
  DestroySwapchain();
  CreateSwapchain();
}

func void
DestroySwapchain()
{
  vkDeviceWaitIdle(app_state.device.logical);
  
  for (I32 i = 0; i < app_state.swapchain.image_count; i += 1)
  {
    DestroyFrameResources(&app_state.swapchain.frame_resources[i]);
    vkDestroyImageView(app_state.device.logical, app_state.swapchain.image_views[i], 0);
  }

  vkDestroySwapchainKHR(app_state.device.logical, app_state.swapchain.handle, 0);

  FreeArena(app_state.swapchain.image_arena);
}

func B32
AcquireNextImage(U32* image_index)
{
  app_state.swapchain.current_index += 1;
  app_state.swapchain.current_index %= app_state.swapchain.image_count;
  U32 current_index = app_state.swapchain.current_index;

  VkResult acquire_result = vkAcquireNextImageKHR(
    app_state.device.logical, app_state.swapchain.handle, U64_MAX,
    app_state.swapchain.frame_resources[current_index].acquire_semaphore, 0,
    image_index
  );
  
  if (acquire_result != VK_SUCCESS) {
    return false;
  }

  vkWaitForFences(app_state.device.logical, 1, &app_state.swapchain.frame_resources[*image_index].submit_fence, true, U64_MAX);
  vkResetFences(app_state.device.logical, 1, &app_state.swapchain.frame_resources[*image_index].submit_fence);

  vkResetCommandPool(app_state.device.logical, app_state.swapchain.frame_resources[*image_index].cmd_pool, 0);

  return true;
}

func void
CreatePipeline()
{
  VkPipelineLayoutCreateInfo layout_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
  };
  VK_CHECK(vkCreatePipelineLayout(app_state.device.logical, &layout_info, 0, &app_state.pipeline.layout));

  VkVertexInputBindingDescription binding_description = {
    .binding = 0,
    .stride = sizeof(Vertex),
    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
  };

  VkVertexInputAttributeDescription attribute_descriptions[2];
  attribute_descriptions[0] = {
    .location = 0,
    .binding = 0,
    .format = VK_FORMAT_R32G32_SFLOAT,
    .offset = offsetof(Vertex, position)
  };
  attribute_descriptions[1] = {
    .location = 1,
    .binding = 0,
    .format = VK_FORMAT_R32G32B32_SFLOAT,
    .offset = offsetof(Vertex, color)
  };

  VkPipelineVertexInputStateCreateInfo vertex_input = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .vertexBindingDescriptionCount = 1,
    .pVertexBindingDescriptions = &binding_description,
    .vertexAttributeDescriptionCount = CountArrayElements(attribute_descriptions),
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
    .cullMode = VK_CULL_MODE_NONE,
    .frontFace = VK_FRONT_FACE_CLOCKWISE,
    .depthBiasEnable = false,
    .lineWidth = 1.0f
  };

  VkDynamicState dynamic_states[] = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
  };

  VkPipelineColorBlendAttachmentState blend_attachment = {
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
    .depthCompareOp = VK_COMPARE_OP_ALWAYS
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

  R_H_LoadShader(app_state.arena, "data/shaders/triangle.vs.glsl",
                 "main", R_SHADER_TYPE_VERTEX,
                 &app_state.vertex_shader);
  R_H_LoadShader(app_state.arena, "data/shaders/triangle.fs.glsl",
                 "main", R_SHADER_TYPE_FRAGMENT,
                 &app_state.fragment_shader);

  VkShaderModule vertex_module;
  {
    VkShaderModuleCreateInfo module_info = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = app_state.vertex_shader.code_size,
      .pCode = (U32*)app_state.vertex_shader.code
    };
    VK_CHECK(vkCreateShaderModule(app_state.device.logical, &module_info, 0, &vertex_module));
  }
  VkPipelineShaderStageCreateInfo vertex_shader = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .stage = VK_SHADER_STAGE_VERTEX_BIT,
    .module = vertex_module,
    .pName = app_state.vertex_shader.entry_point
  };
  
  VkShaderModule fragment_module;
  {
    VkShaderModuleCreateInfo module_info = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = app_state.fragment_shader.code_size,
      .pCode = (U32*)app_state.fragment_shader.code
    };
    VK_CHECK(vkCreateShaderModule(app_state.device.logical, &module_info, 0, &fragment_module));
  }
  VkPipelineShaderStageCreateInfo fragment_shader = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
    .module = fragment_module,
    .pName = app_state.fragment_shader.entry_point
  };

  VkPipelineShaderStageCreateInfo shaders[] = {
    vertex_shader,
    fragment_shader
  };

  VkPipelineRenderingCreateInfo rendering_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
    .colorAttachmentCount = 1,
    .pColorAttachmentFormats = &app_state.swapchain.surface_format.format
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
    .layout = app_state.pipeline.layout,
    .renderPass = 0,
    .subpass = 0
  };
  VK_CHECK(vkCreateGraphicsPipelines(app_state.device.logical,
                                     0, 1, &pipeline_info, 0,
                                     &app_state.pipeline.handle));

  vkDestroyShaderModule(app_state.device.logical, vertex_module, 0);
  vkDestroyShaderModule(app_state.device.logical, fragment_module, 0);
}

func void
DestroyPipeline()
{
  vkDestroyPipelineLayout(app_state.device.logical, app_state.pipeline.layout, 0);
  vkDestroyPipeline(app_state.device.logical, app_state.pipeline.handle, 0);
}

func void
CreateVertexBuffer()
{
  VkDeviceSize size = sizeof(app_state.vertecies[0]) * CountArrayElements(app_state.vertecies);
  app_state.vertex_buffer.size = size;
  
  VkBufferCreateInfo buffer_info = {
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .size = size,
    .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE
  };

  VK_CHECK(vkCreateBuffer(app_state.device.logical, &buffer_info, 0, &app_state.vertex_buffer.handle));

  VkMemoryRequirements memory_requirements;
  vkGetBufferMemoryRequirements(app_state.device.logical, app_state.vertex_buffer.handle, &memory_requirements);
  
  VkPhysicalDeviceMemoryProperties mem_properties = {};
  vkGetPhysicalDeviceMemoryProperties(app_state.device.physical, &mem_properties);

  VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
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
  VK_CHECK(vkAllocateMemory(app_state.device.logical, &allocation_info , 0, &app_state.vertex_buffer.memory));

  VK_CHECK(vkBindBufferMemory(app_state.device.logical, app_state.vertex_buffer.handle, app_state.vertex_buffer.memory, 0));

  void* data;
  VK_CHECK(vkMapMemory(app_state.device.logical, app_state.vertex_buffer.memory, 0, app_state.vertex_buffer.size, 0, &data));
  {
    memcpy(data, app_state.vertecies, size);
  }
  vkUnmapMemory(app_state.device.logical, app_state.vertex_buffer.memory);
}

func void
DestroyVertexBuffer()
{
  vkFreeMemory(app_state.device.logical, app_state.vertex_buffer.memory, 0);
  vkDestroyBuffer(app_state.device.logical, app_state.vertex_buffer.handle, 0);
}

func void
RenderTriangle(U32 image_index)
{
  VkCommandBuffer cmd = app_state.swapchain.frame_resources[image_index].cmd_buffer;

  VkCommandBufferBeginInfo begin_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
  };

  VK_CHECK(vkBeginCommandBuffer(cmd, &begin_info))
  {
    TransitImageLayout(
      cmd,
      app_state.swapchain.images[image_index],
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      0,
      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    );

    VkClearValue clear_value = {
      .color = { {0.01f, 0.01f, 0.033f, 1.0f} }
    };

    VkRenderingAttachmentInfo color_attachment = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .imageView = app_state.swapchain.image_views[image_index],
      .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .clearValue = clear_value
    };

    VkExtent2D render_area = {
      .width = app_state.swapchain.size.width,
      .height = app_state.swapchain.size.height
    };
    VkRenderingInfo rendering_info = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
      .renderArea = {
        .offset = { 0, 0 },
        .extent = render_area
      },
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &color_attachment
    };

    vkCmdBeginRendering(cmd, &rendering_info);
    {
      vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, app_state.pipeline.handle);
      
      VkViewport viewport = {
        .width = (F32)render_area.width,
        .height = (F32)render_area.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
      };
      vkCmdSetViewport(cmd, 0, 1, &viewport);

      VkRect2D scissor = {
        .extent = {
          .width = render_area.width,
          .height = render_area.height
        }
      };
      vkCmdSetScissor(cmd, 0, 1, &scissor);

      VkDeviceSize offset = {};
      vkCmdBindVertexBuffers(cmd, 0, 1, &app_state.vertex_buffer.handle, &offset);

      vkCmdDraw(cmd, CountArrayElements(app_state.vertecies), 1, 0, 0);
    }
    vkCmdEndRendering(cmd);

    TransitImageLayout(
      cmd,
      app_state.swapchain.images[image_index],
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      0,
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT
    );
  }
  VK_CHECK(vkEndCommandBuffer(cmd));

  VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

  VkSubmitInfo submit_info = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = &app_state.swapchain.frame_resources[app_state.swapchain.current_index].acquire_semaphore,
    .pWaitDstStageMask = &wait_stage,
    .commandBufferCount = 1,
    .pCommandBuffers = &cmd,
    .signalSemaphoreCount = 1,
    .pSignalSemaphores = &app_state.swapchain.frame_resources[image_index].release_semaphore
  };

  VK_CHECK(vkQueueSubmit(app_state.device.graphics_queue, 1, &submit_info, app_state.swapchain.frame_resources[image_index].submit_fence));
}

func B32
PresentImage(U32 image_index)
{
  VkPresentInfoKHR present_info = {
    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = &app_state.swapchain.frame_resources[image_index].release_semaphore,
    .swapchainCount = 1,
    .pSwapchains = &app_state.swapchain.handle,
    .pImageIndices = &image_index
  };

  VkResult present_result = vkQueuePresentKHR(app_state.device.graphics_queue, &present_info);

  if (present_result != VK_SUCCESS)
  {
    return false;
  }

  return true;
}

func void
TransitImageLayout(VkCommandBuffer cmd, VkImage image, VkImageLayout layout,
                   VkImageLayout new_layout, VkAccessFlags srs_access,
                   VkAccessFlags dst_access, VkPipelineStageFlags src_stage,
                   VkPipelineStageFlags dst_stage)
{
  VkImageMemoryBarrier image_barrier = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
    .oldLayout = layout,
    .newLayout = new_layout,
    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .image = image,
    .subresourceRange = {
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .baseMipLevel = 0,
      .levelCount = 1,
      .baseArrayLayer = 0,
      .layerCount = 1
    }
  };

  vkCmdPipelineBarrier(cmd, src_stage, dst_stage, 0, 0, 0, 0, 0, 1, &image_barrier);
}
