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
// -- Command Buffer -------------------------------------------------
typedef struct RHI_Metal_CommandBuffer RHI_Metal_CommandBuffer;
struct RHI_Metal_CommandBuffer {
  id<MTLCommandBuffer> mtl;
};
RHI_Metal_CommandBuffer RHI_Metal_CommandBufferNil = ZeroStruct();
DefineArray(RHI_Metal_CommandBuffer, RHI_Metal_CommandBufferArray, RHI_Metal_CommandBufferNil)

func RHI_Metal_CommandBuffer* RHI_Metal_CommandBufferFromHandle(RHI_CommandBuffer handle);

// -------------------------------------------------------------------
// -- Global State ---------------------------------------------------
typedef struct RHI_Metal_Context RHI_Metal_Context;
struct RHI_Metal_Context {
  id<MTLDevice>       device;
  id<MTLCommandQueue> command_queue;

  Arena*                       arena;
  RHI_Metal_CommandBufferArray command_buffers;
} _rhi_metal_context;

