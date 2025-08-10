#pragma once

#include "../base/base_include.h"
#include "base/base_math.h"
#include "assets/mesh.h"

#include "r_texture.h"

#define R_FRAMES_IN_FLIGHT 3
#define R_MAX_BINDINGS 4
#define R_MAX_VERTEX_ATTRIBUTES 8

// -------------------------------------------------------------------
// Buffer
enum R_BufferUsageFlagBits
{
  R_BUFFER_USAGE_FLAG_ZERO = 0,

  R_BUFFER_USAGE_FLAG_VERTEX  = 1 << 0,
  R_BUFFER_USAGE_FLAG_INDEX   = 1 << 1,
  R_BUFFER_USAGE_FLAG_UNIFORM = 1 << 2,
  R_BUFFER_USAGE_FLAG_TRANSFER_SRC = 1 << 3,

  R_BUFFER_USAGE_FLAG_COUNT
};
typedef U64 R_BufferUsageFlags;

// @TODO Only use COHERENT for now
enum R_BufferPropertyFlagBits
{
  R_BUFFER_PROPERTY_FLAG_ZERO = 0,

  R_BUFFER_PROPERTY_FLAG_HOST_COHERENT = 1 << 0,
  R_BUFFER_PROPERTY_FLAG_HOST_VISIBLE = 1 << 1,
};
typedef U64 R_BufferPropertyFlags;

struct R_Buffer;

func R_Buffer* R_CreateBuffer(U32 capacity, R_BufferUsageFlags usage_flags, R_BufferPropertyFlags);
func U64 R_PushBuffer(R_Buffer* buffer, U8* data, U64 size);

func void R_BindVertexBuffer(struct R_CommandBuffer* command_buffer, R_Buffer* buffer, U64 offset);

// -------------------------------------------------------------------
// CommandBuffer
struct R_CommandBuffer;

func R_CommandBuffer* R_GetCommandBuffer();
func void R_BeginCommandBuffer(R_CommandBuffer* command_buffer);
func void R_SubmitCommandBuffer(R_CommandBuffer* command_buffer);

// -------------------------------------------------------------------
// Texture
struct R_TextureTest;

// -------------------------------------------------------------------
// Swapchain
func R_TextureTest* R_AcquireSwapchainTexture(R_CommandBuffer* command_buffer);

// -------------------------------------------------------------------
// Render Pass
enum R_AttachmentLoadOperation
{
	R_ATTACHMENT_LOAD_OPERATION_DONT_CARE,
	R_ATTACHMENT_LOAD_OPERATION_CLEAR,
	R_ATTACHMENT_LOAD_OPERATION_LOAD,
};

enum R_AttachmentStoreOperation
{
	R_ATTACHMENT_STORE_OPERATION_DONT_CARE,
	R_ATTACHMENT_STORE_OPERATION_STORE,
};

struct R_ColorAttachment
{
	R_TextureTest* texture;
	R_AttachmentLoadOperation load_operation;
	R_AttachmentStoreOperation store_operation;
	Vec4f clear_color;
};

struct R_RenderPass;

func R_RenderPass* R_BeginRenderPass(R_CommandBuffer* command_buffer, R_ColorAttachment* color_attachment);
func void R_EndRenderPass(R_CommandBuffer* command_buffer, R_RenderPass* render_pass);

// -------------------------------------------------------------------
// Pipeline
enum R_ShaderType
{
  R_SHADER_TYPE_VERTEX,
  R_SHADER_TYPE_FRAGMENT,
  
  R_SHADER_TYPE_COUNT
};

enum R_ShaderLanguage
{
  R_SHADER_LANGUAGE_SPIRV,

  R_SHADER_LANGUAGE_COUNT
};

struct R_Shader
{
  R_ShaderType      type;
  R_ShaderLanguage  language; // --AlNov: Only SPIRV for now
  U32               code_size;
  U8*               code;
};

enum R_VertexAttributeFormat
{
  R_VERTEX_ATTRIBUTE_FORMAT_VEC2F,
  R_VERTEX_ATTRIBUTE_FORMAT_VEC3F,

  R_VERTEX_ATTRIBUTE_FORMAT_COUNT
};

struct R_VertexAttribute
{
	U32 location;
	R_VertexAttributeFormat format;
	U32 offset;
};

enum R_BindingType
{
  R_BINDING_TYPE_UNIFORM_BUFFER,
  R_BINDING_TYPE_TEXTURE_2D,

  R_BINDING_TYPE_COUNT
};

