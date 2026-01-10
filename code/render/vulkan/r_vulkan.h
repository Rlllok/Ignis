#pragma once

#include "base/base_include.h"

#include "vulkan/vulkan.h"
#if IGNIS_PLATFORM_WIN32
#include "third_party/vulkan/include/vulkan_win32.h"
#endif // IGNIS_PLATFORM_WIN32
#if IGNIS_PLATFORM_LINUX_X11
#include <X11/Xlib.h>
#include "third_party/vulkan/include/vulkan_xlib.h"
#endif // IGNIS_PLATFORM_LINUX_X11
#if IGNIS_PLATFORM_LINUX_WAYLAND
#include "vulkan/vulkan_wayland.h"
#endif // IGNIS_PLATFORM_LINUX

#define VK_CHECK(expression) Assert((expression) == VK_SUCCESS)

#include "r_vk_utils.h"

// --------------------------------------------------
// Buffer
typedef struct R_VK_Buffer R_VK_Buffer;
struct R_VK_Buffer
{
  VkBuffer handle;
  VkDeviceMemory memory;
	void* mapped;
  U64 size;
  U64 capacity;
};
R_VK_Buffer R_VK_BufferDefaultValue = {0};
DefineArray(R_VK_Buffer, R_VK_BufferArray, R_VK_BufferDefaultValue)

func R_VK_Buffer* R_VK_BufferFromHandle(R_Buffer handle);
func R_Buffer R_VK_CreateBuffer(U32 capacity, R_BufferUsageFlags usage_flags, R_BufferPropertyFlags property_flags);
func void R_VK_DestroyBuffer(R_Buffer buffer);
func U64 R_VK_PushBuffer(R_Buffer buffer, U8* data, U64 size);
func void R_VK_ResetBuffer(R_Buffer buffer);
func void R_VK_BindIndexBuffer(R_CommandBuffer command_buffer, R_Buffer buffer, U64 offset, R_IndexSize index_size);
func void R_VK_BindVertexBuffer(R_CommandBuffer command_buffer, R_Buffer buffer, U64 offset);

func void R_VK_BufferGetData(R_Buffer buffer, U64 offset, void* dst, U64 data_size);

// --------------------------------------------------
// Device
typedef struct R_VK_Device R_VK_Device;
struct R_VK_Device
{
  VkDevice logical;
  VkPhysicalDevice physical;

  U32 graphics_queue_index;
  VkQueue graphics_queue;
};

func void R_VK_CreateDevice(void);
func void R_VK_DestroyDevice(void);

// --------------------------------------------------
// Surface/Swapchain
typedef struct FrameResources FrameResources;
struct FrameResources // @TODO Remove
{
  VkFence submit_fence;
  VkCommandPool cmd_pool;
  VkCommandBuffer cmd_buffer;
  VkSemaphore acquire_semaphore;
  VkSemaphore release_semaphore;
};

func FrameResources R_VK_CreateFrameResources(void);
func void R_VK_DestoryFrameResources(FrameResources* resources);

typedef struct R_VK_Swapchain R_VK_Swapchain;
struct R_VK_Swapchain
{
	OS_Window* window;

  Arena* image_arena;

  VkSwapchainKHR handle;
  VkSurfaceKHR surface;
  VkSurfaceFormatKHR surface_format;
  Vec2U32 size;
  U32 image_count;
  R_Texture* textures;
  FrameResources* frame_resources; // @TODO It is per Image for now
};

func void R_VK_CreateSwapchain(OS_Window* window);
func void R_VK_DestroySwapchain(void);
func void R_VK_RecreateSwapchain(OS_Window* window);
func B32 R_VK_SwapchainAcquireNextImage(U32 *image_index);

func R_TextureFormat R_VK_GetSwapchainTextureFormat();
func R_Texture R_VK_AcquireSwapchainTexture(R_CommandBuffer command_buffer);

// -------------------------------------------------------------------
// Render Pass
func R_RenderPass* R_VK_BeginRenderPass(R_CommandBuffer command_buffer, U32 color_targets_count, R_ColorTarget* color_targets, R_DepthStencilTarget* depth_stencil_target);
func void R_VK_EndRenderPass(R_CommandBuffer command_buffer, R_RenderPass* render_pass);

// -------------------------------------------------------------------
// Descriptor Sets
#define R_VK_MAX_POOL_COUNT 64
#define R_VK_SETS_PER_POOL 128
#define R_VK_MAX_UNIFORM_BUFFERS_PER_SET 4
#define R_VK_MAX_SAMPLERS_PER_SET 4

#define R_VK_VERTEX_SHADER_GLOBAL_UNIFORM_SET_SLOT 0
#define R_VK_VERTEX_SHADER_INSTANCE_UNIFORM_SET_SLOT 1
#define R_VK_FRAGMENT_SHADER_GLOBAL_UNIFORM_SET_SLOT 2
#define R_VK_FRAGMENT_SHADER_INSTANCE_UNIFORM_SET_SLOT 3

