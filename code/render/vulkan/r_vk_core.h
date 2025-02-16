#pragma once

#include "../../third_party/vulkan/include/vulkan.h"
#include "../../third_party/vulkan/include/vulkan_win32.h"

#include "base/base_include.h"
#include "os/os_include.h"
#include "render/r_include.h"

#define VK_CHECK(expression) Assert(expression != VK_SUCCESS);

#define NUM_FRAMES_IN_FLIGHT 3

#include "r_vk_device.h"
#include "r_vk_render_pass.h"
#include "r_vk_swapchain.h"
#include "r_vk_command_buffer.h"
#include "r_vk_buffer.h"
#include "r_vk_pipeline.h"

struct R_View
{
  Vec2f   size;
  Vec3f   position;
  F32     fov;

  struct
  {
    alignas(16) Mat4x4f projection;
  } uniform;
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

enum R_CubeMapSideType
{
  R_CUBE_MAP_SIDE_TYPE_RIGHT,
  R_CUBE_MAP_SIDE_TYPE_LEFT,
  R_CUBE_MAP_SIDE_TYPE_TOP,
  R_CUBE_MAP_SIDE_TYPE_BOTTOM,
  R_CUBE_MAP_SIDE_TYPE_BACK,
  R_CUBE_MAP_SIDE_TYPE_FRONT,

  R_CUBE_MAP_SIDE_TYPE_COUNT
};

struct R_VK_CubeMap
{
  VkImage        image;
  VkImageView    view;
  VkDeviceMemory memory;
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

// -------------------------------------------------------------------
// --AlNov: State ----------------------------------------------------
#define MAX_BUFFER_COUNT 64

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

  R_VK_Buffer buffers[MAX_BUFFER_COUNT];
  U32        buffer_count;
  U32        free_buffer_index;
  
  R_VK_RenderPass render_pass;

  R_VK_Pipeline         pipelines[2];
  U32                   pipelines_count;

  U32 active_pipeline_index;

  R_VK_CommandBuffer* current_command_buffer;
  R_VK_Framebuffer*   current_framebuffer;

  U32 current_frame;
  U32 current_image_index;
  
  RectI current_viewport;
  RectI current_scissor;

  R_View view;

  R_SceneObject scene_object;

  R_Texture texture;
  R_VK_CubeMap cubemap;
  VkSampler sampler;

  VkImage        depth_image;
  VkImageView    depth_view;
  VkDeviceMemory depth_memory;
};

global R_VK_State r_vk_state;
// -------------------------------------------------------------------
// --AlNov: Init Stuff -----------------------------------------------
func B32  R_VK_Init(OS_Window* window);
func void R_VK_CreateInstance();
func void R_VK_CreateSurface(R_VK_State* vk_state, OS_Window* window, R_VK_Swapchain* swapchain);
func void R_VK_CreateDescriptorPool();
func void R_VK_CreateSyncTools();
func void R_VK_CreateDepthImage();

func B32  R_VK_BeginFrame();
func void R_VK_EndFrame();
func void R_VK_PresentFrame();
func void R_VK_Draw(R_DrawInfo* info);

// -------------------------------------------------------------------
// --AlNov: Helpers --------------------------------------------------
func U32  R_VK_FindMemoryType(U32 filter, VkMemoryPropertyFlags flags);
func void R_VK_CreateBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags property_flags, U32 size, VkBuffer* out_buffer, VkDeviceMemory* out_memory);
// func void R_VK_PushMeshToBuffer(R_Mesh* mesh);
func void R_VK_MemCopy(VkDeviceMemory memory, void* data, U64 size);
func VkShaderStageFlagBits R_VK_ShaderStageFromShaderType(R_ShaderType type);
func VkFormat R_VK_VkFormatFromAttributeFormat(R_VertexAttributeFormat format);
func VkDescriptorType R_VK_DescriptorTypeFromBindingType(R_BindingType type);

func VkCommandBuffer R_VK_BeginSingleCommands();
func void            R_VK_EndSingleCommands(VkCommandBuffer command_buffer);
func void            R_VK_CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);
func void            R_VK_CopyBufferToImage(VkBuffer buffer, VkImage image, Vec2u image_dimensions);
func void            R_VK_TransitImageLayout(VkImage image, VkFormat format, U32 layer_count, VkImageLayout old_layout, VkImageLayout new_layout);

// -------------------------------------------------------------------
// --AlNov: Texture --------------------------------------------------
func bool LoadTexture(const char* path, U8** out_data, I32* out_width, I32* out_height, I32* out_channels);

func R_Texture R_VK_CreateTexture(const char* path);
func void      R_VK_CreateCubeMap(const char* folder_path, R_VK_CubeMap* out_cubemap);

// -------------------------------------------------------------------
// --AlNov: View -----------------------------------------------------
func R_View R_CreateView(Vec3f position, F32 fov, Vec2f size);
func void   R_VK_BindView(R_View view);
