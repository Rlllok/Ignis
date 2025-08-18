#pragma once

#include "../base/base_include.h"
#include "base/base_math.h"

#define R_FRAMES_IN_FLIGHT 1
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
func void R_ResetBuffer(R_Buffer* buffer);

typedef U8 R_IndexSize;
typedef enum R_IndexSizeEnum
{
	R_INDEX_SIZE_U16,
	R_INDEX_SIZE_U32,

	R_INDEX_SIZE_COUNT
} R_IndexSizeEnum;

func void R_BindIndexBuffer(R_CommandBuffer* command_buffer, R_Buffer* buffer, U64 offset, R_IndexSize index_size);
func void R_BindVertexBuffer(R_CommandBuffer* command_buffer, R_Buffer* buffer, U64 offset);

// -------------------------------------------------------------------
// Texture
typedef U8 R_TextureType;
enum R_TextureTypeEnum
{
  R_TEXTURE_TYPE_2D,
} R_TextureTypeEnum;

typedef U8 R_TextureFormat;
enum R_TextureFormatEnum
{
  R_TEXTURE_FORMAT_NONE,
  R_TEXTURE_FORMAT_R8G8B8A8_UNORM_SRGB,
  R_TEXTURE_FORMAT_B8G8R8A8_UNORM,
  R_TEXTURE_FORMAT_D16_UNORM,
} R_TextureFormatEnum;

typedef U16 R_TextureUsageFlags;
enum
{
  R_TEXTURE_USAGE_FLAG_DEPTH_STENCIL_ATTACHMENT = 1 << 1,
};

typedef struct R_TextureCreateInfo R_TextureCreateInfo;
struct R_TextureCreateInfo
{
  R_TextureType type;
  R_TextureFormat format;
  R_TextureUsageFlags usage_flags;
  U32 width;
  U32 height;
  U32 depth;
  U32 num_levels;
  // sample_count
};
typedef struct R_Texture R_Texture;

func R_Texture* R_CreateTexture(R_TextureCreateInfo* info);

// -------------------------------------------------------------------
// Uniform Data
func void R_BindGlobalVertexUniformData(R_CommandBuffer* command_buffer, R_Buffer* buffer, U64 offset, U64 data_size);
func void R_BindGlobalFragmentUniformData(R_CommandBuffer* command_buffer, R_Buffer* buffer, U64 offset, U64 data_size);

// -------------------------------------------------------------------
// Swapchain
func R_TextureFormat R_GetSwapchainTextureFormat();
func R_Texture* R_AcquireSwapchainTexture(R_CommandBuffer* command_buffer);

// -------------------------------------------------------------------
// Render Pass
typedef U8 R_LoadOperation;
typedef enum R_LoadOperation
{
	R_ATTACHMENT_LOAD_OPERATION_DONT_CARE,
	R_ATTACHMENT_LOAD_OPERATION_CLEAR,
	R_ATTACHMENT_LOAD_OPERATION_LOAD,
} R_LoadOperationEnum;

typedef U8 R_StoreOperation;
typedef enum R_StoreOperationEnum
{
	R_ATTACHMENT_STORE_OPERATION_DONT_CARE,
	R_ATTACHMENT_STORE_OPERATION_STORE,
} R_StoreOperationEnum;

typedef struct R_ColorTarget R_ColorTarget;
struct R_ColorTarget
{
	R_Texture* texture;
	R_LoadOperation load_operation;
	R_StoreOperation store_operation;
	Vec4F32 clear_color;
};

typedef struct R_DepthStencilTarget R_DepthStencilTarget;
struct R_DepthStencilTarget
{
  R_Texture* texture;
  R_LoadOperation depth_load_operation;
  R_StoreOperation depth_store_operation;
  F32 clear_depth;
};

typedef struct R_RenderPass R_RenderPass;

