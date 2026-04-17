#pragma once

#include "rhi_metal.h"

// -------------------------------------------------------------------
// -- Buffer ---------------------------------------------------------
func RHI_Metal_Buffer*
RHI_Metal_BufferFromHandle(RHI_Buffer handle) {
  return RHI_Metal_BufferArrayGetPointer(&_rhi_metal_context.data_buffers, handle);
}

func RHI_Buffer
RHI_Metal_CreateBuffer(U32 capacity, RHI_BufferUsageFlags usage_flags, RHI_BufferPropertyFlags property_flags) {
  RHI_Metal_Buffer mtl_buffer = ZeroStruct();

  mtl_buffer.mtl = [_rhi_metal_context.device newBufferWithLength:capacity options:MTLResourceCPUCacheModeDefaultCache];
  mtl_buffer.capacity = capacity;

  return RHI_Metal_BufferArrayAdd(&_rhi_metal_context.data_buffers, mtl_buffer);
}

func U64
RHI_Metal_PushBuffer(RHI_Buffer buffer, U8* data, U64 size) {
  RHI_Metal_Buffer* mtl_buffer = RHI_Metal_BufferFromHandle(buffer);
  
  Assert(mtl_buffer->mtl.contents != nil);

  if (mtl_buffer->position + size > mtl_buffer->capacity) {
    LogDebug("RHI_Metal. Ring buffer reseted to zero\n")
    mtl_buffer->position = 0;
  }

  U64 offset = mtl_buffer->position;

  memcpy((U8*)mtl_buffer->mtl.contents + mtl_buffer->position, data, size);
  mtl_buffer->position += size;
  U64 padding = 64 - (mtl_buffer->position + 64)%64;
  mtl_buffer->position += padding;

  return offset;
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
  RHI_Metal_CommandBuffer* mtl_command_buffer = RHI_Metal_CommandBufferFromHandle(command_buffer);
  RHI_Metal_Buffer* mtl_buffer = RHI_Metal_BufferFromHandle(buffer);

  [mtl_command_buffer->render_encoder setVertexBuffer:mtl_buffer->mtl offset:offset atIndex:0];
}

// -------------------------------------------------------------------
// -- Descriptor Set -------------------------------------------------
func void
RHI_Metal_BindShaderData(RHI_CommandBuffer command_buffer, RHI_ShaderKind shader_kind, B32 is_global, I32 uniform_buffers_count, RHI_UniformBufferBindingInfo* uniform_infos, I32 sampler_count, RHI_SamplerBindingInfo* sampler_infos) {
  // --AlNov: @TODO
}

// -------------------------------------------------------------------
// -- Texture --------------------------------------------------------
func RHI_Metal_Texture*
RHI_Metal_TextureFromHandle(RHI_Texture handle) {
  return RHI_Metal_TextureArrayGetPointer(&_rhi_metal_context.swapchain_textures, handle);
}

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
  @autoreleasepool {
    RHI_Metal_CommandBuffer* metal_command_buffer = RHI_Metal_CommandBufferFromHandle(command_buffer);
    metal_command_buffer->mtl = [_rhi_metal_context.command_queue commandBuffer];
  }
}

func void
RHI_Metal_SubmitCommandBuffer(RHI_CommandBuffer command_buffer) {
  @autoreleasepool {
    RHI_Metal_CommandBuffer* metal_command_buffer = RHI_Metal_CommandBufferFromHandle(command_buffer);
    [metal_command_buffer->mtl presentDrawable: _rhi_metal_context.current_drawable];
    [metal_command_buffer->mtl commit];
  }
}

// -------------------------------------------------------------------
// -- Swapchain ------------------------------------------------------

func RHI_TextureFormat
RHI_Metal_GetSwapchainTextureFormat() {
  return RHI_TextureFormatFromMetal(_rhi_metal_context.drawable_texture_format);
}

