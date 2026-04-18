#pragma once

#include <Metal/Metal.h>

#include "../rhi_core.h"

// -------------------------------------------------------------------
// -- Buffer ---------------------------------------------------------
typedef struct RHI_Metal_Buffer RHI_Metal_Buffer;
struct RHI_Metal_Buffer {
  id<MTLBuffer> mtl;
  U64           position;
  U64           capacity;
};
RHI_Metal_Buffer RHI_Metal_BufferNil = ZeroStruct();
DefineArray(RHI_Metal_Buffer, RHI_Metal_BufferArray, RHI_Metal_BufferNil)

func RHI_Metal_Buffer* RHI_Metal_BufferFromHandle(RHI_Buffer handle);

// -------------------------------------------------------------------
// -- Pipeline -------------------------------------------------------
typedef struct RHI_Metal_GraphicsPipeline RHI_Metal_GraphicsPipeline;
struct RHI_Metal_GraphicsPipeline {
  id<MTLRenderPipelineState> mtl;
};
RHI_Metal_GraphicsPipeline RHI_Metal_GraphicsPipelineNil = ZeroStruct();
DefineArray(RHI_Metal_GraphicsPipeline, RHI_Metal_GraphicsPipelineArray, RHI_Metal_GraphicsPipelineNil)

func RHI_Metal_GraphicsPipeline* RHI_Metal_GraphicsPipelineFromHandle(RHI_GraphicsPipeline handle);

// -------------------------------------------------------------------
// -- Texture --------------------------------------------------------
typedef struct RHI_Metal_Texture RHI_Metal_Texture;
struct RHI_Metal_Texture {
  id<MTLTexture> mtl;
};
RHI_Metal_Texture RHI_Metal_TextureNil = ZeroStruct();
DefineArray(RHI_Metal_Texture, RHI_Metal_TextureArray, RHI_Metal_TextureNil);

func RHI_Metal_Texture* RHI_Metal_TextureFromHandle(RHI_Texture handle);

// -------------------------------------------------------------------
// -- Command Buffer -------------------------------------------------
typedef struct RHI_Metal_CommandBuffer RHI_Metal_CommandBuffer;
struct RHI_Metal_CommandBuffer {
  id<MTLCommandBuffer> mtl;
  id<MTLRenderCommandEncoder> render_encoder;
};
RHI_Metal_CommandBuffer RHI_Metal_CommandBufferNil = ZeroStruct();
DefineArray(RHI_Metal_CommandBuffer, RHI_Metal_CommandBufferArray, RHI_Metal_CommandBufferNil)

func RHI_Metal_CommandBuffer* RHI_Metal_CommandBufferFromHandle(RHI_CommandBuffer handle);

// -------------------------------------------------------------------
// -- Utils ----------------------------------------------------------
func MTLLoadAction  RHI_Metal_LoadActionFromRHI(RHI_LoadOperation operation);
func MTLStoreAction RHI_Metal_StoreActionFromRHI(RHI_StoreOperation operation);

func MTLPixelFormat    RHI_Metal_PixelFormatFromRHI(RHI_TextureFormat format);
func RHI_TextureFormat RHI_TextureFormatFromMetal(MTLPixelFormat format);

func MTLVertexFormat RHI_Metal_VertexFormatFromRHI(RHI_VertexAttributeFormat format);

// -------------------------------------------------------------------
// -- Global State ---------------------------------------------------
typedef struct RHI_Metal_Context RHI_Metal_Context;
struct RHI_Metal_Context {
  OS_MacOS_Window* window;

  id<MTLDevice>       device;
  id<MTLCommandQueue> command_queue;

  MTLPixelFormat drawable_texture_format;
  id<CAMetalDrawable> current_drawable;
  I32 drawable_count;
  I32 current_swapchain_texture_index;

  Arena*                          arena;
  RHI_Metal_CommandBufferArray    command_buffers;
  RHI_Metal_BufferArray           data_buffers;
  RHI_Metal_GraphicsPipelineArray graphics_pipelines;
  RHI_Metal_TextureArray          swapchain_textures;
} _rhi_metal_context;

