#pragma once

#include "../../third_party/vulkan/include/vulkan.h"
#include "../../third_party/vulkan/include/vulkan_win32.h"

#include "../../base/base_include.h"

#define VK_CHECK(expression) ASSERT(expression != VK_SUCCESS);

#define NUM_FRAMES_IN_FLIGHT 3

#include "r_vk_types.h"

global R_VK_State r_vk_state;

// -------------------------------------------------------------------
// --AlNov: Init Stuff -----------------------------------------------
func b8   R_VK_Init(OS_Window* window);
func void R_VK_CreateInstance();
func void R_VK_CreateDevice();
func void R_VK_CreateSurface(OS_Window* window);
func void R_VK_CreateSwapchain();
func void R_VK_CreateCommandPool();
func void R_VK_CreateDescriptorPool();
func void R_VK_CreateMvpSetLayout();
func void R_VK_CreateMeshPipeline();
func void R_VK_CreateSpherePipeline();
func void R_VK_CreateLinePipeline();
func void R_VK_CreateFramebuffers();
func void R_VK_AllocateCommandBuffers();
func void R_VK_CreateSyncTools();
func void R_VK_CreateDepthImage();

// -------------------------------------------------------------------
// --AlNov: Render Pass ----------------------------------------------
func void R_VK_CreateRenderPass(R_VK_State* vk_state, R_VK_RenderPass* out_render_pass, Rect2f render_area, Vec4f clear_color, f32 clear_depth, u32 clear_stencil);
func void R_VK_DestroyRenderPass(R_VK_State* vk_state, R_VK_RenderPass* render_pass);
func void R_VK_BeginRenderPass(VkCommandBuffer* command_buffer, R_VK_RenderPass* render_pass, VkFramebuffer framebuffer);
func void R_VK_EndRenderPass(VkCommandBuffer* command_buffer, R_VK_RenderPass* render_pass);

// -------------------------------------------------------------------
// --AlNov: Pipeline Functions ---------------------------------------
func R_VK_ShaderStage R_VK_CreateShaderModule(Arena* arena, const char* path, const char* enter_point, R_VK_ShaderType type);
func R_VK_Pipeline    R_VK_CreatePipeline(R_VK_ShaderStage* vertex_shader_stage, R_VK_ShaderStage* fragment_shader_stage);

// -------------------------------------------------------------------
// --AlNov: Draw Functions -------------------------------------------
func b8 R_VK_DrawFrame();
func b8 R_VK_EndFrame();

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

// -------------------------------------------------------------------
// --AlNov: View -----------------------------------------------------
func R_View R_CreateView(Vec3f position, f32 fov, Vec2f size);
func void   R_VK_BindView(R_View view);