func RHI_Texture
RHI_Metal_AcquireSwapchainTexture(RHI_CommandBuffer command_buffer) {
  @autoreleasepool {
    RHI_Texture result = _rhi_metal_context.current_swapchain_texture_index;

    OS_MacOS_View* ns_view = (__bridge OS_MacOS_View*)_rhi_metal_context.window->ns_view;
    _rhi_metal_context.current_drawable = [[ns_view MetalLayer] nextDrawable];

    RHI_Metal_Texture* metal_texture = RHI_Metal_TextureArrayGetPointer(&_rhi_metal_context.swapchain_textures, result);
    metal_texture->mtl = [_rhi_metal_context.current_drawable texture];

    _rhi_metal_context.current_swapchain_texture_index = (_rhi_metal_context.current_swapchain_texture_index + 1)%_rhi_metal_context.drawable_count;

    return result;
  }
}

// -------------------------------------------------------------------
// -- Render Pass ----------------------------------------------------
func RHI_RenderPass*
RHI_Metal_BeginRenderPass(RHI_CommandBuffer command_buffer, U32 color_targets_count, RHI_ColorTarget* color_targets, RHI_DepthStencilTarget* depth_stencil_target) {
  @autoreleasepool {
    RHI_Metal_CommandBuffer* mtl_command_buffer = RHI_Metal_CommandBufferFromHandle(command_buffer);
    
    Assert(mtl_command_buffer->render_encoder == nil);

    Assert(_rhi_metal_context.current_drawable != nil);

    MTLRenderPassDescriptor* pass_descriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    for (I32 i = 0; i < color_targets_count; i += 1) {
      RHI_Metal_Texture* texture = RHI_Metal_TextureFromHandle(color_targets[i].texture);
      MTLLoadAction load_action = RHI_Metal_LoadActionFromRHI(color_targets[i].load_operation);
      MTLClearColor clear_color = MTLClearColorMake(color_targets[i].clear_color.r, color_targets[i].clear_color.g, color_targets[i].clear_color.b, color_targets[i].clear_color.a);
      MTLStoreAction store_action = RHI_Metal_StoreActionFromRHI(color_targets[i].store_operation);

      pass_descriptor.colorAttachments[i].texture = texture->mtl;
      pass_descriptor.colorAttachments[i].loadAction = load_action;
      pass_descriptor.colorAttachments[i].clearColor = clear_color;
      pass_descriptor.colorAttachments[i].storeAction = store_action;
    }
    mtl_command_buffer->render_encoder = [mtl_command_buffer->mtl renderCommandEncoderWithDescriptor:pass_descriptor];
    [mtl_command_buffer->render_encoder setCullMode:MTLCullModeNone];

    return 0;
  }
}

func void
RHI_Metal_EndRenderPass(RHI_CommandBuffer command_buffer, RHI_RenderPass* render_pass) {
  @autoreleasepool {
    RHI_Metal_CommandBuffer* mtl_command_buffer = RHI_Metal_CommandBufferFromHandle(command_buffer);
    
    Assert(mtl_command_buffer->render_encoder != nil);

    [mtl_command_buffer->render_encoder endEncoding];

    mtl_command_buffer->render_encoder = nil;
  }
}

// -------------------------------------------------------------------
// -- Pipeline -------------------------------------------------------
func RHI_Metal_GraphicsPipeline*
RHI_Metal_GraphicsPipelineFromHandle(RHI_GraphicsPipeline handle) {
  return RHI_Metal_GraphicsPipelineArrayGetPointer(&_rhi_metal_context.graphics_pipelines, handle);
}

const char* rhi_metal_shader_source = R"D4LIN4R(
using namespace metal;

struct Vertex {
  float4 position;
};

struct VertexOut {
  float4 position [[position]];
  float4 color;
};

vertex VertexOut vertex_main(const device Vertex* vertex_buffer[[buffer(0)]], uint vid [[vertex_id]]) {
  const float3 colors[3] = {
      float3(1.0, 0.0, 0.0), // Red
      float3(0.0, 1.0, 0.0), // Green
      float3(0.0, 0.0, 1.0)  // Blue
  };

  VertexOut out = {0};
  out.position = vertex_buffer[vid].position;
  out.color = float4(colors[vid], 1.0);
  return out;
}

