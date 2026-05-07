#pragma once

#include "../base/base_include.h"
#include "base/base_math.h"

#define RHI_FRAMES_IN_FLIGHT 2
#define RHI_MAX_BINDINGS 4
#define RHI_MAX_VERTEX_ATTRIBUTES 16

typedef U32 RHI_Handle;
#define RHI_nil 0

typedef U64 RHI_DeviceAddress;

typedef U8 RHI_CompareOperation;
typedef enum RHI_CompareOperationEnum {
  RHI_CompareOperation_Equal,
  RHI_CompareOperation_NotEqual,
  RHI_CompareOperation_Less,
  RHI_CompareOperation_LessOrEqual,
  RHI_CompareOperation_Greater,
  RHI_CompareOperation_GreaterOrEqual,

  RHI_CompareOperation_Count,
} RHI_CompareOperationEnum;

// -------------------------------------------------------------------
// -- Command Buffer -------------------------------------------------
typedef RHI_Handle RHI_CommandBuffer;

func RHI_CommandBuffer RHI_GetCommandBuffer(void);
func void RHI_BeginCommandBuffer(RHI_CommandBuffer command_buffer);
func void RHI_SubmitCommandBuffer(RHI_CommandBuffer command_buffer);

// -------------------------------------------------------------------
// -- Buffer ---------------------------------------------------------
typedef U32 RHI_BufferUsageFlags;
enum {
  RHI_BufferUsageFlag_Zero = 0,

  RHI_BufferUsageFlag_Vertex   = 1 << 0,
  RHI_BufferUsageFlag_Index    = 1 << 1,
  RHI_BufferUsageFlag_Uniform  = 1 << 2,
  RHI_BufferUsageFlag_Storage  = 1 << 3,
  RHI_BufferUsageFlag_Transfer = 1 << 4,
  RHI_BufferUsageFlag_Address  = 1 << 5,

  RHI_BufferUsageFlag_Count
};

// @TODO Only use COHERENT for now
typedef U32 RHI_BufferPropertyFlags;
enum {
  RHI_BufferPropertyFlag_Zero = 0,

  RHI_BufferPropertyFlag_HostCoherent = 1 << 0,
  RHI_BufferPropertyFlag_HostVisible = 1 << 1,
};

typedef RHI_Handle RHI_Buffer;

func RHI_Buffer        RHI_CreateBuffer(Str8 label, U32 capacity, RHI_BufferUsageFlags usage_flags, RHI_BufferPropertyFlags property_flags);
func U64               RHI_PushBuffer(RHI_Buffer buffer, U8* data, U64 size);
func void              RHI_ResetBuffer(RHI_Buffer buffer);
func RHI_DeviceAddress RHI_BufferDeviceAddress(RHI_Buffer buffer);

typedef U8 RHI_IndexSize;
typedef enum RHI_IndexSizeEnum {
	RHI_IndexSize_U16,
	RHI_IndexSize_U32,

	RHI_IndexSize_Count
} RHI_IndexSizeEnum;

func void RHI_BindIndexBuffer(RHI_CommandBuffer command_buffer, RHI_Buffer buffer, U64 offset, RHI_IndexSize index_size);
func void RHI_BindVertexBuffer(RHI_CommandBuffer command_buffer, RHI_Buffer buffer, U64 offset);

// -------------------------------------------------------------------
// -- Texture --------------------------------------------------------
typedef U8 RHI_TextureKind;
enum RHI_TextureKindEnum {
  RHI_TextureKind_2D,
} RHI_TextureKindEnum;

typedef U8 RHI_TextureFormat;
enum RHI_TextureFormatEnum {
  RHI_TextureFormat_None,
  RHI_TextureFormat_R8G8B8A8_SRGB,
  RHI_TextureFormat_R8G8B8A8_UNORM,
  RHI_TextureFormat_B8G8R8A8_UNORM,
  RHI_TextureFormat_R16G16B16A16_SFLOAT,
  RHI_TextureFormat_D16_UNORM,
  RHI_TextureFormat_R16_UINT,
} RHI_TextureFormatEnum;

