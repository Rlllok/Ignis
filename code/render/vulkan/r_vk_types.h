#pragma once
#include "../../third_party/vulkan/include/vulkan.h"
#include "../../third_party/vulkan/include/vulkan_win32.h"

#include "../../base/base_include.h"

// -------------------------------------------------------------------
// AlNov: Device -----------------------------------------------------
struct R_VK_Device
{
  VkDevice logical;
  VkPhysicalDevice physical;
  u32 queue_index;
};

// -------------------------------------------------------------------
// --AlNov: Render Pass ----------------------------------------------
enum R_VK_RenderPassState
{
  R_VK_RENDER_PASS_STATE_NONE,
  R_VK_RENDER_PASS_STATE_READY,
  R_VK_RENDER_PASS_STATE_RECORDING,
  R_VK_RENDER_PASS_STATE_IN_RENDER_PASS,
  R_VK_RENDER_PASS_STATE_RECORDING_ENDED,
  R_VK_RENDER_PASS_STATE_SUBMITTED,
  R_VK_RENDER_PASS_STATE_NOT_ALLOCATED,

  R_VK_RENDER_PASS_STATE_COUNT
};

struct R_VK_RenderPass
{
  VkRenderPass         handle;
  Rect2f               render_area;
  Vec4f                clear_color;
  f32                  clear_depth;
  u32                  clear_stencil;
  R_VK_RenderPassState state;
};

// -------------------------------------------------------------------
// --AlNov: Command Buffer -------------------------------------------
enum R_VK_CommandBufferState
{
  R_VK_COMMAND_BUFFER_STATE_NONE,
  R_VK_COMMAND_BUFFER_STATE_READY,
  R_VK_COMMAND_BUFFER_STATE_RECORDING,
  R_VK_COMMAND_BUFFER_STATE_IN_RENDER_PASS,
  R_VK_COMMAND_BUFFER_STATE_RECORDING_ENDED,
  R_VK_COMMAND_BUFFER_STATE_SUBMITTED,
  R_VK_COMMAND_BUFFER_STATE_NOT_ALLOCATED,

  R_VK_COMMAND_BUFFER_STATE_COUNT
};

struct R_VK_CommandBuffer
{
  VkCommandBuffer handle;

  R_VK_CommandBufferState state;
};

struct R_VK_CommandPool
{
  VkCommandPool pool;
  VkCommandBuffer buffers[NUM_FRAMES_IN_FLIGHT];
};

// -------------------------------------------------------------------
// --AlNov: Framebuffer ----------------------------------------------
struct R_VK_Framebuffer
{
  VkFramebuffer    handle;
  u32              attachment_count;
  VkImageView*     attachments;
  R_VK_RenderPass* render_pass;
};

// -------------------------------------------------------------------
// --AlNov: Swapchain ------------------------------------------------
struct R_VK_Swapchain
{
  VkSwapchainKHR     handle;
  VkSurfaceKHR       surface;
  VkSurfaceFormatKHR surface_format;
  Vec2u              size;
  u32                image_count;
  VkImage*           images;
  VkImageView*       image_views;
  R_VK_Framebuffer*  framebuffers;
};

// -------------------------------------------------------------------
// --AlNov: Pipeline -------------------------------------------------
struct R_VK_Pipeline
{
  VkPipeline       handle;
  VkPipelineLayout layout;
};

// -------------------------------------------------------------------
// --AlNov: Shader ---------------------------------------------------
enum R_VK_ShaderType
{
  R_VK_SHADER_TYPE_NONE,
  R_VK_SHADER_TYPE_VERTEX,
  R_VK_SHADER_TYPE_FRAGMENT,

  R_VK_SHADER_TYPE_COUNT
};

struct R_VK_ShaderStage
{
  R_VK_ShaderType type;
  const char*     enter_point;
  u32             code_size;
  u8*             code;
  
  VkShaderModule                  vk_handle;
  VkPipelineShaderStageCreateInfo vk_info;
};

