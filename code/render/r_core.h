#pragma once

#include "../base/base_include.h"
#include "base/base_math.h"

#include "r_texture.h"

#define R_FRAMES_IN_FLIGHT 3
#define R_MAX_BINDINGS 4
#define R_MAX_VERTEX_ATTRIBUTES 8

// -------------------------------------------------------------------
// CommandBuffer
typedef struct R_CommandBuffer R_CommandBuffer;

func R_CommandBuffer* R_GetCommandBuffer(void);
func void R_BeginCommandBuffer(R_CommandBuffer* command_buffer);
func void R_SubmitCommandBuffer(R_CommandBuffer* command_buffer);

// -------------------------------------------------------------------
// Buffer
typedef U32 R_BufferUsageFlags;
enum
{
  R_BUFFER_USAGE_FLAG_ZERO = 0,

  R_BUFFER_USAGE_FLAG_VERTEX  = 1 << 0,
  R_BUFFER_USAGE_FLAG_INDEX   = 1 << 1,
  R_BUFFER_USAGE_FLAG_UNIFORM = 1 << 2,
  R_BUFFER_USAGE_FLAG_TRANSFER_SRC = 1 << 3,

  R_BUFFER_USAGE_FLAG_COUNT
};

// @TODO Only use COHERENT for now
typedef U32 R_BufferPropertyFlags;
enum
{
  R_BUFFER_PROPERTY_FLAG_ZERO = 0,

  R_BUFFER_PROPERTY_FLAG_HOST_COHERENT = 1 << 0,
  R_BUFFER_PROPERTY_FLAG_HOST_VISIBLE = 1 << 1,
};

typedef struct R_Buffer R_Buffer;

func R_Buffer* R_CreateBuffer(U32 capacity, R_BufferUsageFlags usage_flags, R_BufferPropertyFlags property_flags);
func U64 R_PushBuffer(R_Buffer* buffer, U8* data, U64 size);

func void R_BindVertexBuffer(R_CommandBuffer* command_buffer, R_Buffer* buffer, U64 offset);

// -------------------------------------------------------------------
// Texture
typedef struct R_TextureTest R_TextureTest;

// -------------------------------------------------------------------
// Swapchain
func R_TextureTest* R_AcquireSwapchainTexture(R_CommandBuffer* command_buffer);

// -------------------------------------------------------------------
// Render Pass
typedef U8 R_AttachmentLoadOperation;
typedef enum R_AttachmentLoadOperationEnum
{
	R_ATTACHMENT_LOAD_OPERATION_DONT_CARE,
	R_ATTACHMENT_LOAD_OPERATION_CLEAR,
	R_ATTACHMENT_LOAD_OPERATION_LOAD,
}
R_AttachmentLoadOperationEnum;

typedef U8 R_AttachmentStoreOperation;
typedef enum R_AttachmentStoreOperationEnum
{
	R_ATTACHMENT_STORE_OPERATION_DONT_CARE,
	R_ATTACHMENT_STORE_OPERATION_STORE,
} R_AttachmentStoreOperationEnum;

typedef struct R_ColorAttachment R_ColorAttachment;
struct R_ColorAttachment
{
	R_TextureTest* texture;
	R_AttachmentLoadOperation load_operation;
	R_AttachmentStoreOperation store_operation;
	Vec4F32 clear_color;
};

typedef struct R_RenderPass R_RenderPass;

func R_RenderPass* R_BeginRenderPass(R_CommandBuffer* command_buffer, R_ColorAttachment* color_attachment);
func void R_EndRenderPass(R_CommandBuffer* command_buffer, R_RenderPass* render_pass);

// -------------------------------------------------------------------
// Pipeline
typedef U8 R_ShaderType;
typedef enum R_ShaderTypeEnum
{
  R_SHADER_TYPE_VERTEX,
  R_SHADER_TYPE_FRAGMENT,
  
  R_SHADER_TYPE_COUNT
} R_ShaderTypeEnum;

typedef U8 R_ShaderLanguage;
typedef enum R_ShaderLanguageEnum
{
  R_SHADER_LANGUAGE_SPIRV,

  R_SHADER_LANGUAGE_COUNT
} R_ShaderLanguageEnum;

typedef struct R_Shader R_Shader;
struct R_Shader
{
  R_ShaderType      type;
  R_ShaderLanguage  language; // --AlNov: Only SPIRV for now
  U32               code_size;
  U8*               code;
};

typedef U8 R_VertexAttributeFormat;
typedef enum R_VertexAttributeFormatEnum
{
  R_VERTEX_ATTRIBUTE_FORMAT_VEC2F,
  R_VERTEX_ATTRIBUTE_FORMAT_VEC3F,

  R_VERTEX_ATTRIBUTE_FORMAT_COUNT
} R_VertexAttributeFormatEnum;

typedef struct R_VertexAttribute R_VertexAttribute;
struct R_VertexAttribute
{
	U32 location;
	R_VertexAttributeFormat format;
	U32 offset;
};

typedef U8 R_BindingType;
typedef enum R_BindingTypeEnum
{
  R_BINDING_TYPE_UNIFORM_BUFFER,
  R_BINDING_TYPE_TEXTURE_2D,

  R_BINDING_TYPE_COUNT
} R_BindingTypeEnum;

typedef struct R_BindingInfo R_BindingInfo;
struct R_BindingInfo
{
  R_BindingType type;
  R_ShaderType  shader_type;
};

typedef U8 R_PipelineCullingMode;
typedef enum R_PipelineCullingModeEnum
{
  R_PIPELINE_CULLING_MODE_NONE,

  R_PIPELINE_CULLING_MODE_BACK_CW,
  R_PIPELINE_CULLING_MODE_BACK_CCW,

  R_PIPELINE_CULLING_MODE_COUNT
} R_PipelineCullingModeEnum;

typedef struct R_GraphicsPipelineCreateInfo R_GraphicsPipelineCreateInfo;
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

typedef struct R_GraphicsPipeline R_GraphicsPipeline;

func R_Shader R_CreateShader(Arena* arena, Str8 file_name, R_ShaderType type);
func R_GraphicsPipeline* R_CreateGraphicsPipeline(R_GraphicsPipelineCreateInfo* info);
func void R_BindGraphicsPipeline(R_CommandBuffer* command_buffer, R_GraphicsPipeline* pipeline);

// -------------------------------------------------------------------
// Draw
func void R_DrawPrimitives(R_CommandBuffer* command_buffer, U32 vertex_count, U32 instance_count, U32 first_vertex, U32 first_instance);

// -------------------------------------------------------------------
// Device
typedef struct R_Device R_Device;
struct R_Device
{
	B32 (*Init)(OS_Window* window);

	// Buffer
	R_Buffer* (*CreateBuffer)(U32 capacity, R_BufferUsageFlags usage_flags, R_BufferPropertyFlags property_flags);
	U64 (*PushBuffer)(R_Buffer* buffer, U8* data, U64 size);
	void (*BindVertexBuffer)(R_CommandBuffer* command_buffer, R_Buffer* buffer, U64 offset);

	// Command Buffer
	R_CommandBuffer* (*GetCommandBuffer)(void);
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
typedef U8 R_RendererType;
typedef enum R_RendererTypeEnum
{
  R_RENDERER_TYPE_NONE,

  R_RENDERER_TYPE_VK,

  R_RENDERER_TYPE_COUNT
} R_RendererTypeEnum;

typedef struct R_State R_State;
struct R_State
{
	R_Device device;

	R_RendererType renderer_type;
} _r_state;

func B32 R_Init(R_RendererType type, OS_Window* window);
func B32 R_Shutdown(void);