typedef U16 RHI_TextureUsageFlags;
enum RHI_TextureUsageFlagsEnum {
  RHI_TEXTURE_USAGE_FLAG_COLOR_ATTACHMENT = 1 << 0,
  RHI_TEXTURE_USAGE_FLAG_DEPTH_STENCIL_ATTACHMENT = 1 << 1,
  RHI_TEXTURE_USAGE_FLAG_TRANSFER_SRC = 1 << 2,
  RHI_TEXTURE_USAGE_FLAG_TRANSFER_DST = 1 << 3,
  RHI_TEXTURE_USAGE_FLAG_SAMPLED = 1 << 4,
} RHI_TextureUsageFlagsEnum;

typedef struct RHI_TextureCreateInfo RHI_TextureCreateInfo;
struct RHI_TextureCreateInfo {
  RHI_TextureKind       kind;
  RHI_TextureFormat     format;
  RHI_TextureUsageFlags usage_flags;
  U32                   width;
  U32                   height;
  U32                   depth;
  U32                   num_levels;
  // sample_count
};
typedef RHI_Handle RHI_Texture;

func RHI_Texture       RHI_CreateTexture(RHI_TextureCreateInfo* info);
func B32               RHI_DestroyTexture(RHI_Texture texture);
func void              RHI_LoadImageToTexture(Str8 image_path, RHI_Texture texture);
func void              RHI_CopyTexture(RHI_CommandBuffer command_buffer, RHI_Texture source, RHI_Texture destination);
func U64               RHI_CopyTextureToBuffer(RHI_CommandBuffer command_buffer, RHI_Texture texture, RHI_Buffer buffer);
func void              RHI_CopyBufferToTexture(RHI_CommandBuffer command_buffer, RHI_Buffer buffer, U64 offset, U64 size, RHI_Texture texture);
func RHI_TextureFormat RHI_GetTextureFormat(RHI_Texture texture);
func Vec2I32           RHI_GetTextureDimension(RHI_Texture texture);

typedef U8 RHI_FilterKind;
enum RHI_FilterKindEnum {
  RHI_FilterKind_Nearest,
  RHI_FilterKind_Linear,
} RHI_FilterKindEnum;

typedef U8 RHI_SamplerMipmapMode;
enum RHI_SamplerMipmapModeEnum {
  RHI_SamplerMipmapMode_Nearest,
  RHI_SamplerMipmapMode_Linear,
} RHI_SamplerMimapModeEnum;

typedef U8 RHI_SamplerAddressMode;
enum RHI_SamplerAddressModeEnum {
  RHI_SamplerAddressMode_Repeat,
  RHI_SamplerAddressMode_MirroredRepeat,
  RHI_SamplerAddressMode_ClampToEdge,
  RHI_SamplerAddressMode_ClampToBorder,
} RHI_SamplerAddressModeEnum;

typedef struct RHI_TextureSamplerCreateInfo RHI_TextureSamplerCreateInfo;
struct RHI_TextureSamplerCreateInfo {
  RHI_FilterKind         mag_filter;
  RHI_FilterKind         min_filter;
  RHI_SamplerMipmapMode  mipmap_mode;
  RHI_SamplerAddressMode address_mode_u;
  RHI_SamplerAddressMode address_mode_v;
  RHI_SamplerAddressMode address_mode_w;
  F32                    mip_lod_bias;
  B32                    anisotropy_enable;
  F32                    max_anisotropy;
  F32                    compare_enable;
  RHI_CompareOperation   compare_operation;
  F32                    min_lod;
  F32                    max_lod;
};

typedef RHI_Handle RHI_TextureSampler;

func RHI_TextureSampler RHI_CreateTextureSampler(RHI_TextureSamplerCreateInfo* info);

// -------------------------------------------------------------------
// -- Uniform Data ---------------------------------------------------
typedef struct RHI_UniformBufferBindingInfo RHI_UniformBufferBindingInfo;
struct RHI_UniformBufferBindingInfo {
  U32        binding;
  RHI_Buffer buffer;
  U64        offset;
  U64        size;
};

typedef struct RHI_SamplerBindingInfo RHI_SamplerBindingInfo;
struct RHI_SamplerBindingInfo {
  U32                binding;
  RHI_TextureSampler sampler;
  RHI_Texture        texture;
};