func R_RenderPass* R_BeginRenderPass(R_CommandBuffer* command_buffer, U32 color_targets_count, R_ColorTarget* color_targets, R_DepthStencilTarget* depth_stencil_target);
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

	U32 global_uniform_count;
};

typedef U8 R_VertexAttributeFormat;
typedef enum R_VertexAttributeFormatEnum
{
  R_VERTEX_ATTRIBUTE_FORMAT_VEC2F32,
  R_VERTEX_ATTRIBUTE_FORMAT_VEC3F32,
	R_VERTEX_ATTRIBUTE_FORMAT_VEC4F32,

  R_VERTEX_ATTRIBUTE_FORMAT_COUNT
} R_VertexAttributeFormatEnum;

func U32 GetSizeOfVertexAttributeFormat(R_VertexAttributeFormat format);

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

typedef U8 R_CompareOperation;
typedef enum R_CompareOperationEnum
{
  R_COMPARE_OPERATION_EQUAL,
  R_COMPARE_OPERATION_NOT_EQUAL,
  R_COMPARE_OPERATION_LESS,
  R_COMPARE_OPERATION_LESS_OR_EQUAL,
  R_COMPARE_OPERATION_GREATER,
  R_COMPARE_OPERATION_GREATER_OR_EQUAL,

  R_COMPARE_OPERATION_COUNT
} R_CompareOperationEnum;


typedef struct R_PipelineDepthStencilState R_PipelineDepthStencilState;
struct R_PipelineDepthStencilState
{
  B32 depth_test_enable;
  B32 depth_write_enable;
  R_CompareOperation depth_compare_operation;
  // @TODO Without Stencil for now
};

typedef struct R_GraphicsPipelineTargetInfo R_GraphicsPipelineTargetInfo;
struct R_GraphicsPipelineTargetInfo
{
  U32 color_targets_count;
  R_TextureFormat* color_targets_formats;
  R_TextureFormat depth_target_format;
};

typedef struct R_GraphicsPipelineCreateInfo R_GraphicsPipelineCreateInfo;
struct R_GraphicsPipelineCreateInfo
{
	R_Shader vertex_shader;
	R_Shader fragment_shader;
	U32 vertex_attributes_count;
	R_VertexAttribute* vertex_attributes;
  R_PipelineDepthStencilState depth_stencil_state;
  R_GraphicsPipelineTargetInfo target_info;
};

typedef struct R_GraphicsPipeline R_GraphicsPipeline;

func R_Shader R_CreateShader(Arena* arena, Str8 file_name, R_ShaderType type, U32 global_uniform_count);
func R_GraphicsPipeline* R_CreateGraphicsPipeline(R_GraphicsPipelineCreateInfo* info);
func void R_BindGraphicsPipeline(R_CommandBuffer* command_buffer, R_GraphicsPipeline* pipeline);

// -------------------------------------------------------------------
// Draw
func void R_SetViewport(R_CommandBuffer* command_buffer, RectI32 viewport);
func void R_SetScissor(R_CommandBuffer* command_buffer, RectI32 scissor);

func void R_DrawPrimitives(R_CommandBuffer* command_buffer, U32 vertex_count, U32 instance_count, U32 first_vertex, U32 first_instance);
func void R_DrawIndexedPrimitives(R_CommandBuffer* command_buffer, U32 index_count, U32 instance_count, U32 first_index, I32 vertex_offset, U32 first_instance);

// -------------------------------------------------------------------
// Device
typedef struct R_Device R_Device;
struct R_Device
{
	B32 (*Init)(OS_Window* window);

	// Buffer
	R_Buffer* (*CreateBuffer)(U32 capacity, R_BufferUsageFlags usage_flags, R_BufferPropertyFlags property_flags);
	U64 (*PushBuffer)(R_Buffer* buffer, U8* data, U64 size);
	void (*ResetBuffer)(R_Buffer* buffer);
	void (*BindIndexBuffer)(R_CommandBuffer* command_buffer, R_Buffer* buffer, U64 offset, R_IndexSize index_size);
	void (*BindVertexBuffer)(R_CommandBuffer* command_buffer, R_Buffer* buffer, U64 offset);

