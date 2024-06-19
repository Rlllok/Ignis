#pragma once

#include "../../third_party/vulkan/include/vulkan.h"
#include "../../third_party/vulkan/include/vulkan_win32.h"

#include "../../base/base_include.h"

#define NUM_FRAMES_IN_FLIGHT 3

// -------------------------------------------------------------------
// --AlNov: Mesh Info ------------------------------------------------
// @TODO TO MOVE
struct R_VK_MVP
{
  // --AlNov: @TDO Projection matrix should be stored somewhere else
  alignas(16) Vec3f   color;
  alignas(16) Vec3f   center_position;
  alignas(16) Mat4x4f view;
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
struct R_VK_Device
{
  VkDevice logical;
  VkPhysicalDevice physical;
  u32 queue_index;
};

struct R_VK_WindowResources
{
  VkSwapchainKHR swapchain;
  VkSurfaceKHR surface;
  VkSurfaceFormatKHR surface_format;
  Vec2u size;
  u32 image_count;
  VkImage* images;
  VkImageView* image_views;
  VkFramebuffer* framebuffers;

  bool is_window_resized;
};

struct R_VK_CommandPool
{
  VkCommandPool pool;
  VkCommandBuffer buffers[NUM_FRAMES_IN_FLIGHT];
};

struct R_VK_DescriptorPool
{
  VkDescriptorPool pool;
};

struct R_VK_Pipeline
{
  VkPipeline pipeline;
  VkPipelineLayout layout;
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
// --AlNov: Pipeline -------------------------------------------------
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
  
  // --AlNov: Vulkan
  VkShaderModule                  vk_handle;
  VkPipelineShaderStageCreateInfo vk_info;
};


// -------------------------------------------------------------------
// --AlNov: Globals --------------------------------------------------
struct R_VK_State
{
  Arena* arena;

  VkInstance           instance;
  R_VK_Device          device;
  R_VK_WindowResources window_resources;
  R_VK_CommandPool     cmd_pool;
  R_VK_DescriptorPool  descriptor_pool;
  R_VK_SyncTools       sync_tools;
  R_VK_Buffer          big_buffer;
  R_VK_Buffer          staging_buffer;
  
  VkRenderPass render_pass;
  R_VK_Pipeline mesh_pipeline;
  R_VK_Pipeline sphere_pipeline;
  R_VK_Pipeline line_pipeline;

  VkDescriptorSetLayout mvp_layout;

  R_MeshList mesh_list;
  R_LineList line_list;

  R_Texture texture;
  VkSampler sampler;
};
global R_VK_State r_vk_state;

// -------------------------------------------------------------------
// --AlNov: Init Stuff -----------------------------------------------
func void R_VK_Init(OS_Window* window);
func void R_VK_CreateInstance();
func void R_VK_CreateDevice();
func void R_VK_CreateSurface(OS_Window* window);
func void R_VK_CreateSwapchain();
func void R_VK_CreateCommandPool();
func void R_VK_CreateDescriptorPool();
func void R_VK_CreateMvpSetLayout();
func void R_VK_CreateRenderPass();
func void R_VK_CreateMeshPipeline();
func void R_VK_CreateSpherePipeline();
func void R_VK_CreateLinePipeline();
func void R_VK_CreateFramebuffers();
func void R_VK_AllocateCommandBuffers();
func void R_VK_CreateSyncTools();

// -------------------------------------------------------------------
// --AlNov: Pipeline Functions ---------------------------------------
func R_VK_ShaderStage R_VK_CreateShaderModule(Arena* arena, const char* path, const char* enter_point, R_VK_ShaderType type);
func R_VK_Pipeline    R_VK_CreatePipeline(R_VK_ShaderStage* vertex_shader_stage, R_VK_ShaderStage* fragment_shader_stage);

// -------------------------------------------------------------------
// --AlNov: Draw Functions -------------------------------------------
func void R_DrawFrame();
func void R_EndFrame();

// -------------------------------------------------------------------
// --AlNov: Helpers --------------------------------------------------
func u32  R_VK_FindMemoryType(u32 filter, VkMemoryPropertyFlags flags);
func void R_VK_CreateBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags property_flags, u32 size, VkBuffer* out_buffer, VkDeviceMemory* out_memory);
func void R_VK_PushBuffer(R_VK_Buffer* buffer, void* data, u64 size);
func void R_VK_PushMeshToBuffer(R_Mesh* mesh);
func void R_VK_MemCopy(VkDeviceMemory memory, void* data, u64 size);

// AlNov: From Vulkan Tutorial @TODO Maybe should be replaced
func VkCommandBuffer R_VK_BeginSingleCommands();
func void            R_VK_EndSingleCommands(VkCommandBuffer command_buffer);
func void            R_VK_CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);
func void            R_VK_CopyBufferToImage(VkBuffer buffer, VkImage image, Vec2u image_dimensions);
func void            R_VK_TransitImageLayout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);

// -------------------------------------------------------------------
// --AlNov: Mesh List Functions
func void R_PushMesh(R_MeshList* list, R_Mesh* mesh);
func void R_AddMeshToDrawList(R_Mesh* mesh);

// -------------------------------------------------------------------
// --AlNov: Line List Functions
func void R_PushLine(R_LineList* list, R_Line* line);
func void R_AddLineToDrawList(R_Line* line);

// -------------------------------------------------------------------
// --AlNov: Texture --------------------------------------------------
func R_Texture R_VK_CreateTexture(const char* path);
