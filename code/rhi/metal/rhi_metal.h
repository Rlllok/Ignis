#pragma once

#include <Metal/Metal.h>

#include "../rhi_core.h"

// -------------------------------------------------------------------
// -- Synchronization ------------------------------------------------
typedef struct RHI_Metal_Semaphore RHI_Metal_Semaphore;
struct RHI_Metal_Semaphore {
  id<MTLSharedEvent> event;
  B32                in_use;
};
RHI_Metal_Semaphore RHI_Metal_Semaphore_Nil = ZeroStruct();
DefineArray(RHI_Metal_Semaphore, RHI_Metal_SemaphoreArray, RHI_Metal_Semaphore_Nil);

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
  RHI_Shader*                vertex_shader; // --AlNov: @TODO Not really like the idea of pointer to RHI_Shader
  id<MTL4ArgumentTable>      vertex_argument_table;
  RHI_Shader*                fragment_shader;
  id<MTL4ArgumentTable>      fragment_argument_table;
  id<MTLDepthStencilState>   depth_stencil_state;
};
RHI_Metal_GraphicsPipeline RHI_Metal_GraphicsPipelineNil = ZeroStruct();
DefineArray(RHI_Metal_GraphicsPipeline, RHI_Metal_GraphicsPipelineArray, RHI_Metal_GraphicsPipelineNil)

func RHI_Metal_GraphicsPipeline* RHI_Metal_GraphicsPipelineFromHandle(RHI_GraphicsPipeline handle);

// -------------------------------------------------------------------
// -- Texture --------------------------------------------------------
typedef struct RHI_Metal_Texture RHI_Metal_Texture;
struct RHI_Metal_Texture {
  id<MTLTexture>    mtl;
  Vec3I32           size;
  RHI_TextureFormat format;
};
RHI_Metal_Texture RHI_Metal_TextureNil = ZeroStruct();
DefineArray(RHI_Metal_Texture, RHI_Metal_TextureArray, RHI_Metal_TextureNil);

func RHI_Metal_Texture* RHI_Metal_TextureFromHandle(RHI_Texture handle);

// -------------------------------------------------------------------
// -- Command Buffer -------------------------------------------------
typedef struct RHI_Metal_CommandBuffer RHI_Metal_CommandBuffer;
struct RHI_Metal_CommandBuffer {
  id<MTL4CommandBuffer>        mtl;
  id<MTL4CommandAllocator>     allocator[RHI_FRAMES_IN_FLIGHT];
  id<MTLResidencySet>          residency_set[RHI_FRAMES_IN_FLIGHT];
  id<MTLSharedEvent>           shared_event;
  U64                          event_count;
  id<MTL4RenderCommandEncoder> render_encoder;

  RHI_Metal_GraphicsPipeline* current_graphics_pipeline;

  I32               current_frame;
  MTLViewport       current_viewport;
  RHI_Metal_Buffer* current_index_buffer;
  U64               index_buffer_offset;
  RHI_IndexSize     index_size;
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

func MTLIndexType RHI_Metal_IndexTypeFromRHI(RHI_IndexSize index_size);

func MTLCompareFunction RHI_Metal_CompareFunctionFromRHI(RHI_CompareOperation operation);

// -------------------------------------------------------------------
// -- Global State ---------------------------------------------------
typedef struct RHI_Metal_Context RHI_Metal_Context;
struct RHI_Metal_Context {
  OS_MacOS_Window* window;

  id<MTLDevice>        device;
  id<MTL4CommandQueue> command_queue;
  id<MTLResidencySet>  residency_set;

  MTLPixelFormat drawable_texture_format;
  id<CAMetalDrawable> current_drawable;
  I32 drawable_count;
  I32 current_swapchain_texture_index;

  Arena*                          arena;
  RHI_Metal_CommandBufferArray    command_buffers;
  RHI_Metal_SemaphoreArray        semaphores;
  RHI_Metal_BufferArray           data_buffers;
  RHI_Metal_GraphicsPipelineArray graphics_pipelines;
  RHI_Metal_TextureArray          textures;
} _rhi_metal_context;