typedef struct R_VK_DescriptorPool R_VK_DescriptorPool;
struct R_VK_DescriptorPool
{
	VkDescriptorPool vk_pools[R_VK_MAX_POOL_COUNT];
	VkDescriptorSet vk_sets[R_VK_MAX_POOL_COUNT][R_VK_SETS_PER_POOL];
	I32 pool_count;
	I32 sets_count;
};

func void R_VK_BindGlobalShaderData(R_CommandBuffer command_buffer, R_ShaderType shader_type, I32 uniform_buffers_count, R_UniformBufferBindingInfo* uniform_info, I32 sampler_counts, R_SamplerBindingInfo* sampler_infos);
func void R_VK_BindInstanceShaderData(R_CommandBuffer command_buffer, R_ShaderType shader_type, I32 uniform_buffers_count, R_UniformBufferBindingInfo* uniform_info, I32 sampler_counts, R_SamplerBindingInfo* sampler_infos);

func void R_VK_BindGlobalVertexShaderData(R_CommandBuffer command_buffer, R_UniformBufferBindingInfo uniform_info);
func void R_VK_BindInstanceVertexShaderData(R_CommandBuffer command_buffer, R_UniformBufferBindingInfo uniform_info);
func void R_VK_BindGlobalFragmentShaderData(R_CommandBuffer command_buffer, R_UniformBufferBindingInfo uniform_info);
func void R_VK_BindInstanceFragmentShaderData(R_CommandBuffer command_buffer, R_UniformBufferBindingInfo uniform_info);

// --------------------------------------------------
// Pipeline
#define R_VK_MAX_OBJECTS 1024

typedef struct R_VK_GraphicsPipeline R_VK_GraphicsPipeline;
struct R_VK_GraphicsPipeline
{
  VkPipeline handle;
  VkPipelineLayout layout;
	U32 vertex_global_uniforms_count;
  U32 vertex_global_samplers_count;
  U32 vertex_instance_uniforms_count;
  U32 vertex_instance_samplers_count;
	U32 fragment_global_uniforms_count;
	U32 fragment_global_samplers_count;
  U32 fragment_instance_uniforms_count;
  U32 fragment_instance_samplers_count;
	VkDescriptorSetLayout vertex_global_set_layout;
	VkDescriptorSetLayout vertex_instance_set_layout;
	VkDescriptorSetLayout fragment_global_set_layout;
	VkDescriptorSetLayout fragment_instance_set_layout;
};
R_VK_GraphicsPipeline R_VK_GraphicsPipelineDefaultValue = {0};
DefineArray(R_VK_GraphicsPipeline, R_VK_GraphicsPipelineArray, R_VK_GraphicsPipelineDefaultValue)

func R_VK_GraphicsPipeline* R_VK_GraphicsPipelineFromHandle(R_GraphicsPipeline pipeline);
func R_GraphicsPipeline R_VK_CreateGraphicsPipeline(R_GraphicsPipelineCreateInfo* info);
func void R_VK_DestroyGraphicsPipeline(R_GraphicsPipeline pipeline);
func void R_VK_BindGraphicsPipeline(R_CommandBuffer command_buffer, R_GraphicsPipeline pipeline);

// -------------------------------------------------------------------
// Draw
func void R_VK_SetViewport(R_CommandBuffer command_buffer, RectI32 viewport);
func void R_VK_SetScissor(R_CommandBuffer command_buffer, RectI32 scissor);

func void R_VK_DrawPrimitives(R_CommandBuffer command_buffer, U32 vertex_count, U32 instance_count, U32 first_vertex, U32 first_instance);
func void R_VK_DrawIndexedPrimitives(R_CommandBuffer command_buffer, U32 index_count, U32 instance_count, U32 first_index, I32 vertex_offset, U32 first_instance);
func void R_VK_PresentTexture(R_CommandBuffer command_buffer, R_Texture texture);

// -------------------------------------------------------------------
// Texture
typedef struct R_VK_Texture R_VK_Texture;
struct R_VK_Texture
{
  R_TextureFormat format;

	VkImage image;
	VkImageView view;
	VkDeviceMemory memory;
  VkImageLayout layout;
  VkImageAspectFlagBits aspect_mask;
  Vec2I32 size;
  B32 from_swapchain; // --AlNov: If image created by swapchain, it has to be destroyed by swapchain
};
R_VK_Texture R_VK_TextureDefaultValue = {0};
DefineArray(R_VK_Texture, R_VK_TextureArray, R_VK_TextureDefaultValue)
U32 R_VK_TextureFreeListDefaultValue = R_NIL;
DefineArray(U32, R_VK_TextureFreeList, R_VK_TextureFreeListDefaultValue)

