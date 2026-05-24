#pragma once

#include "base/base_include.h"
#include "os/os_include.h"
#include "rhi/rhi_core.h"

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

#include "third_party/glslang/include/Include/glslang_c_interface.h"
#include "third_party/glslang/include/Public/resource_limits_c.h"

VkResult VK_CHECK_RESULT = VK_SUCCESS;
#define VK_CHECK(expression) VK_CHECK_RESULT = expression; Assert((VK_CHECK_RESULT) == VK_SUCCESS)

#include "rhi_vk_utils.h"

// -------------------------------------------------------------------
// -- Synchronization ------------------------------------------------
typedef struct RHI_VK_Semaphore RHI_VK_Semaphore;
struct RHI_VK_Semaphore {
  VkSemaphore vk;
  B32         in_use;
};
RHI_VK_Semaphore RHI_VK_SemaphoreNil = ZeroStruct();
DefineArray(RHI_VK_Semaphore, RHI_VK_SemaphoreArray, RHI_VK_SemaphoreNil)

// -------------------------------------------------------------------
// -- Buffer ---------------------------------------------------------
typedef struct RHI_VK_Buffer RHI_VK_Buffer;
struct RHI_VK_Buffer {
  VkBuffer       vk;
  VkDeviceMemory memory;
	void*          mapped;
  U64            size;
  U64            capacity;
};
RHI_VK_Buffer RHI_VK_BufferNil = ZeroStruct();
DefineArray(RHI_VK_Buffer, RHI_VK_BufferArray, RHI_VK_BufferNil)

func RHI_VK_Buffer*    RHI_VK_BufferFromHandle(RHI_Buffer handle);
func RHI_Buffer        RHI_VK_CreateBuffer(Str8 label, U32 capacity, RHI_BufferUsageFlags usage_flags, RHI_BufferPropertyFlags property_flags);
func void              RHI_VK_DestroyBuffer(RHI_Buffer buffer);
func U64               RHI_VK_PushBuffer(RHI_Buffer buffer, U8* data, U64 size);
func void              RHI_VK_ResetBuffer(RHI_Buffer buffer);
func RHI_DeviceAddress RHI_VK_BufferDeviceAddress(RHI_Buffer buffer);
func void              RHI_VK_BindIndexBuffer(RHI_CommandBuffer command_buffer, RHI_Buffer buffer, U64 offset, RHI_IndexSize index_size);
func void              RHI_VK_BindVertexBuffer(RHI_CommandBuffer command_buffer, RHI_Buffer buffer, U64 offset);

func void RHI_VK_BufferGetData(RHI_Buffer buffer, U64 offset, void* dst, U64 data_size);

// -------------------------------------------------------------------
// -- Device ---------------------------------------------------------
typedef struct RHI_VK_Device RHI_VK_Device;
struct RHI_VK_Device {
  VkDevice         logical;
  VkPhysicalDevice physical;
  U32              graphics_queue_index;
  VkQueue          graphics_queue;
};

func void RHI_VK_CreateDevice();
func void RHI_VK_DestroyDevice();

// -------------------------------------------------------------------
// -- Surface/Swapchain ----------------------------------------------
#define RHI_VK_SWAPCHAIN_MAX_IMAGES 8

typedef struct RHI_VK_Swapchain RHI_VK_Swapchain;
struct RHI_VK_Swapchain {
	OS_Window*         window;
  VkSwapchainKHR     vk;
  VkSurfaceKHR       surface;
  VkSurfaceFormatKHR surface_format;
  Vec2U32            size;
  U32                image_count;
  RHI_Texture        textures[RHI_VK_SWAPCHAIN_MAX_IMAGES];
  VkSemaphore        render_finished_semaphores[RHI_VK_SWAPCHAIN_MAX_IMAGES];
};

func void RHI_VK_CreateSwapchain(OS_Window* window);
func void RHI_VK_DestroySwapchain();
func void RHI_VK_RecreateSwapchain(OS_Window* window);
func B32  RHI_VK_SwapchainAcquireNextImage(U32 *image_index);

func RHI_TextureFormat RHI_VK_GetSwapchainTextureFormat();
func RHI_Texture       RHI_VK_AcquireSwapchainTexture(RHI_CommandBuffer command_buffer);

