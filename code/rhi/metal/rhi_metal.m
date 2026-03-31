#pragma once

#include <Metal/Metal.h>

#include "../rhi_core.h"

// -------------------------------------------------------------------
// -- Buffer ---------------------------------------------------------
typedef struct RHI_Metal_Buffer RHI_Metal_Buffer;
struct RHI_Metal_Buffer {
  id<MTLBuffer> handle;
};

func RHI_Buffer
RHI_Metal_CreateBuffer(U32 capacity, RHI_BufferUsageFlags usage_flags, RHI_BufferPropertyFlags property_flags) {
  // --AlNov: @TODO
}

func U64
RHI_Metal_PushBuffer(RHI_Buffer buffer, U8* data, U64 size) {
  // --AlNov: @TODO
}

func void
RHI_Metal_ResetBuffer(RHI_Buffer buffer) {
  // --AlNov: @TODO
}

func void
RHI_Metal_BindIndexBuffer(RHI_CommandBuffer command_buffer, RHI_Buffer buffer, U64 offset, RHI_IndexSize index_size) {
  // --AlNov: @TODO
}

func void
RHI_Metal_BindVertexBuffer(RHI_CommandBuffer command_buffer, RHI_Buffer buffer, U64 offset) {
  // --AlNov: @TODO
}

// -------------------------------------------------------------------
// -- Descriptor Set -------------------------------------------------
func void
RHI_MetalBindShaderData(RHI_CommandBuffer command_buffer, RHI_ShaderKind shader_kind, B32 is_global, I32 uniform_buffers_count, RHI_UniformBufferBindingInfo* uniform_infos, I32 sampler_count, RHI_SamplerBindingInfo* sampler_infos) {
  // --AlNov: @TODO
}

// -------------------------------------------------------------------
// -- Texture --------------------------------------------------------
func RHI_Texture
RHI_Metal_CreateTexture(RHI_TextureCreateInfo* info) {
  // --AlNov: @TODO
}

func void
RHI_Metal_DestroyTexture(RHI_Texture texture) {
  // --AlNov: @TODO
}

func void
RHI_Metal_LoadDataToTexture(U8* data, U64 data_size, RHI_Texture texture) {
  // --AlNov: @TODO
}

func void
RHI_Metal_CopyTexture(RHI_CommandBuffer command_buffer, RHI_Texture source, RHI_Texture destination) {
  // --AlNov: @TODO
}

// -------------------------------------------------------------------
// -- Global State ---------------------------------------------------
typedef struct RHI_Metal_Context RHI_Metal_Context;
struct RHI_Metal_Context {
  id<MTLDevice> device;
} _rhi_metal_context;

func B32
RHI_Metal_Init(OS_Window* window) {
  return 0;
}

func B32
RHI_Metal_Shutdown() {
  return 0;
}

func void
RHI_Metal_HadleResize(OS_Window* window) {
  // --AlNov: @TODO
}