func R_VK_Texture* R_VK_TextureFromHandle(R_Texture handle);
func R_Texture R_VK_CreateTexture(R_TextureCreateInfo* info);
func B32 R_VK_DestroyTexture(R_Texture texture);
func void R_VK_LoadDataToTexture(U8* data, U64 data_size, R_Texture texture);
func void R_VK_CopyTexture(R_CommandBuffer command_buffer, R_Texture source, R_Texture destination);
func U64 R_VK_CopyTextureToBuffer(R_CommandBuffer command_buffer, R_Texture texture, R_Buffer buffer);
func void R_VK_CopyBufferToTexture(R_CommandBuffer command_buffer, R_Buffer buffer, U64 offset, U64 size, R_Texture texture);
func R_TextureFormat R_VK_GetTextureFormat(R_Texture texture);
func Vec2I32         R_VK_GetTextureDimension(R_Texture texture);

func void R_VK_ChangeTextureLayout(VkCommandBuffer cmd, R_VK_Texture* texture, VkImageLayout new_layout);

typedef struct R_VK_TextureSampler R_VK_TextureSampler;
struct R_VK_TextureSampler
{
  VkSampler handle;
};
R_VK_TextureSampler R_VK_TextureSamplerDefaultValue = {0};
DefineArray(R_VK_TextureSampler, R_VK_TextureSamplerArray, R_VK_TextureSamplerDefaultValue)

func R_VK_TextureSampler* R_VK_TextureSamplerFromHandle(R_TextureSampler sampler);
func R_TextureSampler R_VK_CreateTextureSampler(R_TextureSamplerCreateInfo* info);

// -------------------------------------------------------------------
// Command Buffer
typedef struct R_VK_CommandBuffer R_VK_CommandBuffer;
struct R_VK_CommandBuffer
{
	VkCommandBuffer handle[R_FRAMES_IN_FLIGHT];
	VkFence submit_fence[R_FRAMES_IN_FLIGHT];
	VkSemaphore acquire_semaphore[R_FRAMES_IN_FLIGHT];
	VkSemaphore release_semaphore[R_FRAMES_IN_FLIGHT];

	R_VK_DescriptorPool descriptor_pool[R_FRAMES_IN_FLIGHT];

	R_VK_GraphicsPipeline* binded_graphics_pipeline;
  VkViewport current_viewport;
};
R_VK_CommandBuffer R_VK_CommandBufferDefaultValue = {0};
DefineArray(R_VK_CommandBuffer, R_VK_CommandBufferArray, R_VK_CommandBufferDefaultValue)

func R_VK_CommandBuffer* R_VK_CommandBufferFromHandle(R_CommandBuffer command_buffer);
func R_CommandBuffer R_VK_GetCommandBuffer(void);
func void R_VK_ReleaseCommandBuffer(R_CommandBuffer command_buffer);
func void R_VK_BeginCommandBuffer(R_CommandBuffer command_buffer);
func void R_VK_SubmitCommandBuffer(R_CommandBuffer command_buffer);

func VkCommandBuffer R_VK_BeginSingleCmd(void);
func void R_VK_EndSingleCmd(VkCommandBuffer cmd);

// --------------------------------------------------
// Global State
typedef struct R_VK_State R_VK_State;
struct R_VK_State
{
  Arena* arena;

  VkInstance instance;
  R_VK_Device device;
  R_VK_Swapchain swapchain;
	VkCommandPool command_pool;

#if IGNIS_DEBUG
  VkDebugUtilsMessengerEXT debug_messenger;
#endif // IGNIS_DEBUG
	
  R_VK_BufferArray buffers;
  R_VK_GraphicsPipelineArray graphics_pipelines;
  R_VK_CommandBufferArray command_buffers;
  R_VK_TextureSamplerArray samplers;
  R_VK_TextureArray textures;
  R_VK_TextureFreeList textures_free_list;

  U32 current_frame;
  U32 current_target;
} _r_vk_state;

func B32 R_VK_Init(OS_Window* window);
func B32 R_VK_Shutdown(void);

func void R_VK_HandleResize(OS_Window* window);

// -------------------------------------------------------------------
// Debug Tools
#if IGNIS_DEBUG
VKAPI_ATTR VkBool32 VKAPI_CALL R_VK_DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
func VkDebugUtilsMessengerCreateInfoEXT R_VK_PopulateDebugMessengerCreateInfo(void);
func VkResult R_VK_CreateDebugUtilsMessenger(VkInstance instance, VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMesseneger);
func void R_VK_DestroyDebugUtilsMessenger(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, VkAllocationCallbacks* pAllocator);
func VkResult R_VK_CreateDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT* debugMessenger);
#endif // IGNIS_DEBUG

