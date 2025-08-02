#pragma once

#include "base/base_include.h"

#include "vulkan/vulkan.h"
#if IGNIS_PLATFORM_LINUX
#include "vulkan/vulkan_wayland.h"
#endif // IGNIS_PLATFORM_LINUX
#if IGNIS_PLATFORM_WIN32
#include "third_party/vulkan/include/vulkan_win32.h"
#endif // IGNIS_PLATFORM_WIN32

#define VK_CHECK(expression) Assert(expression != VK_SUCCESS)

#include "r_vk_utils.h"

// --------------------------------------------------
// Buffer
struct R_VK_Buffer
{
  VkBuffer handle;
  VkDeviceMemory memory;
  U64 size;
  U64 capacity;
};

func R_VK_Buffer R_VK_BufferCreate(U64 capacity, BufferUsageFlags usage_flags, BufferPropertyFlags flags);
func void R_VK_BufferDestroy();

// --------------------------------------------------
// Device
struct R_VK_Device
{
  VkDevice logical;
  VkPhysicalDevice physical;

  U32 graphics_queue_index;
  VkQueue graphics_queue;
};

func void R_VK_DeviceCreate();
func void R_VK_DeviceDestroy();

// --------------------------------------------------
// Surface/Swapchain
#define R_VK_FRAMES_IN_FLIGHT 3
struct FrameResources
{
  VkFence submit_fence;
  VkCommandPool cmd_pool;
  VkCommandBuffer cmd_buffer;
  VkSemaphore acquire_semaphore;
  VkSemaphore release_semaphore;
};

func void R_VK_FrameResourcesCreate(FrameResources* resources);
func void R_VK_FrameResourcesDestroy(FrameResources* resources);

struct R_VK_Swapchain
{
  Arena* image_arena;

  VkSwapchainKHR handle;
  VkSurfaceKHR surface;
  VkSurfaceFormatKHR surface_format;
  Vec2u size;
  U32 image_count;
  VkImage* images;
  VkImageView* image_views;
  VkImage depth_image;
  VkDeviceMemory depth_image_memory;
  VkImageView depth_image_view;
  FrameResources frame_resources[R_VK_FRAMES_IN_FLIGHT]; // Per Image
};

func void R_VK_SurfaceCreate(OS_Window* window);
func void R_VK_SurfaceDestroy();

func void R_VK_SwapchainCreate(OS_Window* window);
func void R_VK_SwapchainDestroy();
func void R_VK_SwapchainRecreate(OS_Window* window);
func B32 R_VK_SwapchainAcquireNextImage(U32 *image_index);

// --------------------------------------------------
// Pipeline
#define R_VK_MAX_OBJECTS 1024

struct R_VK_GraphicsShader
{
  VkPipeline pipeline;
  VkPipelineLayout pipeline_layout;

  VkDescriptorPool global_pool[R_VK_FRAMES_IN_FLIGHT];
  VkDescriptorPool instance_set_pool[R_VK_FRAMES_IN_FLIGHT];
  
  VkDescriptorSetLayout global_set_layout;
  VkDescriptorSetLayout instance_set_layout;

  VkDescriptorSet global_sets[R_VK_FRAMES_IN_FLIGHT];
  VkDescriptorSet instance_sets[R_VK_MAX_OBJECTS];
};

func void R_VK_GraphicsShaderCreate(R_Pipeline* pipeline);
func void R_VK_GraphicsShaderDestroy();

// --------------------------------------------------
// Global State
struct R_VK_State
{
  Arena* arena;

  VkInstance instance;
  R_VK_Device device;
  R_VK_Swapchain swapchain;

  R_VK_GraphicsShader graphics_shaders[32];
  U32 graphics_shaders_count;

  U32 current_frame;
} _r_vk_state;

func B32 R_VK_Init(OS_Window* window);
func B32 R_VK_Shutdown();
