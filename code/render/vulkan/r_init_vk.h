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
  alignas(16) Vec3f color;
  alignas(16) Vec3f center_position;
};

struct R_MeshVertex
{
  Vec3f position;
};

struct R_Mesh
{
  R_VK_MVP mvp;

  R_Mesh* next;
  R_Mesh* previous;

  R_MeshVertex* vertecies;
  u32 vertex_count;
  
  u32* indecies;
  u32 index_count;

  // --AlNov: @TODO Should not be here
  VkDescriptorSet mvp_set;
  VkBuffer mvp_buffer;
  VkDeviceMemory mvp_memory;
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

struct R_VK_State
{
  Arena* arena;

  VkInstance instance;
  R_VK_Device device;
  R_VK_WindowResources window_resources;
  R_VK_CommandPool cmd_pool;
  R_VK_DescriptorPool descriptor_pool;
  R_VK_SyncTools sync_tools;
  R_VK_VertexBuffer vertex_buffer;
  R_VK_IndexBuffer index_buffer;
  
  VkRenderPass render_pass;
  R_VK_Pipeline mesh_pipeline;
  R_VK_Pipeline line_pipeline;

  VkDescriptorSetLayout mvp_layout;

  R_MeshList mesh_list;
  R_LineList line_list;
};

// -------------------------------------------------------------------
// --AlNov: Globals --------------------------------------------------
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
func void R_VK_CreateLinePipeline();
func void R_VK_CreateFramebuffers();
func void R_VK_AllocateCommandBuffers();
func void R_VK_CreateSyncTools();
func void R_VK_CreateVertexBuffer();
func void R_VK_CreateIndexBuffer();

// -------------------------------------------------------------------
// --AlNov: Draw Functions -------------------------------------------
func void R_DrawFrame();
func void R_EndFrame();

// -------------------------------------------------------------------
// --AlNov: Helpers --------------------------------------------------
func void R_VK_CreateBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags property_flags, u32 size, VkBuffer* out_buffer, VkDeviceMemory* out_memory);
// --AlNov: @TODO Maybe Vertex and Index buffer can be replaced with one R_VK_Buffer structure
func void R_VK_PushVertexBuffer(R_VK_VertexBuffer* buffer, void* data, u64 size);
func void R_VK_PushIndexBuffer(R_VK_IndexBuffer* buffer, void* data, u64 size);
// --AlNov: @TODO Is it needed - it mappes memory inside
func void R_VK_MemCopy(VkDeviceMemory memory, void* data, u64 size);

// -------------------------------------------------------------------
// --AlNov: Mesh List Functions
func void R_PushMesh(R_MeshList* list, R_Mesh* mesh);
func void R_AddMeshToDrawList(R_Mesh* mesh);

// -------------------------------------------------------------------
// --AlNov: Line List Functions
func void R_PushLine(R_LineList* list, R_Line* line);
func void R_AddLineToDrawList(R_Line* line);