// -------------------------------------------------------------------
// -- Render Pass ----------------------------------------------------
typedef struct RHI_VK_RenderPass RHI_VK_RenderPass;
struct RHI_VK_RenderPass {
  RHI_RenderPass header;
  VkRenderPass   vk;

  struct RHI_VK_Framebuffer* framebuffer;
};
RHI_VK_RenderPass RHI_VK_RenderPassNil = ZeroStruct();
DefineArray(RHI_VK_RenderPass, RHI_VK_RenderPassArray, RHI_VK_RenderPassNil)

func B32 RHI_VK_RenderPassEqual(RHI_VK_RenderPass a, RHI_VK_RenderPass b);

func RHI_VK_RenderPass* RHI_VK_CreateRenderPass(U32 color_targets_count, RHI_ColorTarget* color_targets, RHI_DepthStencilTarget* depth_stencil_target);
func VkRenderPass       RHI_VK_CreateTmpVkRenderPass(RHI_GraphicsPipelineCreateInfo* pipeline_info);
func RHI_RenderPass*    RHI_VK_BeginRenderPass(RHI_CommandBuffer command_buffer, U32 color_targets_count, RHI_ColorTarget* color_targets, RHI_DepthStencilTarget* depth_stencil_target, RHI_Resource* resources, I32 resources_count);
func void               RHI_VK_EndRenderPass(RHI_CommandBuffer command_buffer, RHI_RenderPass* render_pass);

// -------------------------------------------------------------------
// -- Framebuffer ----------------------------------------------------
typedef struct RHI_VK_Framebuffer RHI_VK_Framebuffer;
struct RHI_VK_Framebuffer {
  VkFramebuffer vk;
  VkImageView   color_attachments[RHI_MAX_COLOR_ATTACHMENTS];
  I32           color_attachments_count;
  VkImageView   depth_stencil_attachment;
  Vec2I32       size;
};
RHI_VK_Framebuffer RHI_VK_FramebufferNil = ZeroStruct();
DefineArray(RHI_VK_Framebuffer, RHI_VK_FramebufferArray, RHI_VK_FramebufferNil)

func B32                 RHI_VK_EqualFramebuffer(RHI_VK_Framebuffer a, RHI_VK_Framebuffer b);
func RHI_VK_Framebuffer* RHI_VK_CreateFramebuffer(RHI_VK_RenderPass* render_pass, RHI_ColorTarget* color_targets, I32 color_targets_count, RHI_DepthStencilTarget* depth_stencil_target);

// -------------------------------------------------------------------
// -- Descriptor Sets ------------------------------------------------
#define RHI_VK_MAX_POOL_COUNT 64
#define RHI_VK_SETS_PER_POOL 128
#define RHI_VK_MAX_UNIFORM_BUFFERS_PER_SET 4
#define RHI_VK_MAX_SAMPLERS_PER_SET 4

#define RHI_VK_VERTEX_SHADER_GLOBAL_UNIFORM_SET_SLOT 0
#define RHI_VK_VERTEX_SHADER_INSTANCE_UNIFORM_SET_SLOT 1
#define RHI_VK_FRAGMENT_SHADER_GLOBAL_UNIFORM_SET_SLOT 2
#define RHI_VK_FRAGMENT_SHADER_INSTANCE_UNIFORM_SET_SLOT 3

typedef struct RHI_VK_DescriptorPool RHI_VK_DescriptorPool;
struct RHI_VK_DescriptorPool {
	VkDescriptorPool vk_pools[RHI_VK_MAX_POOL_COUNT];
	VkDescriptorSet  vk_sets[RHI_VK_MAX_POOL_COUNT][RHI_VK_SETS_PER_POOL];
	I32              pool_count;
	I32              sets_count;
};

func void RHI_VK_BindShaderArguments(RHI_CommandBuffer command_buffer, RHI_ShaderKind stage, RHI_ShaderArgument* arguments, I32 arguments_count);

// -------------------------------------------------------------------
// -- Pipeline -------------------------------------------------------
#define RHI_VK_MAX_OBJECTS 1024

func RHI_Shader RHI_VK_CreateShader(Arena* arena, RHI_ShaderCreateInfo* info);

