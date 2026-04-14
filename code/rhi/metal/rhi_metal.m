#pragma once

#include "rhi_metal.h"

// -------------------------------------------------------------------
// -- Buffer ---------------------------------------------------------
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
RHI_Metal_BindShaderData(RHI_CommandBuffer command_buffer, RHI_ShaderKind shader_kind, B32 is_global, I32 uniform_buffers_count, RHI_UniformBufferBindingInfo* uniform_infos, I32 sampler_count, RHI_SamplerBindingInfo* sampler_infos) {
  // --AlNov: @TODO
}

// -------------------------------------------------------------------
// -- Texture --------------------------------------------------------
func RHI_Texture
RHI_Metal_CreateTexture(RHI_TextureCreateInfo* info) {
  // --AlNov: @TODO
}

func B32
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

func U64
RHI_Metal_CopyTextureToBuffer(RHI_CommandBuffer command_buffer, RHI_Texture texture, RHI_Buffer buffer) {
  // --AlNov: @TODO
}

func void
RHI_Metal_CopyBufferToTexture(RHI_CommandBuffer command_buffer, RHI_Buffer buffer, U64 offset, U64 size, RHI_Texture texture) {
  // --AlNov: @TODO
}

func RHI_TextureFormat
RHI_Metal_GetTextureFormat(RHI_Texture texture) {
  // --AlNov: @TODO
}

func Vec2I32
RHI_Metal_GetTextureDimension(RHI_Texture texture) {
  // --AlNov: @TODO
}

func RHI_TextureSampler
RHI_Metal_CreateTextureSampler(RHI_TextureSamplerCreateInfo* info) {
  // --AlNov: @TODO
}

// -------------------------------------------------------------------
// -- Command Buffer -------------------------------------------------

func RHI_Metal_CommandBuffer*
RHI_Metal_CommandBufferFromHandle(RHI_CommandBuffer handle) {
  return RHI_Metal_CommandBufferArrayGetPointer(&_rhi_metal_context.command_buffers, handle);
}

func RHI_CommandBuffer
RHI_Metal_GetCommandBuffer() {
  RHI_Metal_CommandBuffer result = ZeroStruct();

  return RHI_Metal_CommandBufferArrayAdd(&_rhi_metal_context.command_buffers, result);
}

func void
RHI_Metal_BeginCommandBuffer(RHI_CommandBuffer command_buffer) {
  RHI_Metal_CommandBuffer* metal_command_buffer = RHI_Metal_CommandBufferFromHandle(command_buffer);
  metal_command_buffer->mtl = [_rhi_metal_context.command_queue commandBuffer];
}

func void
RHI_Metal_SubmitCommandBuffer(RHI_CommandBuffer command_buffer) {
  RHI_Metal_CommandBuffer* metal_command_buffer = RHI_Metal_CommandBufferFromHandle(command_buffer);
  [metal_command_buffer->mtl presentDrawable: _rhi_metal_context.current_drawable];
  [metal_command_buffer->mtl commit];
}

// -------------------------------------------------------------------
// -- Swapchain ------------------------------------------------------

func RHI_TextureFormat
RHI_Metal_GetSwapchainTextureFormat() {
  // --AlNov: @TODO¡
}

func RHI_Texture
RHI_Metal_AcquireSwapchainTexture(RHI_CommandBuffer command_buffer) {
    RHI_Texture result = _rhi_metal_context.current_swapchain_texture_index;

    OS_MacOS_View* ns_view = (__bridge OS_MacOS_View*)_rhi_metal_context.window->ns_view;
    _rhi_metal_context.current_drawable = [[ns_view MetalLayer] nextDrawable];

    RHI_Metal_Texture* metal_texture = RHI_Metal_TextureArrayGetPointer(&_rhi_metal_context.swapchain_textures, result);
    metal_texture->mtl = [_rhi_metal_context.current_drawable texture];

    _rhi_metal_context.current_swapchain_texture_index = (_rhi_metal_context.current_swapchain_texture_index + 1)%_rhi_metal_context.drawable_count;
}