struct R_VK_ShaderProgram
{
  R_VK_ShaderStage      shader_stages[R_VK_SHADER_TYPE_COUNT];
  R_VK_Pipeline         pipeline;
  VkDescriptorSetLayout set_layout;
};

// -------------------------------------------------------------------
// --AlNov: Mesh Info ------------------------------------------------
// @TODO TO MOVE
struct R_VK_MVP
{
  // --AlNov: @TDO Projection matrix should be stored somewhere else
  alignas(16) Vec3f   color;
  alignas(16) Mat4x4f view;
  alignas(16) Mat4x4f translation;
};

struct R_MeshVertex
{
  Vec3f position;
  Vec3f normal;
  Vec2f uv;
};

struct R_Mesh
{
  R_VK_MVP mvp;

  R_Mesh* next;
  R_Mesh* previous;

  R_MeshVertex* vertecies;
  u32           vertex_count;
  VkDeviceSize  vertex_offset;
  
  u32*         indecies;
  u32          index_count;
  VkDeviceSize index_offset;

  VkDescriptorSet mvp_set;
  VkDeviceSize mvp_offset;
};

struct R_MeshList
{
    R_Mesh* first;
    R_Mesh* last;
    u32 count;
};

struct R_View
{
  Vec2f   size;
  Vec3f   position;
  f32     fov;

  struct
  {
    alignas(16) Mat4x4f projection;
  } uniform;
};

// -------------------------------------------------------------------
// --AlNov: Line Info ------------------------------------------------
// @TODO TO MOVE
struct R_LineVertex
{
  Vec3f position;
};

struct R_Line
{
  R_Line* next;
  R_Line* previous;
  
  R_LineVertex vertecies[2];
};

struct R_LineList
{
  R_Line* first;
  R_Line* last;
  u32 count;
};

// -------------------------------------------------------------------
// --AlNov: Texture --------------------------------------------------
struct R_Texture
{
  VkImage        vk_image;
  VkImageView    vk_view;
  VkDeviceMemory vk_memory;
  VkDeviceSize   size;
};

// -------------------------------------------------------------------
// --AlNov: Main States ----------------------------------------------

struct R_VK_DescriptorPool
{
  VkDescriptorPool pool;
};

struct R_VK_SyncTools
{
  VkFence fences[NUM_FRAMES_IN_FLIGHT];
  VkSemaphore image_available_semaphores[NUM_FRAMES_IN_FLIGHT];
  VkSemaphore image_ready_semaphores[NUM_FRAMES_IN_FLIGHT];
};

struct R_VK_VertexBuffer
{
  VkBuffer buffer;
  VkDeviceMemory memory;
  void* mapped_memory;
  u32 current_position;
  u32 size;
};

struct R_VK_IndexBuffer
{
  VkBuffer buffer;
  VkDeviceMemory memory;
  void* mapped_memory;
  u32 current_position;
  u32 size;
};

struct R_VK_Buffer
{
  VkBuffer        buffer;
  VkDeviceMemory  memory;
  void*           mapped_memory;
  u32             current_position;
  u32             size;
};

// -------------------------------------------------------------------
// --AlNov: State ----------------------------------------------------
struct R_VK_State
{
  Arena* arena;

  VkInstance           instance;
  R_VK_Device          device;
  R_VK_Swapchain       swapchain;
  VkCommandPool        command_pool;
  R_VK_CommandBuffer*  command_buffers;
  R_VK_DescriptorPool  descriptor_pool;
  R_VK_SyncTools       sync_tools;
  R_VK_Buffer          big_buffer;
  R_VK_Buffer          staging_buffer;
  
  R_VK_RenderPass render_pass;

  R_VK_ShaderProgram mesh_program;
  R_VK_ShaderProgram sphere_program;
  R_VK_ShaderProgram line_program;
  R_VK_ShaderProgram fullscreen_program;

  R_View view;

  R_MeshList mesh_list;
  R_LineList line_list;

  R_Texture texture;
  VkSampler sampler;

  VkImage        depth_image;
  VkImageView    depth_view;
  VkDeviceMemory depth_memory;
};