typedef struct RHI_VK_GraphicsPipeline RHI_VK_GraphicsPipeline;
struct RHI_VK_GraphicsPipeline {
  VkPipeline            vk;
  VkPipelineLayout      layout;
  RHI_Shader*           vertex_shader; // --AlNov: @TODO Not really like the idea of pointer to RHI_Shader
  RHI_Shader*           fragment_shader;
};
RHI_VK_GraphicsPipeline RHI_VK_GraphicsPipelineNil = {0};
DefineArray(RHI_VK_GraphicsPipeline, RHI_VK_GraphicsPipelineArray, RHI_VK_GraphicsPipelineNil)

func RHI_VK_GraphicsPipeline* RHI_VK_GraphicsPipelineFromHandle(RHI_GraphicsPipeline pipeline);
func RHI_GraphicsPipeline     RHI_VK_CreateGraphicsPipeline(RHI_GraphicsPipelineCreateInfo* info);
func void                     RHI_VK_DestroyGraphicsPipeline(RHI_GraphicsPipeline pipeline);
func void                     RHI_VK_BindGraphicsPipeline(RHI_CommandBuffer command_buffer, RHI_GraphicsPipeline pipeline);

// -------------------------------------------------------------------
// -- Set States And Draw --------------------------------------------
func void RHI_VK_SetViewport(RHI_CommandBuffer command_buffer, RectI32 viewport);
func void RHI_VK_SetScissor(RHI_CommandBuffer command_buffer, RectI32 scissor);
func void RHI_VK_DrawPrimitives(RHI_CommandBuffer command_buffer, U32 vertex_count, U32 instance_count, U32 first_vertex, U32 first_instance);
func void RHI_VK_DrawIndexedPrimitives(RHI_CommandBuffer command_buffer, U32 index_count, U32 instance_count, U32 first_index, I32 vertex_offset, U32 first_instance);
func void RHI_VK_PresentTexture(RHI_CommandBuffer command_buffer, RHI_Texture texture);

// -------------------------------------------------------------------
// -- Texture --------------------------------------------------------
typedef struct RHI_VK_Texture RHI_VK_Texture;
struct RHI_VK_Texture {
  RHI_TextureFormat     format;
	VkImage               image;
	VkImageView           view;
	VkDeviceMemory        memory;
  VkImageLayout         layout;
  VkImageAspectFlagBits aspect_mask;
  Vec2I32               size;
  B32                   from_swapchain; // --AlNov: If image created by swapchain, it has to be destroyed by swapchain
};
RHI_VK_Texture RHI_VK_TextureNil = {0};
DefineArray(RHI_VK_Texture, RHI_VK_TextureArray, RHI_VK_TextureNil)
U32 RHI_VK_TextureFreeListNil = RHI_nil;
DefineArray(U32, RHI_VK_TextureFreeList, RHI_VK_TextureFreeListNil)

func RHI_VK_Texture*   RHI_VK_TextureFromHandle(RHI_Texture handle);
func RHI_Texture       RHI_VK_CreateTexture(RHI_TextureCreateInfo* info);
func B32               RHI_VK_DestroyTexture(RHI_Texture texture);
func void              RHI_VK_LoadDataToTexture(U8* data, U64 data_size, RHI_Texture texture);
func void              RHI_VK_CopyTexture(RHI_CommandBuffer command_buffer, RHI_Texture source, RHI_Texture destination);
func U64               RHI_VK_CopyTextureToBuffer(RHI_CommandBuffer command_buffer, RHI_Texture texture, RHI_Buffer buffer);
func void              RHI_VK_CopyBufferToTexture(RHI_CommandBuffer command_buffer, RHI_Buffer buffer, U64 offset, RHI_Texture texture);
func RHI_TextureFormat RHI_VK_GetTextureFormat(RHI_Texture texture);
func Vec2I32           RHI_VK_GetTextureDimension(RHI_Texture texture);

func void RHI_VK_ChangeTextureLayout(VkCommandBuffer cmd, RHI_VK_Texture* texture, VkImageLayout new_layout);

