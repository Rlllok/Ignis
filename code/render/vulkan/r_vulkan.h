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

func R_VK_Buffer R_VK_CreateBuffer(U64 capacity, BufferUsageFlags usage_flags, BufferPropertyFlags flags);
func void R_VK_DestroyBuffer(R_VK_Buffer* buffer);

// --------------------------------------------------
// Device
struct R_VK_Device
{
  VkDevice logical;
  VkPhysicalDevice physical;

  U32 graphics_queue_index;
  VkQueue graphics_queue;
};

func void R_VK_CreateDevice();
func void R_VK_DestroyDevice();

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

func void R_VK_CreateFrameResources(FrameResources* resources);
func void R_VK_DestoryFrameResources(FrameResources* resources);

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
  FrameResources* frame_resources; // @TODO It is per Image for now
};

// --AlNov: @TODO Remove. Created in CreateSwapchain
func void R_VK_CreateSurface(OS_Window* window);
func void R_VK_DestroySurface();

func void R_VK_CreateSwapchain(OS_Window* window);
func void R_VK_DestroySwapchain();
func void R_VK_RecreateSwapchain(OS_Window* window);
func B32 R_VK_SwapchainAcquireNextImage(U32 *image_index);

// --------------------------------------------------
// Pipeline
#define R_VK_MAX_OBJECTS 1024

// --AlNov: @TODO I feel that Pipeline is a better name, as it was before.
struct R_VK_GraphicsPipeline
{
  VkPipeline handle;
  VkPipelineLayout layout;

  VkDescriptorPool global_set_pool[R_VK_FRAMES_IN_FLIGHT];
  VkDescriptorPool instance_set_pool[R_VK_FRAMES_IN_FLIGHT];
  
  VkDescriptorSetLayout global_set_layout;
  VkDescriptorSetLayout instance_set_layout;

  VkDescriptorSet global_sets[R_VK_FRAMES_IN_FLIGHT];
  VkDescriptorSet instance_sets[R_VK_MAX_OBJECTS]; // AlNov: @TODO Should be FRAMES*OBJECTS

	U32 object_number;
};

func void R_VK_CreateGraphicsPipeline(R_Pipeline* pipeline);
func void R_VK_DestroyGraphicsPipeline();

func void R_VK_BindPipeline(R_Pipeline* pipeline, U8* global_data, U32 global_data_size);

// -------------------------------------------------------------------
// Render Pass
func void R_VK_RenderPassBegin(R_AttachmentLoadOperation load_operation, Vec4f clear_color);
func void R_VK_RenderPassEnd();

// -------------------------------------------------------------------
// Command Buffer
func VkCommandBuffer R_VK_BeginSingleCmd();
func void R_VK_EndSingleCmd(VkCommandBuffer cmd);

// -------------------------------------------------------------------
// Texture
struct R_VK_Texture
{
	VkImage image;
	VkImageView view;
	VkDeviceMemory memory;
};

func R_Texture R_VK_CreateTexture(Str8 path);

// -------------------------------------------------------------------
// Draw
func void R_VK_BeginFrame();
func void R_VK_EndFrame();

func void R_VK_GeometryPrepare(AST_Geometry* geometry);
func B32 R_VK_GeometryDraw(R_DrawGeometryInfo* draw_info); // --AlNov: @TODO Change struct name to R_GeometryDrawInfo

// --------------------------------------------------
// Global State
struct R_VK_State
{
  Arena* arena;

  VkInstance instance;
  R_VK_Device device;
  R_VK_Swapchain swapchain;
	VkCommandPool cmd_pool;

#if IGNIS_DEBUG
  VkDebugUtilsMessengerEXT debug_messenger;
#endif // IGNIS_DEBUG

  R_VK_GraphicsPipeline graphics_pipelines[32];
  U32 graphics_pipelines_count;

  R_VK_Buffer geometry_buffer;
  R_VK_Buffer staging_buffer;

	R_VK_Texture default_texture;
	VkSampler default_sampler;

  PipelineID binded_pipeline_id;
  
  U32 current_frame;
  U32 current_target;
} _r_vk_state;

func B32 R_VK_Init(OS_Window* window);
func B32 R_VK_Shutdown();

func void R_VK_HandleResize(OS_Window* window);

// -------------------------------------------------------------------
// Debug Tools
#if IGNIS_DEBUG
VKAPI_ATTR VkBool32 VKAPI_CALL R_VK_DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
func void R_VK_PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& messengerInfo);
func VkResult R_VK_CreateDebugUtilsMessenger(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMesseneger);
func void R_VK_DestroyDebugUtilsMessenger(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, VkAllocationCallbacks* pAllocator);
func VkResult R_VK_CreateDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT* debugMessenger);
#endif // IGNIS_DEBUG