func void RHI_BindGlobalVertexShaderData(RHI_CommandBuffer command_buffer, I32 uniform_buffers_count, RHI_UniformBufferBindingInfo* uniform_info, I32 samplers_count, RHI_SamplerBindingInfo* sampler_info);
func void RHI_BindInstanceVertexShaderData(RHI_CommandBuffer command_buffer, I32 uniform_buffers_count, RHI_UniformBufferBindingInfo* uniform_info, I32 samplers_count, RHI_SamplerBindingInfo* sampler_info);
func void RHI_BindGlobalFragmentShaderData(RHI_CommandBuffer command_buffer, I32 uniform_buffers_count, RHI_UniformBufferBindingInfo* uniform_info, I32 samplers_count, RHI_SamplerBindingInfo* sampler_info);
func void RHI_BindInstanceFragmentShaderData(RHI_CommandBuffer command_buffer, I32 uniform_buffers_count, RHI_UniformBufferBindingInfo* uniform_info, I32 samplers_count, RHI_SamplerBindingInfo* sampler_info);

// -------------------------------------------------------------------
// -- Swapchain ------------------------------------------------------
func RHI_TextureFormat RHI_GetSwapchainTextureFormat();
func RHI_Texture       RHI_AcquireSwapchainTexture(RHI_CommandBuffer command_buffer);

// -------------------------------------------------------------------
// -- Render Pass ----------------------------------------------------
#define RHI_MAX_COLOR_ATTACHMENTS 4

typedef U8 RHI_LoadOperation;
typedef enum RHI_LoadOperation {
  RHI_AttachmentLoadOperation_DontCare,
  RHI_AttachmentLoadOperation_Clear,
  RHI_AttachmentLoadOperation_Load,
} RHI_LoadOperationEnum;

typedef U8 RHI_StoreOperation;
typedef enum RHI_StoreOperationEnum {
  RHI_AttachmentStoreOperation_DontCare,
  RHI_AttachmentStoreOperation_Store,
} RHI_StoreOperationEnum;

typedef struct RHI_ColorTarget RHI_ColorTarget;
struct RHI_ColorTarget {
	RHI_Texture        texture;
	RHI_LoadOperation  load_operation;
	RHI_StoreOperation store_operation;
	Vec4F32            clear_color;
};

typedef struct RHI_DepthStencilTarget RHI_DepthStencilTarget;
struct RHI_DepthStencilTarget {
  RHI_Texture        texture;
  RHI_LoadOperation  load_operation;
  RHI_StoreOperation store_operation;
  F32                clear_depth;
};

typedef struct RHI_Resource RHI_Resource;
struct RHI_Resource {
  // --AlNov: @TODO Only Buffer for now
  RHI_Buffer buffer;
  // --AlNov: @TODO RHI_ResourceUsage usage;
  // --AlNvo: @TODO RHI_RenderStage stage;
};

typedef struct RHI_RenderPass RHI_RenderPass;
struct RHI_RenderPass {
  RHI_ColorTarget        color_targets[RHI_MAX_COLOR_ATTACHMENTS];
  I32                    color_targets_count;
  RHI_DepthStencilTarget depth_stencil_target;
};

func RHI_RenderPass* RHI_BeginRenderPass(RHI_CommandBuffer command_buffer, U32 color_targets_count, RHI_ColorTarget* color_targets, RHI_DepthStencilTarget* depth_stencil_target, RHI_Resource* resources, I32 resources_count);
func void            RHI_EndRenderPass(RHI_CommandBuffer command_buffer, RHI_RenderPass* render_pass);

// -------------------------------------------------------------------
// -- Pipeline -------------------------------------------------------
typedef U8 RHI_ShaderKind;
typedef enum RHI_ShaderKindEnum {
  RHI_ShaderKind_Vertex,
  RHI_ShaderKind_Fragment,
  
  RHI_ShaderKind_Count
} RHI_ShaderKindEnum;

typedef U8 RHI_ShaderLanguage;
typedef enum RHI_ShaderLanguageEnum {
  RHI_ShaderLanguage_SPIRV,
  RHI_ShaderLanguage_Metal,

  RHI_ShaderLanguage_Count
} RHI_ShaderLanguageEnum;