	// Uniform Data
	void (*BindGlobalVertexUniformData)(R_CommandBuffer* command_buffer, R_Buffer* buffer, U64 offset, U64 data_size);
	void (*BindGlobalFragmentUniformData)(R_CommandBuffer* command_buffer, R_Buffer* buffer, U64 offset, U64 data_size);

  // Texture
  R_Texture* (*CreateTexture)(R_TextureCreateInfo* info);

	// Command Buffer
	R_CommandBuffer* (*GetCommandBuffer)(void);
	void (*BeginCommandBuffer)(R_CommandBuffer* command_buffer);
	void (*SubmitCommandBuffer)(R_CommandBuffer* command_buffer);

	// Swapchain
  R_TextureFormat (*GetSwapchainTextureFormat)();
	R_Texture* (*AcquireSwapchainTexture)(R_CommandBuffer* command_buffer);

	// Render Pass
	R_RenderPass* (*BeginRenderPass)(R_CommandBuffer* command_buffer, U32 color_targets_count, R_ColorTarget* color_targets, R_DepthStencilTarget* depth_stencil_target);
	void (*EndRenderPass)(R_CommandBuffer* command_buffer, R_RenderPass* render_pass);
	
	// Graphics Pipeline
	R_GraphicsPipeline* (*CreateGraphicsPipeline)(R_GraphicsPipelineCreateInfo* info);
	void (*BindGraphicsPipeline)(R_CommandBuffer* command_buffer, R_GraphicsPipeline* pipeline);

	// Draw
	void (*SetViewport)(R_CommandBuffer* command_buffer, RectI32 viewport);
	void (*SetScissor)(R_CommandBuffer* command_buffer, RectI32 scissor);
	void (*DrawPrimitives)(R_CommandBuffer* command_buffer, U32 vertex_count, U32 instance_count, U32 first_vertex, U32 first_instance);
	void (*DrawIndexedPrimitives)(R_CommandBuffer* command_buffer, U32 index_count, U32 instance_count, U32 first_index, I32 vertex_offset, U32 first_instance);
};

#define AssignDeviceFunction(api_name, function_name) _r_state.device.function_name = R_##api_name##_##function_name;
#define AssignDeviceFunctions(api_name) \
	AssignDeviceFunction(api_name, Init) \
	AssignDeviceFunction(api_name, CreateBuffer) \
	AssignDeviceFunction(api_name, PushBuffer) \
	AssignDeviceFunction(api_name, ResetBuffer) \
	AssignDeviceFunction(api_name, BindIndexBuffer) \
	AssignDeviceFunction(api_name, BindVertexBuffer) \
	AssignDeviceFunction(api_name, BindGlobalVertexUniformData) \
	AssignDeviceFunction(api_name, BindGlobalFragmentUniformData) \
  AssignDeviceFunction(api_name, CreateTexture) \
	AssignDeviceFunction(api_name, GetCommandBuffer) \
	AssignDeviceFunction(api_name, BeginCommandBuffer) \
	AssignDeviceFunction(api_name, SubmitCommandBuffer) \
  AssignDeviceFunction(api_name, GetSwapchainTextureFormat) \
	AssignDeviceFunction(api_name, AcquireSwapchainTexture) \
	AssignDeviceFunction(api_name, BeginRenderPass) \
	AssignDeviceFunction(api_name, EndRenderPass) \
	AssignDeviceFunction(api_name, CreateGraphicsPipeline) \
	AssignDeviceFunction(api_name, BindGraphicsPipeline) \
	AssignDeviceFunction(api_name, SetViewport) \
	AssignDeviceFunction(api_name, SetScissor) \
	AssignDeviceFunction(api_name, DrawPrimitives) \
	AssignDeviceFunction(api_name, DrawIndexedPrimitives) \

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