fragment float4 fragment_main(VertexOut in [[stage_in]]) {
  return in.color;
}
)D4LIN4R";

func RHI_GraphicsPipeline
RHI_Metal_CreateGraphicsPipeline(RHI_GraphicsPipelineCreateInfo* info) {
  @autoreleasepool {
    RHI_Metal_GraphicsPipeline pipeline = ZeroStruct();

    NSString* source = [[NSString alloc] initWithUTF8String:rhi_metal_shader_source];
    MTLCompileOptions* compile_options = [[MTLCompileOptions alloc] init];
    compile_options.languageVersion = MTLLanguageVersion2_0;

    NSError* err = nil;
    id<MTLLibrary> library = [_rhi_metal_context.device newLibraryWithSource:source options:compile_options error:&err];

    if (err) {
      NSLog(@"%@", err);
      Assert(1);
    }

    id<MTLFunction> vertex_function = [library newFunctionWithName:@"vertex_main"];
    id<MTLFunction> fragment_function = [library newFunctionWithName:@"fragment_main"];

    MTLRenderPipelineDescriptor* pipeline_descriptor = [MTLRenderPipelineDescriptor new];
    pipeline_descriptor.vertexFunction = vertex_function;
    pipeline_descriptor.fragmentFunction = fragment_function;
    for (I32 i = 0; i < info->color_targets_count; i += 1) {
      pipeline_descriptor.colorAttachments[0].pixelFormat = RHI_Metal_PixelFormatFromRHI(info->color_target_infos[i].format);
    }

    pipeline.mtl = [_rhi_metal_context.device newRenderPipelineStateWithDescriptor:pipeline_descriptor error:0];

    return RHI_Metal_GraphicsPipelineArrayAdd(&_rhi_metal_context.graphics_pipelines, pipeline);
  }
}

func void
RHI_Metal_BindGraphicsPipeline(RHI_CommandBuffer command_buffer, RHI_GraphicsPipeline pipeline) {
  RHI_Metal_CommandBuffer* mtl_command_buffer = RHI_Metal_CommandBufferFromHandle(command_buffer);
  RHI_Metal_GraphicsPipeline* mtl_pipeline = RHI_Metal_GraphicsPipelineFromHandle(pipeline);

  Assert(mtl_command_buffer->render_encoder != nil);
  
  [mtl_command_buffer->render_encoder setRenderPipelineState:mtl_pipeline->mtl];
}

// -------------------------------------------------------------------
// -- Set States And Draw --------------------------------------------
func void
RHI_Metal_SetViewport(RHI_CommandBuffer command_buffer, RectI32 viewport) {
  RHI_Metal_CommandBuffer* mtl_command_buffer = RHI_Metal_CommandBufferFromHandle(command_buffer);

  Assert(mtl_command_buffer->render_encoder != nil);

  MTLViewport mtl_viewport = {
    .originX = viewport.x,
    .originY = viewport.y,
    .width = viewport.w,
    .height = viewport.h,
    .znear = 0.0f,
    .zfar = 10.0f,
  };
  [mtl_command_buffer->render_encoder setViewport:mtl_viewport];
}

func void
RHI_Metal_SetScissor(RHI_CommandBuffer command_buffer, RectI32 scissor) {
  // --AlNov: @TODO
}

