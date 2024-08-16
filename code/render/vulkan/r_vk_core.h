#pragma once

#include "../../third_party/vulkan/include/vulkan.h"
#include "../../third_party/vulkan/include/vulkan_win32.h"

#include "base/base_include.h"
#include "os/os_include.h"
#include "render/r_include.h"

#define VK_CHECK(expression) ASSERT(expression != VK_SUCCESS);

#define NUM_FRAMES_IN_FLIGHT 3

#include "r_vk_types.h"

global R_VK_State r_vk_state;

// -------------------------------------------------------------------
// --AlNov: Init Stuff -----------------------------------------------
func b8   R_VK_Init(OS_Window* window);
func void R_VK_CreateInstance();
func void R_VK_CreateDevice();
func void R_VK_CreateSurface(R_VK_State* vk_state, OS_Window* window, R_VK_Swapchain* swapchain);
func void R_VK_CreateSwapchain();
func void R_VK_CreateDescriptorPool();
func void R_VK_CreateSyncTools();
func void R_VK_CreateDepthImage();

// -------------------------------------------------------------------
// --AlNov: Render Pass ----------------------------------------------
func void R_VK_CreateRenderPass(R_VK_State* vk_state, R_VK_RenderPass* out_render_pass, Rect2f render_area);
func void R_VK_DestroyRenderPass(R_VK_State* vk_state, R_VK_RenderPass* render_pass);

func void R_VK_BeginFrame();
func void R_VK_EndFrame();
func void R_VK_BeginRenderPass(Vec4f clear_color, f32 clear_depth, f32 clear_stencil);
func void R_VK_EndRenderPass();
func void R_VK_Draw(R_DrawInfo* info);

// -------------------------------------------------------------------
// --AlNov: Command Buffer -------------------------------------------
func void R_VK_CreateCommandPool(R_VK_State* vk_state);
func void R_VK_AllocateCommandBuffer(R_VK_State* vk_state, VkCommandPool pool, R_VK_CommandBuffer* out_command_buffer);
func void R_VK_FreeCommandBuffer(R_VK_State* vk_state, VkCommandPool pool, R_VK_CommandBuffer* command_buffer);
func void R_VK_BeginCommandBuffer(R_VK_CommandBuffer* command_buffer);
func void R_VK_EndCommandBuffer(R_VK_CommandBuffer* command_buffer);
func void R_VK_SubmitComandBuffer(R_VK_CommandBuffer* command_buffer); // --AlNov: @TODO Only change state for now
func void R_VK_ResetCommandBuffer(R_VK_CommandBuffer* command_buffer);
func void R_VK_BeginSingleUseCommandBuffer(R_VK_State* vk_state, VkCommandPool pool, R_VK_CommandBuffer* out_command_buffer);
func void R_VK_EndSingleUseCommandBuffer(R_VK_State* vk_state, VkCommandPool pool, R_VK_CommandBuffer* command_buffer, VkQueue queue);

// -------------------------------------------------------------------
// --AlNov: Framebuffer ----------------------------------------------
func void R_VK_CreateFramebuffer(R_VK_State* vk_state, R_VK_RenderPass* render_pass, Vec2u size, u32 attachment_count, VkImageView* attachments, R_VK_Framebuffer* out_framebuffer);
func void R_VK_DestroyFramebuffer(R_VK_State* vk_state, R_VK_Framebuffer* framebuffer);

// -------------------------------------------------------------------
// --AlNov: Shader ---------------------------------------------------
func void R_VK_BindShaderProgram(R_VK_CommandBuffer* command_buffer, R_VK_ShaderProgram* program);

// -------------------------------------------------------------------
// --AlNov: Pipeline Functions ---------------------------------------
func b8   R_VK_CreatePipeline(R_Pipeline* pipeline);
func void R_VK_BindPipeline(R_Pipeline* pipeline);

// -------------------------------------------------------------------
// --AlNov: Helpers --------------------------------------------------
func u32  R_VK_FindMemoryType(u32 filter, VkMemoryPropertyFlags flags);
func void R_VK_CreateBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags property_flags, u32 size, VkBuffer* out_buffer, VkDeviceMemory* out_memory);
// func void R_VK_PushMeshToBuffer(R_Mesh* mesh);
func void R_VK_MemCopy(VkDeviceMemory memory, void* data, u64 size);
func VkShaderStageFlagBits R_VK_ShaderStageFromShaderType(R_ShaderType type);
func VkFormat R_VK_VkFormatFromAttributeFormat(R_VertexAttributeFormat format);
func VkDescriptorType R_VK_DescriptorTypeFromBindingType(R_BindingType type);

func VkCommandBuffer R_VK_BeginSingleCommands();
func void            R_VK_EndSingleCommands(VkCommandBuffer command_buffer);
func void            R_VK_CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);
func void            R_VK_CopyBufferToImage(VkBuffer buffer, VkImage image, Vec2u image_dimensions);
func void            R_VK_TransitImageLayout(VkImage image, VkFormat format, u32 layer_count, VkImageLayout old_layout, VkImageLayout new_layout);

// -------------------------------------------------------------------
// --AlNov: Texture --------------------------------------------------
func bool LoadTexture(const char* path, u8** out_data, i32* out_width, i32* out_height, i32* out_channels);

func R_Texture R_VK_CreateTexture(const char* path);
func void      R_VK_CreateCubeMap(const char* folder_path, R_VK_CubeMap* out_cubemap);

// -------------------------------------------------------------------
// --AlNov: View -----------------------------------------------------
func R_View R_CreateView(Vec3f position, f32 fov, Vec2f size);
func void   R_VK_BindView(R_View view);