typedef U8 RHI_ShaderArgumentKind;
typedef enum RHI_ShaderArgumentKindEnum {
  RHI_ShaderArgumentKind_BufferAddress,
} RHI_ShaderArgumentKindEnum;

typedef struct RHI_ShaderArgument RHI_ShaderArgument;
struct RHI_ShaderArgument {
  RHI_ShaderArgumentKind kind;
  union {
    RHI_DeviceAddress address;
  };
};

typedef struct RHI_Shader RHI_Shader;
struct RHI_Shader {
  RHI_ShaderKind          kind;
  RHI_ShaderLanguage      language; // --AlNov: Only SPIRV for now
  U32                     code_size;
  U8*                     code;
  RHI_ShaderArgumentKind* arguments;
  I32                     arguments_count;
};

typedef U8 RHI_VertexAttributeFormat;
typedef enum RHI_VertexAttributeFormatEnum {
  RHI_VertexAttributeFormat_Vec2F32,
  RHI_VertexAttributeFormat_Vec3F32,
  RHI_VertexAttributeFormat_Vec4F32,

  RHI_VertexAttributeFormat_Vec4I32,

  RHI_VertexAttributeFormat_Count
} RHI_VertexAttributeFormatEnum;

func U32 RHI_GetSizeOfVertexAttributeFormat(RHI_VertexAttributeFormat format);

typedef struct RHI_VertexAttribute RHI_VertexAttribute;
struct RHI_VertexAttribute {
	U32                       location;
	RHI_VertexAttributeFormat format;
	U32                       offset;
};

typedef U8 RHI_BindingKind;
typedef enum RHI_BindingKindEnum {
  RHI_BindingKind_UniformBuffer,
  RHI_BindingKind_Sampler,

  RHI_BindingKind_Count
} RHI_BindingKindEnum;

typedef struct RHI_BindingInfo RHI_BindingInfo;
struct RHI_BindingInfo {
  RHI_BindingKind kind;
  RHI_ShaderKind  shader_type;
};

typedef U8 RHI_PipelineCullingMode;
typedef enum RHI_PipelineCullingModeEnum {
  RHI_PipelineCullingMode_None,

  RHI_PipelineCullingMode_BackClockwise,
  RHI_PipelineCullingMode_BackCounterClockwise,

  RHI_PipelineCullingMode_Count
} RHI_PipelineCullingModeEnum;

typedef struct RHI_PipelineDepthStencilState RHI_PipelineDepthStencilState;
struct RHI_PipelineDepthStencilState {
  B32                  depth_test_enable;
  B32                  depth_write_enable;
  RHI_CompareOperation depth_compare_operation;
  RHI_TextureFormat    depth_target_format;
  // @TODO Without Stencil for now
};

typedef struct RHI_GraphicsPipelineColorTargetInfo RHI_GraphicsPipelineColorTargetInfo;
struct RHI_GraphicsPipelineColorTargetInfo {
  RHI_TextureFormat format;
  B32               blend_enable;
};

typedef struct RHI_GraphicsPipelineCreateInfo RHI_GraphicsPipelineCreateInfo;
struct RHI_GraphicsPipelineCreateInfo {
	RHI_Shader*                          vertex_shader;
	RHI_Shader*                          fragment_shader;
	U32                                  vertex_attributes_count;
	RHI_VertexAttribute*                 vertex_attributes;
  U32                                  color_targets_count;
  RHI_GraphicsPipelineColorTargetInfo* color_target_infos;
  RHI_PipelineDepthStencilState        depth_stencil_state;
};

typedef RHI_Handle RHI_GraphicsPipeline;

func RHI_GraphicsPipeline RHI_CreateGraphicsPipeline(RHI_GraphicsPipelineCreateInfo* info);
func void                 RHI_BindGraphicsPipeline(RHI_CommandBuffer command_buffer, RHI_GraphicsPipeline pipeline);

typedef struct RHI_ShaderCreateInfo RHI_ShaderCreateInfo;
struct RHI_ShaderCreateInfo {
  Str8                    file_name;
  RHI_ShaderKind          kind;
  RHI_ShaderArgumentKind* arguments;
  I32                     arguments_count;
};