func void
RHI_Metal_DrawPrimitives(RHI_CommandBuffer command_buffer, U32 vertex_count, U32 instance_count, U32 first_vertex, U32 first_instance) {
  RHI_Metal_CommandBuffer* mtl_command_buffer = RHI_Metal_CommandBufferFromHandle(command_buffer);

  Assert(mtl_command_buffer->render_encoder);
  
  [mtl_command_buffer->render_encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount: 3];
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
// -- Utils ----------------------------------------------------------
func MTLLoadAction
RHI_Metal_LoadActionFromRHI(RHI_LoadOperation operation) {
  MTLLoadAction result = MTLLoadActionDontCare;
  switch (operation) {
    default: Assert(1); break;

    case RHI_AttachmentLoadOperation_DontCare: result = MTLLoadActionDontCare; break;
    case RHI_AttachmentLoadOperation_Load:     result = MTLLoadActionLoad;     break;
    case RHI_AttachmentLoadOperation_Clear:    result = MTLLoadActionClear;    break;
  }
  return result;
}

func MTLStoreAction
RHI_Metal_StoreActionFromRHI(RHI_StoreOperation operation) {
  MTLStoreAction result = MTLStoreActionDontCare;
  switch (operation) {
    default: Assert(1); break;

    case RHI_AttachmentStoreOperation_DontCare: result = MTLStoreActionDontCare; break;
    case RHI_AttachmentStoreOperation_Store:    result = MTLStoreActionStore;    break;
  }
  return result;
}

func MTLPixelFormat
RHI_Metal_PixelFormatFromRHI(RHI_TextureFormat format) {
  MTLPixelFormat result = 0;
  switch (format) {
    default: Assert(1); break;

    case RHI_TextureFormat_None:                result = MTLPixelFormatInvalid;         break;
    case RHI_TextureFormat_R8G8B8A8_SRGB:       result = MTLPixelFormatRGBA8Unorm_sRGB; break;
    case RHI_TextureFormat_R8G8B8A8_UNORM:      result = MTLPixelFormatRGBA8Unorm;      break;
    case RHI_TextureFormat_B8G8R8A8_UNORM:      result = MTLPixelFormatBGRA8Unorm;      break;
    case RHI_TextureFormat_R16G16B16A16_SFLOAT: result = MTLPixelFormatRGBA16Float;     break;
    case RHI_TextureFormat_D16_UNORM:           result = MTLPixelFormatDepth16Unorm;    break;
    case RHI_TextureFormat_R16_UINT:            result = MTLPixelFormatR16Uint;         break;
  }
  return result;
}

func RHI_TextureFormat
RHI_TextureFormatFromMetal(MTLPixelFormat format) {
  RHI_TextureFormat result = RHI_TextureFormat_None;
  switch (format) {
    default: Assert(1); break;

    case MTLPixelFormatInvalid:         result = RHI_TextureFormat_None;                break;
    case MTLPixelFormatRGBA8Unorm_sRGB: result = RHI_TextureFormat_R8G8B8A8_SRGB;       break;
    case MTLPixelFormatRGBA8Unorm:      result = RHI_TextureFormat_R8G8B8A8_UNORM;      break;
    case MTLPixelFormatBGRA8Unorm:      result = RHI_TextureFormat_B8G8R8A8_UNORM;      break;
    case MTLPixelFormatRGBA16Float:     result = RHI_TextureFormat_R16G16B16A16_SFLOAT; break;
    case MTLPixelFormatDepth16Unorm:    result = RHI_TextureFormat_D16_UNORM;           break;
    case MTLPixelFormatR16Uint:         result = RHI_TextureFormat_R16_UINT;            break;
  }
  return result;
}

// -------------------------------------------------------------------
// -- Global State ---------------------------------------------------
func B32
RHI_Metal_Init(OS_Window* window) {
  _rhi_metal_context.window = (OS_MacOS_Window*)window;

  _rhi_metal_context.arena = AllocateArena(Gigabytes(32), Kilobytes(64));
  _rhi_metal_context.command_buffers = RHI_Metal_CommandBufferArrayAllocate(_rhi_metal_context.arena, 32);
  _rhi_metal_context.data_buffers = RHI_Metal_BufferArrayAllocate(_rhi_metal_context.arena, 32);
  _rhi_metal_context.graphics_pipelines = RHI_Metal_GraphicsPipelineArrayAllocate(_rhi_metal_context.arena, 32);

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

  _rhi_metal_context.drawable_texture_format = MTLPixelFormatBGRA8Unorm;

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