typedef struct RHI_VK_TextureSampler RHI_VK_TextureSampler;
struct RHI_VK_TextureSampler {
  VkSampler vk;
};
RHI_VK_TextureSampler RHI_VK_TextureSamplerNil = {0};
DefineArray(RHI_VK_TextureSampler, RHI_VK_TextureSamplerArray, RHI_VK_TextureSamplerNil)

func RHI_VK_TextureSampler* RHI_VK_TextureSamplerFromHandle(RHI_TextureSampler sampler);
func RHI_TextureSampler     RHI_VK_CreateTextureSampler(RHI_TextureSamplerCreateInfo* info);

// -------------------------------------------------------------------
// -- Command Buffer -------------------------------------------------
typedef struct RHI_VK_CommandBuffer RHI_VK_CommandBuffer;
struct RHI_VK_CommandBuffer {
	VkCommandBuffer          vk[RHI_FRAMES_IN_FLIGHT];
	VkFence                  submit_fence[RHI_FRAMES_IN_FLIGHT];
	VkSemaphore              acquire_semaphore[RHI_FRAMES_IN_FLIGHT];
	RHI_VK_DescriptorPool    descriptor_pool[RHI_FRAMES_IN_FLIGHT];
  RHI_VK_RenderPass*       active_render_pass;
	RHI_VK_GraphicsPipeline* binded_graphics_pipeline;
  RHI_VK_Texture*          current_swapchain_texture;
  VkViewport               current_viewport;
};  
RHI_VK_CommandBuffer RHI_VK_CommandBufferNil = {0};
DefineArray(RHI_VK_CommandBuffer, RHI_VK_CommandBufferArray, RHI_VK_CommandBufferNil)

func RHI_VK_CommandBuffer* RHI_VK_CommandBufferFromHandle(RHI_CommandBuffer command_buffer);
func RHI_CommandBuffer     RHI_VK_GetCommandBuffer();
func void                  RHI_VK_ReleaseCommandBuffer(RHI_CommandBuffer command_buffer);
func void                  RHI_VK_BeginCommandBuffer(RHI_CommandBuffer command_buffer);
func VkCommandBuffer       RHI_VK_BeginSingleCmd();
func void                  RHI_VK_EndSingleCmd(VkCommandBuffer cmd);

// -------------------------------------------------------------------
// -- Global State ---------------------------------------------------
typedef struct RHI_VK_State RHI_VK_State;
struct RHI_VK_State {
  Arena*                       arena;
  VkInstance                   instance;
  RHI_VK_Device                device;
  RHI_VK_Swapchain             swapchain;
	VkCommandPool                command_pool;
  RHI_VK_SemaphoreArray        semaphores;
  RHI_VK_BufferArray           buffers;
  RHI_VK_RenderPassArray       render_passes;
  RHI_VK_FramebufferArray      framebuffers;
  RHI_VK_GraphicsPipelineArray graphics_pipelines;
  RHI_VK_CommandBufferArray    command_buffers;
  RHI_VK_TextureSamplerArray   samplers;
  RHI_VK_TextureArray          textures;
  RHI_VK_TextureFreeList       textures_free_list;
  U32                          current_frame;
  U32                          current_target;

#if IGNIS_VULKAN_DEBUG
  VkDebugUtilsMessengerEXT debug_messenger;
#endif // IGNIS_VULKAN_DEBUG
} _rhi_vk_state;

func B32  RHI_VK_Init(OS_Window* window);
func B32  RHI_VK_Shutdown(void);
func void RHI_VK_HandleResize(OS_Window* window);

// -------------------------------------------------------------------
// -- Debug Tools ----------------------------------------------------
#if IGNIS_VULKAN_DEBUG
VKAPI_ATTR VkBool32 VKAPI_CALL RHI_VK_DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageKind, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
func VkDebugUtilsMessengerCreateInfoEXT RHI_VK_PopulateDebugMessengerCreateInfo(void);
func VkResult RHI_VK_CreateDebugUtilsMessenger(VkInstance instance, VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMesseneger);
func void RHI_VK_DestroyDebugUtilsMessenger(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, VkAllocationCallbacks* pAllocator);
func VkResult RHI_VK_CreateDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT* debugMessenger);
#endif // IGNIS_VULKAN_DEBUG