func RHI_Shader RHI_CreateShader(Arena* arena, RHI_ShaderCreateInfo* info);

func void RHI_BindShaderArguments(RHI_CommandBuffer command_buffer, RHI_ShaderKind stage, RHI_ShaderArgument* arguments, I32 arguments_count);

// -------------------------------------------------------------------
// -- Set States And Draw --------------------------------------------
func void RHI_SetViewport(RHI_CommandBuffer command_buffer, RectI32 viewport);
func void RHI_SetScissor(RHI_CommandBuffer command_buffer, RectI32 scissor);
func void RHI_DrawPrimitives(RHI_CommandBuffer command_buffer, U32 vertex_count, U32 instance_count, U32 first_vertex, U32 first_instance);
func void RHI_DrawIndexedPrimitives(RHI_CommandBuffer command_buffer, U32 index_count, U32 instance_count, U32 first_index, I32 vertex_offset, U32 first_instance);
func void RHI_PresentTexture(RHI_CommandBuffer command_buffer, RHI_Texture texture);

// -------------------------------------------------------------------
// -- Device ---------------------------------------------------------
typedef struct RHI_Device RHI_Device;
struct RHI_Device {
	B32 (*Init)(OS_Window* window);
  B32 (*Shutdown)(void);

	// Buffer
	RHI_Buffer (*CreateBuffer)(Str8 label, U32 capacity, RHI_BufferUsageFlags usage_flags, RHI_BufferPropertyFlags property_flags);
	U64 (*PushBuffer)(RHI_Buffer buffer, U8* data, U64 size);
	void (*ResetBuffer)(RHI_Buffer buffer);
  RHI_DeviceAddress (*BufferDeviceAddress)(RHI_Buffer buffer);
	void (*BindIndexBuffer)(RHI_CommandBuffer command_buffer, RHI_Buffer buffer, U64 offset, RHI_IndexSize index_size);
	void (*BindVertexBuffer)(RHI_CommandBuffer command_buffer, RHI_Buffer buffer, U64 offset);

	// Uniform Data
  void (*BindShaderArguments)(RHI_CommandBuffer command_buffer, RHI_ShaderKind stage, RHI_ShaderArgument* arguments, I32 arguments_count);

  // Texture
  RHI_Texture (*CreateTexture)(RHI_TextureCreateInfo* info);
  B32 (*DestroyTexture)(RHI_Texture texture);
  void (*LoadDataToTexture)(U8* data, U64 data_size, RHI_Texture texture);
  void (*CopyTexture)(RHI_CommandBuffer command_buffer, RHI_Texture source, RHI_Texture destination);
  U64 (*CopyTextureToBuffer)(RHI_CommandBuffer command_buffer, RHI_Texture texture, RHI_Buffer buffer);
  void (*CopyBufferToTexture)(RHI_CommandBuffer command_buffer, RHI_Buffer buffer, U64 offset, U64 size, RHI_Texture texture);
  RHI_TextureFormat (*GetTextureFormat)(RHI_Texture texture);
  Vec2I32         (*GetTextureDimension)(RHI_Texture texture);

  RHI_TextureSampler (*CreateTextureSampler)(RHI_TextureSamplerCreateInfo* info);

	// Command Buffer
	RHI_CommandBuffer (*GetCommandBuffer)(void);
	void (*BeginCommandBuffer)(RHI_CommandBuffer command_buffer);
	void (*SubmitCommandBuffer)(RHI_CommandBuffer command_buffer);

	// Swapchain
  RHI_TextureFormat (*GetSwapchainTextureFormat)();
	RHI_Texture (*AcquireSwapchainTexture)(RHI_CommandBuffer command_buffer);

	// Render Pass
	RHI_RenderPass* (*BeginRenderPass)(RHI_CommandBuffer command_buffer, U32 color_targets_count, RHI_ColorTarget* color_targets, RHI_DepthStencilTarget* depth_stencil_target, RHI_Resource* resources, I32 resources_count);
	void (*EndRenderPass)(RHI_CommandBuffer command_buffer, RHI_RenderPass* render_pass);