// -------------------------------------------------------------------
// -- Render Pass ----------------------------------------------------
func RHI_RenderPass*
RHI_Metal_BeginRenderPass(RHI_CommandBuffer command_buffer, U32 color_targets_count, RHI_ColorTarget* color_targets, RHI_DepthStencilTarget* depth_stencil_target) {
    RHI_Metal_CommandBuffer* mtl_command_buffer = RHI_Metal_CommandBufferFromHandle(command_buffer);
    
    Assert(mtl_command_buffer->render_encoder == nil);

    Assert(_rhi_metal_context.current_drawable != nil);

    MTLRenderPassDescriptor* pass_descriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    pass_descriptor.colorAttachments[0].texture = [_rhi_metal_context.current_drawable texture];
    pass_descriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
    pass_descriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.8f, 0.2f, 0.3f, 1.0f);
    pass_descriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
    mtl_command_buffer->render_encoder = [mtl_command_buffer->mtl renderCommandEncoderWithDescriptor:pass_descriptor];

    return 0;
}

func void
RHI_Metal_EndRenderPass(RHI_CommandBuffer command_buffer, RHI_RenderPass* render_pass) {
  // --AlNov: @TODO
  RHI_Metal_CommandBuffer* mtl_command_buffer = RHI_Metal_CommandBufferFromHandle(command_buffer);
  
  Assert(mtl_command_buffer->render_encoder != nil);

  [mtl_command_buffer->render_encoder endEncoding];

  mtl_command_buffer->render_encoder = nil;
}

// -------------------------------------------------------------------
// -- Pipeline -------------------------------------------------------

func RHI_GraphicsPipeline
RHI_Metal_CreateGraphicsPipeline(RHI_GraphicsPipelineCreateInfo* info) {
  // --AlNov: @TODO
}

func void
RHI_Metal_BindGraphicsPipeline(RHI_CommandBuffer command_buffer, RHI_GraphicsPipeline pipeline) {
  // --AlNov: @TODO
}

// -------------------------------------------------------------------
// -- Set States And Draw --------------------------------------------
func void
RHI_Metal_SetViewport(RHI_CommandBuffer command_buffer, RectI32 viewport) {
  // --AlNov: @TODO
}

func void
RHI_Metal_SetScissor(RHI_CommandBuffer command_buffer, RectI32 scissor) {
  // --AlNov: @TODO
}

func void
RHI_Metal_DrawPrimitives(RHI_CommandBuffer command_buffer, U32 vertex_count, U32 instance_count, U32 first_vertex, U32 first_instance) {
  // --AlNov: @TODO
}

func void
RHI_Metal_DrawIndexedPrimitives(RHI_CommandBuffer command_buffer, U32 index_count, U32 instance_count, U32 first_index, I32 vertex_offset, U32 first_instance) {
  // --AlNov: @TODO
}

func void
RHI_Metal_PresentTexture(RHI_CommandBuffer command_buffer, RHI_Texture texture) {
  // --AlNov: @TODO
}

// -------------------------------------------------------------------
// -- Global State ---------------------------------------------------
func B32
RHI_Metal_Init(OS_Window* window) {
  _rhi_metal_context.window = (OS_MacOS_Window*)window;

  _rhi_metal_context.arena = AllocateArena(Gigabytes(32), Kilobytes(64));
  _rhi_metal_context.command_buffers = RHI_Metal_CommandBufferArrayAllocate(_rhi_metal_context.arena, 32);

  _rhi_metal_context.device = MTLCreateSystemDefaultDevice();
  Assert(_rhi_metal_context.device != nil);
  LogInfo("Metal. Device name: %s\n", [[_rhi_metal_context.device name]UTF8String]);

  _rhi_metal_context.command_queue = [_rhi_metal_context.device newCommandQueue];

  OS_MacOS_Window* macos_window = (OS_MacOS_Window*)window;
  OS_MacOS_View* view = (__bridge OS_MacOS_View*)macos_window->ns_view;
  Assert(view != nil);

  CAMetalLayer* metal_layer = [view MetalLayer];
  metal_layer.device = _rhi_metal_context.device;
  metal_layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
  metal_layer.framebufferOnly = YES;
  metal_layer.drawableSize = NSSizeToCGSize([view bounds].size);

  _rhi_metal_context.drawable_count = [metal_layer maximumDrawableCount];
  _rhi_metal_context.swapchain_textures = RHI_Metal_TextureArrayAllocate(_rhi_metal_context.arena, _rhi_metal_context.drawable_count);
  _rhi_metal_context.swapchain_textures.length = _rhi_metal_context.drawable_count;

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