struct R_BindingInfo
{
  R_BindingType type;
  R_ShaderType  shader_type;
};

enum R_PipelineCullingMode
{
  R_PIPELINE_CULLING_MODE_NONE,

  R_PIPELINE_CULLING_MODE_BACK_CW,
  R_PIPELINE_CULLING_MODE_BACK_CCW,

  R_PIPELINE_CULLING_MODE_COUNT
};

struct R_GraphicsPipelineCreateInfo
{
	R_Shader vertex_shader;
	R_Shader fragment_shader;
	U32 vertex_attributes_count;
	R_VertexAttribute* vertex_attributes;
	R_BindingInfo global_bindings[R_MAX_BINDINGS];
	U32 global_bindings_count;
	R_BindingInfo instance_bindings[R_MAX_BINDINGS];
	U32 instance_bindings_count;
};

struct R_GraphicsPipeline;

func R_Shader R_CreateShader(Arena* arena, Str8 file_name, R_ShaderType type);
func R_GraphicsPipeline* R_CreateGraphicsPipeline(R_GraphicsPipelineCreateInfo* info);
func void R_BindGraphicsPipeline(R_CommandBuffer* command_buffer, R_GraphicsPipeline* pipeline);

// -------------------------------------------------------------------
// Draw
func void R_DrawPrimitives(R_CommandBuffer* command_buffer, U32 vertex_count, U32 instance_count, U32 first_vertex, U32 first_instance);

// -------------------------------------------------------------------
// Device
struct R_Device
{
	B32 (*Init)(OS_Window* window);

	// Buffer
	R_Buffer* (*CreateBuffer)(U32 capacity, R_BufferUsageFlags usage_flags, R_BufferPropertyFlags property_flags);
	U64 (*PushBuffer)(R_Buffer* buffer, U8* data, U64 size);
	void (*BindVertexBuffer)(R_CommandBuffer* command_buffer, R_Buffer* buffer, U64 offset);

	// Command Buffer
	R_CommandBuffer* (*GetCommandBuffer)();
	void (*BeginCommandBuffer)(R_CommandBuffer* command_buffer);
	void (*SubmitCommandBuffer)(R_CommandBuffer* command_buffer);

	// Swapchain
	R_TextureTest* (*AcquireSwapchainTexture)(R_CommandBuffer* command_buffer);

	// Render Pass
	R_RenderPass* (*BeginRenderPass)(R_CommandBuffer* command_buffer, R_ColorAttachment* color_attachment);
	void (*EndRenderPass)(R_CommandBuffer* command_buffer, R_RenderPass* render_pass);
	
	// Graphics Pipeline
	R_GraphicsPipeline* (*CreateGraphicsPipeline)(R_GraphicsPipelineCreateInfo* info);
	void (*BindGraphicsPipeline)(R_CommandBuffer* command_buffer, R_GraphicsPipeline* pipeline);

	// Draw
	void (*DrawPrimitives)(R_CommandBuffer* command_buffer, U32 vertex_count, U32 instance_count, U32 first_vertex, U32 first_instance);
};

#define AssignDeviceFunction(api_name, function_name) _r_state.device.function_name = R_##api_name##_##function_name;
#define AssignDeviceFunctions(api_name) \
	AssignDeviceFunction(api_name, Init) \
	AssignDeviceFunction(api_name, CreateBuffer) \
	AssignDeviceFunction(api_name, PushBuffer) \
	AssignDeviceFunction(api_name, BindVertexBuffer) \
	AssignDeviceFunction(api_name, GetCommandBuffer) \
	AssignDeviceFunction(api_name, BeginCommandBuffer) \
	AssignDeviceFunction(api_name, SubmitCommandBuffer) \
	AssignDeviceFunction(api_name, AcquireSwapchainTexture) \
	AssignDeviceFunction(api_name, BeginRenderPass) \
	AssignDeviceFunction(api_name, EndRenderPass) \
	AssignDeviceFunction(api_name, CreateGraphicsPipeline) \
	AssignDeviceFunction(api_name, BindGraphicsPipeline) \
	AssignDeviceFunction(api_name, DrawPrimitives) \

// -------------------------------------------------------------------
// State
enum R_RendererType
{
  R_RENDERER_TYPE_NONE,

  R_RENDERER_TYPE_VK,

  R_RENDERER_TYPE_COUNT
};

struct R_State
{
	R_Device device;

	R_RendererType renderer_type;
} _r_state;

func B32 R_Init(R_RendererType type, OS_Window* window);
func B32 R_Shutdown();