  // Shader
  RHI_Shader (*CreateShader)(Arena* arena, RHI_ShaderCreateInfo* info);
	
	// Graphics Pipeline
	RHI_GraphicsPipeline (*CreateGraphicsPipeline)(RHI_GraphicsPipelineCreateInfo* info);
	void (*BindGraphicsPipeline)(RHI_CommandBuffer command_buffer, RHI_GraphicsPipeline pipeline);

	// State and Draw
	void (*SetViewport)(RHI_CommandBuffer command_buffer, RectI32 viewport);
	void (*SetScissor)(RHI_CommandBuffer command_buffer, RectI32 scissor);
	void (*DrawPrimitives)(RHI_CommandBuffer command_buffer, U32 vertex_count, U32 instance_count, U32 first_vertex, U32 first_instance);
	void (*DrawIndexedPrimitives)(RHI_CommandBuffer command_buffer, U32 index_count, U32 instance_count, U32 first_index, I32 vertex_offset, U32 first_instance);
  void (*PresentTexture)(RHI_CommandBuffer command_buffer, RHI_Texture texture);
};

#define AssignDeviceFunction(api_name, function_name) _r_state.device.function_name = RHI_##api_name##_##function_name;
#define AssignDeviceFunctions(api_name) \
	AssignDeviceFunction(api_name, Init) \
  AssignDeviceFunction(api_name, Shutdown) \
	AssignDeviceFunction(api_name, CreateBuffer) \
	AssignDeviceFunction(api_name, PushBuffer) \
	AssignDeviceFunction(api_name, ResetBuffer) \
	AssignDeviceFunction(api_name, BufferDeviceAddress) \
	AssignDeviceFunction(api_name, BindIndexBuffer) \
	AssignDeviceFunction(api_name, BindVertexBuffer) \
	AssignDeviceFunction(api_name, BindShaderArguments) \
  AssignDeviceFunction(api_name, CreateTexture) \
  AssignDeviceFunction(api_name, DestroyTexture) \
  AssignDeviceFunction(api_name, LoadDataToTexture) \
  AssignDeviceFunction(api_name, CopyTexture) \
  AssignDeviceFunction(api_name, CopyTextureToBuffer) \
  AssignDeviceFunction(api_name, CopyBufferToTexture) \
  AssignDeviceFunction(api_name, GetTextureFormat) \
  AssignDeviceFunction(api_name, GetTextureDimension) \
  AssignDeviceFunction(api_name, CreateTextureSampler) \
	AssignDeviceFunction(api_name, GetCommandBuffer) \
	AssignDeviceFunction(api_name, BeginCommandBuffer) \
	AssignDeviceFunction(api_name, SubmitCommandBuffer) \
  AssignDeviceFunction(api_name, GetSwapchainTextureFormat) \
	AssignDeviceFunction(api_name, AcquireSwapchainTexture) \
	AssignDeviceFunction(api_name, BeginRenderPass) \
	AssignDeviceFunction(api_name, EndRenderPass) \
  AssignDeviceFunction(api_name, CreateShader) \
	AssignDeviceFunction(api_name, CreateGraphicsPipeline) \
	AssignDeviceFunction(api_name, BindGraphicsPipeline) \
	AssignDeviceFunction(api_name, SetViewport) \
	AssignDeviceFunction(api_name, SetScissor) \
	AssignDeviceFunction(api_name, DrawPrimitives) \
	AssignDeviceFunction(api_name, DrawIndexedPrimitives) \
  AssignDeviceFunction(api_name, PresentTexture) \

// -------------------------------------------------------------------
// -- State ----------------------------------------------------------
typedef U8 RHI_RendererKind;
typedef enum RHI_RendererKindEnum {
  RHI_RendererKind_None,

  RHI_RendererKind_Vulkan,
  RHI_RendererKind_Metal,

  RHI_RendererKind_Count
} RHI_RendererKindEnum;

typedef struct RHI_State RHI_State;
struct RHI_State {
	RHI_Device       device;
	RHI_RendererKind renderer_type;
} _r_state;

func B32 RHI_Init(OS_Window* window);
func B32 RHI_Shutdown();
