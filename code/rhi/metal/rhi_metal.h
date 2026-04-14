#pragma once

#include <Metal/Metal.h>

#include "../rhi_core.h"

// -------------------------------------------------------------------
// -- Buffer ---------------------------------------------------------
typedef struct RHI_Metal_Buffer RHI_Metal_Buffer;
struct RHI_Metal_Buffer {
  id<MTLBuffer>       handle;
};

// -------------------------------------------------------------------
// -- Texture --------------------------------------------------------
typedef struct RHI_Metal_Texture RHI_Metal_Texture;
struct RHI_Metal_Texture {
  id<MTLTexture> mtl;
};
RHI_Metal_Texture RHI_Metal_TextureNil = ZeroStruct();
DefineArray(RHI_Metal_Texture, RHI_Metal_TextureArray, RHI_Metal_TextureNil);

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
// -- Global State ---------------------------------------------------
typedef struct RHI_Metal_Context RHI_Metal_Context;
struct RHI_Metal_Context {
  OS_MacOS_Window* window;

  id<MTLDevice>       device;
  id<MTLCommandQueue> command_queue;

  id<CAMetalDrawable> current_drawable;
  I32 drawable_count;
  I32 current_swapchain_texture_index;

  Arena*                       arena;
  RHI_Metal_CommandBufferArray command_buffers;
  RHI_Metal_TextureArray       swapchain_textures;
} _rhi_metal_context;

