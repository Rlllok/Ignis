#pragma once

#include "rhi_metal.h"

// -------------------------------------------------------------------
// -- Synchronization ------------------------------------------------
func RHI_Metal_Semaphore*
RHI_Metal_SemaphoreFromHandle(RHI_Semaphore semaphore) {
  return RHI_Metal_SemaphoreArrayGetPointer(&_rhi_metal_context.semaphores, semaphore);
}

func RHI_Semaphore
RHI_Metal_CreateSemaphore() {
  @autoreleasepool {
    RHI_Semaphore result = 0;

    for (I32 semaphore_index = 1; semaphore_index < _rhi_metal_context.semaphores.length; semaphore_index += 1) {
      RHI_Metal_Semaphore* mtl_semaphore = RHI_Metal_SemaphoreArrayGetPointer(&_rhi_metal_context.semaphores, semaphore_index);

      if (!mtl_semaphore->in_use) {
        mtl_semaphore->event = [_rhi_metal_context.device newSharedEvent];
        mtl_semaphore->in_use = 1;
        result = semaphore_index;
        break;
      }
    }

    Assert(result != 0);
    return result;
  }
}

func void
RHI_Metal_DestroySemaphore(RHI_Semaphore semaphore) {
  @autoreleasepool {
    RHI_Metal_Semaphore* mtl_semaphore = RHI_Metal_SemaphoreArrayGetPointer(&_rhi_metal_context.semaphores, semaphore);
    mtl_semaphore->event = 0;
    mtl_semaphore->in_use = 0;
  }
}

func void
RHI_Metal_WaitSemaphore(RHI_Semaphore semaphore, U64 value) {
  @autoreleasepool {
    RHI_Metal_Semaphore* mtl_semaphore = RHI_Metal_SemaphoreArrayGetPointer(&_rhi_metal_context.semaphores, semaphore);
    [mtl_semaphore->event waitUntilSignaledValue:value timeoutMS:U64_MAX];
  }
}

// -------------------------------------------------------------------
// -- Buffer ---------------------------------------------------------
func RHI_Metal_Buffer*
RHI_Metal_BufferFromHandle(RHI_Buffer handle) {
  return RHI_Metal_BufferArrayGetPointer(&_rhi_metal_context.data_buffers, handle);
}

func RHI_Buffer
RHI_Metal_CreateBuffer(Str8 label, U32 capacity, RHI_BufferUsageFlags usage_flags, RHI_BufferPropertyFlags property_flags) {
  @autoreleasepool {
    RHI_Metal_Buffer mtl_buffer = ZeroStruct();

    mtl_buffer.mtl = [_rhi_metal_context.device newBufferWithLength:capacity options:MTLResourceCPUCacheModeDefaultCache];
    mtl_buffer.capacity = capacity;
    
    mtl_buffer.mtl.label = [NSString stringWithUTF8String:CFromStr8(label)];

    [_rhi_metal_context.residency_set addAllocation:mtl_buffer.mtl];
    [_rhi_metal_context.residency_set commit];

    return RHI_Metal_BufferArrayAdd(&_rhi_metal_context.data_buffers, mtl_buffer);
  }
}

func U64
RHI_Metal_PushBuffer(RHI_Buffer buffer, U8* data, U64 size) {
  RHI_Metal_Buffer* mtl_buffer = RHI_Metal_BufferFromHandle(buffer);
  
  Assert(mtl_buffer->mtl.contents != nil);
  Assert(mtl_buffer->capacity >= size);

  if (mtl_buffer->position + size > mtl_buffer->capacity) {
    LogDebug("RHI_Metal. Ring® buffer reseted to zero\n")
    mtl_buffer->position = 0;
  }

  U64 offset = mtl_buffer->position;

  memcpy((U8*)mtl_buffer->mtl.contents + mtl_buffer->position, data, size);
  mtl_buffer->position += size;
  U64 alignment = 16;
  U64 padding = alignment - (mtl_buffer->position + alignment)%alignment;
  if (alignment == 0) padding = 0;
  mtl_buffer->position += padding;

  return offset;
}

func void
RHI_Metal_ResetBuffer(RHI_Buffer buffer) {
  // --AlNov: @TODO
}

func RHI_DeviceAddress
RHI_Metal_BufferDeviceAddress(RHI_Buffer buffer) {
  RHI_Metal_Buffer* mtl_buffer = RHI_Metal_BufferFromHandle(buffer);
  return mtl_buffer->mtl.gpuAddress;
}

func void
RHI_Metal_BindIndexBuffer(RHI_CommandBuffer command_buffer, RHI_Buffer buffer, U64 offset, RHI_IndexSize index_size) {
  RHI_Metal_CommandBuffer* mtl_command_buffer = RHI_Metal_CommandBufferFromHandle(command_buffer);
  RHI_Metal_Buffer* mtl_buffer = RHI_Metal_BufferFromHandle(buffer);

  mtl_command_buffer->current_index_buffer = mtl_buffer;
  mtl_command_buffer->index_buffer_offset = offset;
  mtl_command_buffer->index_size = index_size;
}

func void
RHI_Metal_BindVertexBuffer(RHI_CommandBuffer command_buffer, RHI_Buffer buffer, U64 offset) {
  @autoreleasepool {
    RHI_Metal_CommandBuffer* mtl_command_buffer = RHI_Metal_CommandBufferFromHandle(command_buffer);
    RHI_Metal_GraphicsPipeline* mtl_pipeline = mtl_command_buffer->current_graphics_pipeline;
    RHI_Metal_Buffer* mtl_buffer = RHI_Metal_BufferFromHandle(buffer);

    [mtl_pipeline->vertex_argument_table setAddress:(mtl_buffer->mtl.gpuAddress + offset) atIndex:0];
  }
}

// -------------------------------------------------------------------
// -- Descriptor Set -------------------------------------------------
func void
RHI_Metal_BindShaderArguments(RHI_CommandBuffer command_buffer, RHI_ShaderKind stage, RHI_ShaderArgument* arguments, I32 arguments_count) {
  RHI_Metal_CommandBuffer* mtl_command_buffer = RHI_Metal_CommandBufferFromHandle(command_buffer);
  RHI_Metal_GraphicsPipeline* mtl_pipeline = mtl_command_buffer->current_graphics_pipeline;

  Assert(mtl_pipeline != 0);

  if ((stage & RHI_ShaderKind_Vertex) == RHI_ShaderKind_Vertex) {
    for (I32 argument_index = 0; argument_index < arguments_count; argument_index += 1) {
      RHI_ShaderArgument* argument = arguments + argument_index;
      switch (argument->kind) {
        case RHI_ShaderArgumentKind_BufferAddress: {
          RHI_DeviceAddress address = arguments[argument_index].address;
          [mtl_pipeline->vertex_argument_table setAddress:arguments[argument_index].address atIndex:argument_index + 1]; // --AlNov: @NOTE Buffer with index 0 is reserved as vertex buffer
        } break;

        case RHI_ShaderArgumentKind_ArrayOfTextures: {
          for (I32 texture_index = 0; texture_index < argument->textures.count; texture_index += 1) {
            RHI_Metal_Texture* texture = RHI_Metal_TextureFromHandle(argument->textures.array[texture_index]);
            [mtl_pipeline->vertex_argument_table setTexture:texture->mtl.gpuResourceID atIndex:texture_index];
          };
        } break;
      };
    }
    [mtl_command_buffer->render_encoder setArgumentTable:mtl_pipeline->vertex_argument_table atStages:MTLRenderStageVertex];
  }
  if ((stage & RHI_ShaderKind_Fragment) == RHI_ShaderKind_Fragment) {
    for (I32 argument_index = 0; argument_index < arguments_count; argument_index += 1) {
      RHI_ShaderArgument* argument = arguments + argument_index;
      switch (argument->kind) {
        case RHI_ShaderArgumentKind_BufferAddress: {
          RHI_DeviceAddress address = arguments[argument_index].address;
          [mtl_pipeline->fragment_argument_table setAddress:arguments[argument_index].address atIndex:argument_index + 1]; // --AlNov: @NOTE Buffer with index 0 is reserved as fragment buffer
        } break;

        case RHI_ShaderArgumentKind_ArrayOfTextures: {
          for (I32 texture_index = 0; texture_index < argument->textures.count; texture_index += 1) {
            RHI_Metal_Texture* texture = RHI_Metal_TextureFromHandle(argument->textures.array[texture_index]);
            [mtl_pipeline->fragment_argument_table setTexture:texture->mtl.gpuResourceID atIndex:texture_index];
          };
        } break;
      };
    }
    [mtl_command_buffer->render_encoder setArgumentTable:mtl_pipeline->fragment_argument_table atStages:MTLRenderStageFragment];
  }
}

// -------------------------------------------------------------------
// -- Texture --------------------------------------------------------
func RHI_Metal_Texture*
RHI_Metal_TextureFromHandle(RHI_Texture handle) {
  return RHI_Metal_TextureArrayGetPointer(&_rhi_metal_context.textures, handle);
}

func RHI_Texture
RHI_Metal_CreateTexture(RHI_TextureCreateInfo* info) {
  @autoreleasepool {
    RHI_Metal_Texture result = ZeroStruct();

    MTLTextureDescriptor* descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:RHI_Metal_PixelFormatFromRHI(info->format) width:info->width height:info->height mipmapped:NO];
    if (info->usage_flags & RHI_TEXTURE_USAGE_FLAG_SAMPLED) {
      descriptor.usage = MTLTextureUsageShaderRead;
    } else if (info->usage_flags & RHI_TEXTURE_USAGE_FLAG_DEPTH_STENCIL_ATTACHMENT) {
      descriptor.usage = MTLTextureUsageRenderTarget;
    }
    descriptor.storageMode = MTLStorageModeShared;
    result.mtl = [_rhi_metal_context.device newTextureWithDescriptor:descriptor];
    result.size = MakeVec3I32(info->width, info->height, info->depth);
    result.format = info->format;

    [_rhi_metal_context.residency_set addAllocation:result.mtl];
    [_rhi_metal_context.residency_set commit];

    return RHI_Metal_TextureArrayAdd(&_rhi_metal_context.textures, result);
  }
}

func B32
RHI_Metal_DestroyTexture(RHI_Texture texture) {
  // --AlNov: @TODO
  B32 result = 0;
  return result;
}

func RHI_TextureDeviceId
RHI_Metal_GetTextureDeviceId(RHI_Texture texture) {
  RHI_Metal_Texture* mtl_texture = RHI_Metal_TextureFromHandle(texture);
  return mtl_texture->mtl.gpuResourceID._impl;
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
  U64 result = 0;
  return result;
}

func void
RHI_Metal_CopyBufferToTexture(RHI_CommandBuffer command_buffer, RHI_Buffer buffer, U64 offset, RHI_Texture texture) {
  @autoreleasepool {
    RHI_Metal_CommandBuffer* mtl_command_buffer = RHI_Metal_CommandBufferFromHandle(command_buffer);
    RHI_Metal_Buffer* mtl_buffer = RHI_Metal_BufferFromHandle(buffer);
    RHI_Metal_Texture* mtl_texture = RHI_Metal_TextureFromHandle(texture);

    id<MTL4ComputeCommandEncoder> compute_encoder = [mtl_command_buffer->mtl computeCommandEncoder]; {
      [mtl_command_buffer->mtl useResidencySet:_rhi_metal_context.residency_set];

      [compute_encoder copyFromBuffer:mtl_buffer->mtl
        sourceOffset: offset
        sourceBytesPerRow:mtl_texture->size.x*RHI_BytesPerPixelFromFormat(mtl_texture->format)
        sourceBytesPerImage:mtl_texture->size.x*mtl_texture->size.y*RHI_BytesPerPixelFromFormat(mtl_texture->format)
        sourceSize:MTLSizeMake(mtl_texture->size.x, mtl_texture->size.y, mtl_texture->size.z)
        toTexture:mtl_texture->mtl
        destinationSlice:0
        destinationLevel:0
        destinationOrigin:MTLOriginMake(0, 0, 0)
      ];

      [compute_encoder barrierAfterStages:MTLStageBlit
        beforeQueueStages:MTLStageFragment
        visibilityOptions:MTL4VisibilityOptionResourceAlias
      ];
    }
    [compute_encoder endEncoding];
  }
}

func RHI_TextureFormat
RHI_Metal_GetTextureFormat(RHI_Texture texture) {
  return RHI_Metal_TextureFromHandle(texture)->format;
}

func Vec2I32
RHI_Metal_GetTextureDimension(RHI_Texture texture) {
  // --AlNov: @TODO
  Vec2I32 result = MakeVec2I32(0.0f, 0.0f);
  return result;
}

func RHI_TextureSampler
RHI_Metal_CreateTextureSampler(RHI_TextureSamplerCreateInfo* info) {
  // --AlNov: @TODO
  RHI_TextureSampler result = 0;
  return result;
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
  result.mtl = [_rhi_metal_context.device newCommandBuffer];
  for (I32 i = 0; i < RHI_FRAMES_IN_FLIGHT; i += 1) {
    result.allocator[i] = [_rhi_metal_context.device newCommandAllocator];

    MTLResidencySetDescriptor* residency_set_descriptor = [[MTLResidencySetDescriptor new] init];
    residency_set_descriptor.initialCapacity = 42;
    result.event_count = 0;
    result.shared_event = [_rhi_metal_context.device newSharedEvent];
    result.shared_event.signaledValue = result.event_count;
  }

  return RHI_Metal_CommandBufferArrayAdd(&_rhi_metal_context.command_buffers, result);
}

func void
RHI_Metal_BeginCommandBuffer(RHI_CommandBuffer command_buffer) {
  @autoreleasepool {
    RHI_Metal_CommandBuffer* metal_command_buffer = RHI_Metal_CommandBufferFromHandle(command_buffer);

    metal_command_buffer->event_count += 1;
    [metal_command_buffer->shared_event waitUntilSignaledValue:metal_command_buffer->event_count - RHI_FRAMES_IN_FLIGHT timeoutMS:10];
 
    [metal_command_buffer->allocator[metal_command_buffer->current_frame] reset];
    [metal_command_buffer->mtl beginCommandBufferWithAllocator:metal_command_buffer->allocator[metal_command_buffer->current_frame]];
  }
}

func void
RHI_Metal_EndCommandBuffer(RHI_CommandBuffer command_buffer) {
  RHI_Metal_CommandBuffer* metal_command_buffer = RHI_Metal_CommandBufferFromHandle(command_buffer);

  [metal_command_buffer->mtl endCommandBuffer];
  metal_command_buffer->current_frame = (metal_command_buffer->current_frame + 1)%RHI_FRAMES_IN_FLIGHT;
}

func void
RHI_Metal_SubmitCommandBuffer(RHI_CommandBuffer command_buffer, RHI_SemaphoreSignalInfo* wait_semaphores, I32 wait_semaphores_count, RHI_SemaphoreSignalInfo* signal_semaphores, I32 signal_semaphores_count) {
  @autoreleasepool {
    RHI_Metal_CommandBuffer* metal_command_buffer = RHI_Metal_CommandBufferFromHandle(command_buffer);
    [_rhi_metal_context.command_queue commit:&metal_command_buffer->mtl count:1];

    for (I32 signal_index = 0; signal_index < signal_semaphores_count; signal_index += 1) {
      RHI_SemaphoreSignalInfo* info = signal_semaphores + signal_index;
      RHI_Metal_Semaphore* mtl_semaphore = RHI_Metal_SemaphoreFromHandle(info->semaphore);
      [_rhi_metal_context.command_queue signalEvent:mtl_semaphore->event value:info->value];
    }
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

    RHI_Metal_Texture* metal_texture = RHI_Metal_TextureArrayGetPointer(&_rhi_metal_context.textures, result);
    metal_texture->mtl = [_rhi_metal_context.current_drawable texture];

    _rhi_metal_context.current_swapchain_texture_index = (_rhi_metal_context.current_swapchain_texture_index + 1)%_rhi_metal_context.drawable_count;

    return result;
  }
}

func void
RHI_Metal_Present(RHI_CommandBuffer command_buffer) {
  RHI_Metal_CommandBuffer* metal_command_buffer = RHI_Metal_CommandBufferFromHandle(command_buffer);
  [_rhi_metal_context.command_queue waitForDrawable:_rhi_metal_context.current_drawable];
  [_rhi_metal_context.command_queue signalEvent:metal_command_buffer->shared_event value:metal_command_buffer->event_count];
  [_rhi_metal_context.command_queue signalDrawable:_rhi_metal_context.current_drawable];
  [_rhi_metal_context.current_drawable present];
}


// -------------------------------------------------------------------
// -- Render Pass ----------------------------------------------------
func RHI_RenderPass*
RHI_Metal_BeginRenderPass(RHI_CommandBuffer command_buffer, U32 color_targets_count, RHI_ColorTarget* color_targets, RHI_DepthStencilTarget* depth_stencil_target, RHI_Resource* resources, I32 resources_count) {
  @autoreleasepool {
    RHI_Metal_CommandBuffer* mtl_command_buffer = RHI_Metal_CommandBufferFromHandle(command_buffer);
    
    Assert(mtl_command_buffer->render_encoder == nil);

    Assert(_rhi_metal_context.current_drawable != nil);

    MTL4RenderPassDescriptor* pass_descriptor = [MTL4RenderPassDescriptor new];
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

    if (depth_stencil_target != 0) {
      RHI_Metal_Texture* texture = RHI_Metal_TextureFromHandle(depth_stencil_target->texture);

      pass_descriptor.depthAttachment.texture = texture->mtl;
      pass_descriptor.depthAttachment.loadAction = RHI_Metal_LoadActionFromRHI(depth_stencil_target->load_operation);
      pass_descriptor.depthAttachment.storeAction = RHI_Metal_StoreActionFromRHI(depth_stencil_target->store_operation);
      pass_descriptor.depthAttachment.clearDepth = depth_stencil_target->clear_depth;
    }

    mtl_command_buffer->render_encoder = [mtl_command_buffer->mtl renderCommandEncoderWithDescriptor:pass_descriptor];
    [mtl_command_buffer->mtl useResidencySet:_rhi_metal_context.residency_set];
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
func RHI_Shader
RHI_Metal_CreateShader(Arena* arena, RHI_ShaderCreateInfo* info) {
  RHI_Shader result = ZeroStruct();

  ScratchArena scratch = BeginScratchArena(arena); 
  Str8 file_name = ConcatStr8(scratch.arena, info->file_name, Str8C(".metal"));
  FILE* file = fopen(CFromStr8(file_name), "r");
  Assert(file);
  EndScratchArena(scratch);

  fseek(file, 0L, SEEK_END);
  result.code_size = ftell(file);
  result.code = (U8*)PushArena(arena, result.code_size*sizeof(U8));
  rewind(file);
  fread(result.code, result.code_size*sizeof(U8), 1, file);
  fclose(file);

  result.kind = info->kind;
  result.language = RHI_ShaderLanguage_Metal;
  result.arguments = PushArena(arena, sizeof(RHI_ShaderArgumentKind)*info->arguments_count);
  for (I32 argument_index = 0; argument_index < info->arguments_count; argument_index += 1) {
    result.arguments[argument_index] = info->arguments[argument_index];
  }
  result.arguments_count = info->arguments_count;

  return result;
}

func RHI_Metal_GraphicsPipeline*
RHI_Metal_GraphicsPipelineFromHandle(RHI_GraphicsPipeline handle) {
  return RHI_Metal_GraphicsPipelineArrayGetPointer(&_rhi_metal_context.graphics_pipelines, handle);
}

func RHI_GraphicsPipeline
RHI_Metal_CreateGraphicsPipeline(RHI_GraphicsPipelineCreateInfo* info) {
  @autoreleasepool {
    RHI_Metal_GraphicsPipeline pipeline = ZeroStruct();

     NSError* error = nil;
    MTLCompileOptions* compile_options = [[MTLCompileOptions alloc] init];
    compile_options.languageVersion = MTLLanguageVersion3_0;
    Assert(error == nil);

    NSString* vertex_source = [[NSString alloc] initWithBytesNoCopy:info->vertex_shader->code length:info->vertex_shader->code_size encoding:NSUTF8StringEncoding freeWhenDone:NO];
    id<MTLLibrary> vertex_library = [_rhi_metal_context.device newLibraryWithSource:vertex_source options:compile_options error:&error];
    if (error) {
      NSLog(@"%@", error);
      Assert(1);
    }
    id<MTLFunction> vertex_function = [vertex_library newFunctionWithName:@"VertexMain"];
    Assert(vertex_function != nil)
    Assert(error == nil);

    NSString* fragment_source = [[NSString alloc] initWithBytesNoCopy:info->fragment_shader->code length:info->fragment_shader->code_size encoding:NSUTF8StringEncoding freeWhenDone:NO];
    id<MTLLibrary> fragment_library = [_rhi_metal_context.device newLibraryWithSource:fragment_source options:compile_options error:&error];
    if (error) {
      NSLog(@"%@", error);
      Assert(1);
    }
    id<MTLFunction> fragment_function = [fragment_library newFunctionWithName:@"FragmentMain"];
    Assert(fragment_function != nil);
    Assert(error == nil);

    {
        
      I32 buffer_addresses_count = 1; // --AlNov: @NOTO buffer(0) is reserved as vertex buffer
      I32 textures_count = 0;
      for(I32 argument_index = 0; argument_index < info->vertex_shader->arguments_count; argument_index += 1) {
        if (info->vertex_shader->arguments[argument_index] == RHI_ShaderArgumentKind_BufferAddress) {
          buffer_addresses_count += 1;
        };
        if (info->vertex_shader->arguments[argument_index] == RHI_ShaderArgumentKind_ArrayOfTextures) {
          textures_count = 512;
        }
      }
      MTL4ArgumentTableDescriptor* table_descriptor = [MTL4ArgumentTableDescriptor new];
      table_descriptor.initializeBindings = 1,
      table_descriptor.maxBufferBindCount = buffer_addresses_count;
      table_descriptor.maxTextureBindCount = textures_count;
      pipeline.vertex_argument_table = [_rhi_metal_context.device newArgumentTableWithDescriptor:table_descriptor error:&error];
    }

    {
      I32 textures_count = 0;
      I32 buffer_addresses_count = 1; // --AlNov: @NOTO buffer(0) is reserved as vertex buffer
      for(I32 argument_index = 0; argument_index < info->fragment_shader->arguments_count; argument_index += 1) {
        if (info->fragment_shader->arguments[argument_index] == RHI_ShaderArgumentKind_BufferAddress) {
          buffer_addresses_count += 1;
        };
        if (info->vertex_shader->arguments[argument_index] == RHI_ShaderArgumentKind_ArrayOfTextures) {
          textures_count = 512;
        }
      }
      MTL4ArgumentTableDescriptor* table_descriptor = [MTL4ArgumentTableDescriptor new];
      table_descriptor.initializeBindings = 1,
      table_descriptor.maxBufferBindCount = buffer_addresses_count;
      pipeline.fragment_argument_table = [_rhi_metal_context.device newArgumentTableWithDescriptor:table_descriptor error:&error];
    }

    MTLRenderPipelineDescriptor* pipeline_descriptor = [MTLRenderPipelineDescriptor new];
    pipeline_descriptor.vertexFunction = vertex_function;
    pipeline_descriptor.fragmentFunction = fragment_function;
    
    U32 stride = 0;
    MTLVertexDescriptor* vertex_descriptor = [MTLVertexDescriptor vertexDescriptor];
    for (I32 i = 0; i < info->vertex_attributes_count; i += 1) {
      RHI_VertexAttribute* vertex_attribute = info->vertex_attributes + i;

      vertex_descriptor.attributes[i].format = RHI_Metal_VertexFormatFromRHI(vertex_attribute->format);
      vertex_descriptor.attributes[i].offset = vertex_attribute->offset;
      vertex_descriptor.attributes[i].bufferIndex = 0; // @NOTE @TODO fixed index for Vertex Buffer

      stride += RHI_GetSizeOfVertexAttributeFormat(vertex_attribute->format);
    }
    if (stride > 0) {
      vertex_descriptor.layouts[0].stride = stride;
      vertex_descriptor.layouts[0].stepRate = 1;
      vertex_descriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
    }
    pipeline_descriptor.vertexDescriptor = vertex_descriptor;

    for (I32 i = 0; i < info->color_targets_count; i += 1) {
      pipeline_descriptor.colorAttachments[0].pixelFormat = RHI_Metal_PixelFormatFromRHI(info->color_target_infos[i].format);
      pipeline_descriptor.colorAttachments[0].blendingEnabled = info->color_target_infos[i].blend_enable;
      pipeline_descriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
      pipeline_descriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
      pipeline_descriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
      pipeline_descriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
      pipeline_descriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorZero;
      pipeline_descriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
    }

    if (info->depth_stencil_state.depth_test_enable) {
      pipeline_descriptor.depthAttachmentPixelFormat = RHI_Metal_PixelFormatFromRHI(info->depth_stencil_state.depth_target_format);

      MTLDepthStencilDescriptor* depth_stencil_descriptor = [MTLDepthStencilDescriptor new];
      depth_stencil_descriptor.depthCompareFunction = RHI_Metal_CompareFunctionFromRHI(info->depth_stencil_state.depth_compare_operation);
      depth_stencil_descriptor.depthWriteEnabled = info->depth_stencil_state.depth_write_enable;
      pipeline.depth_stencil_state = [_rhi_metal_context.device newDepthStencilStateWithDescriptor:depth_stencil_descriptor];
    }

    pipeline.mtl = [_rhi_metal_context.device newRenderPipelineStateWithDescriptor:pipeline_descriptor error:&error];
    if (pipeline.mtl == nil) {
      LogError("Failed to create Graphics Pipeline. Error: %s\n", [error.localizedDescription UTF8String]);
      NSLog(@"%@", error);
      Assert(1);
    }

    return RHI_Metal_GraphicsPipelineArrayAdd(&_rhi_metal_context.graphics_pipelines, pipeline);
  }
}

func void
RHI_Metal_BindGraphicsPipeline(RHI_CommandBuffer command_buffer, RHI_GraphicsPipeline pipeline) {
  @autoreleasepool {
    RHI_Metal_CommandBuffer* mtl_command_buffer = RHI_Metal_CommandBufferFromHandle(command_buffer);
    RHI_Metal_GraphicsPipeline* mtl_pipeline = RHI_Metal_GraphicsPipelineFromHandle(pipeline);

    Assert(mtl_command_buffer->render_encoder != nil);
    
    [mtl_command_buffer->render_encoder setRenderPipelineState:mtl_pipeline->mtl];
    if (mtl_pipeline->depth_stencil_state != nil) {
      [mtl_command_buffer->render_encoder setDepthStencilState:mtl_pipeline->depth_stencil_state];
    }

    mtl_command_buffer->current_graphics_pipeline = mtl_pipeline;
  }
}

// -------------------------------------------------------------------
// -- Resource Table -------------------------------------------------

func RHI_ResourceTable
RHI_Metal_CreateResourceTable(Arena* arena, U32 max_textures_count, U32 max_samplers_count) {
  return 0;
}

func void
RHI_Metal_DestroyResourceTable(RHI_ResourceTable* table) {
}

func RHI_TextureDeviceId
RHI_Metal_ResourceTableAddTexture(RHI_ResourceTable table, RHI_Texture texture) {
  RHI_Metal_Texture* mtl_texture = RHI_Metal_TextureFromHandle(texture);
  return mtl_texture->mtl.gpuResourceID._impl;
}

func RHI_SamplerDeviceId
RHI_Metal_ResourceTableAddSampler(RHI_ResourceTable table, RHI_TextureSampler sampler) {
  return 0;
}

func void
RHI_Metal_BindResourceTable(RHI_CommandBuffer command_buffer, RHI_ResourceTable table) {
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
  mtl_command_buffer->current_viewport = mtl_viewport;
}

func RectI32
RHI_Metal_GetViewport(RHI_CommandBuffer command_buffer) {
  RHI_Metal_CommandBuffer* mtl_command_buffer = RHI_Metal_CommandBufferFromHandle(command_buffer);
  RectI32 result = ZeroStruct();
  result.x = mtl_command_buffer->current_viewport.originX;
  result.y = mtl_command_buffer->current_viewport.originY;
  result.w = mtl_command_buffer->current_viewport.width;
  result.h = mtl_command_buffer->current_viewport.height;
  return result;
}

func RectI32
RHI_Metal_GetScissor(RHI_CommandBuffer command_buffer) {
  RectI32 result = ZeroStruct();
  // --AlNov: @TODO
  return result;
}

func void
RHI_Metal_SetScissor(RHI_CommandBuffer command_buffer, RectI32 scissor) {
  RHI_Metal_CommandBuffer* mtl_command_buffer = RHI_Metal_CommandBufferFromHandle(command_buffer);
  MTLScissorRect mtl_scissor = {
    .height = scissor.h,
    .width = scissor.w,
    .x = scissor.x,
    .y = scissor.y,
  };
  [mtl_command_buffer->render_encoder setScissorRect:mtl_scissor];
}

func void
RHI_Metal_DrawPrimitives(RHI_CommandBuffer command_buffer, U32 vertex_count, U32 instance_count, U32 first_vertex, U32 first_instance) {
  RHI_Metal_CommandBuffer* mtl_command_buffer = RHI_Metal_CommandBufferFromHandle(command_buffer);

  Assert(mtl_command_buffer->render_encoder);
  
  [mtl_command_buffer->render_encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:vertex_count instanceCount:instance_count baseInstance:first_instance];
}

func void
RHI_Metal_DrawIndexedPrimitives(RHI_CommandBuffer command_buffer, U32 index_count, U32 instance_count, U32 first_index, I32 vertex_offset, U32 first_instance) {
  RHI_Metal_CommandBuffer* mtl_command_buffer = RHI_Metal_CommandBufferFromHandle(command_buffer);

  Assert(mtl_command_buffer->render_encoder);

  [mtl_command_buffer->render_encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
    indexCount:index_count
    indexType:RHI_Metal_IndexTypeFromRHI(mtl_command_buffer->index_size)
    indexBuffer:mtl_command_buffer->current_index_buffer->mtl.gpuAddress + mtl_command_buffer->index_buffer_offset
    indexBufferLength:mtl_command_buffer->current_index_buffer->capacity - mtl_command_buffer->index_buffer_offset
    instanceCount:instance_count
    baseVertex:vertex_offset
    baseInstance:first_instance
  ];

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
    case RHI_TextureFormat_R8_UNORM:            result = MTLPixelFormatR8Unorm;         break;
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
    case MTLPixelFormatR8Unorm:         result = RHI_TextureFormat_R8_UNORM;            break;
    case MTLPixelFormatDepth16Unorm:    result = RHI_TextureFormat_D16_UNORM;           break;
    case MTLPixelFormatR16Uint:         result = RHI_TextureFormat_R16_UINT;            break;
  }
  return result;
}

func MTLVertexFormat
RHI_Metal_VertexFormatFromRHI(RHI_VertexAttributeFormat format) {
  MTLVertexFormat result = MTLVertexFormatInvalid;
  switch (format) {
    default: Assert(1); break;

    case RHI_VertexAttributeFormat_Vec2F32: result = MTLVertexFormatFloat2; break;
    case RHI_VertexAttributeFormat_Vec3F32: result = MTLVertexFormatFloat3; break;
    case RHI_VertexAttributeFormat_Vec4F32: result = MTLVertexFormatFloat4; break;
    
    case RHI_VertexAttributeFormat_Vec4I32: result = MTLVertexFormatInt4; break;
  };
  return result;
}

func MTLIndexType
RHI_Metal_IndexTypeFromRHI(RHI_IndexSize index_size) {
  MTLIndexType result = 0;
  switch (index_size) {
    default: Assert(1); break;

    case RHI_IndexSize_U16: result = MTLIndexTypeUInt16; break;
    case RHI_IndexSize_U32: result = MTLIndexTypeUInt32; break;
  }
  return result;
}

func MTLCompareFunction
RHI_Metal_CompareFunctionFromRHI(RHI_CompareOperation operation) {
  MTLCompareFunction result = MTLCompareFunctionNever;
  switch (operation) {
    default: Assert(1); break;

    case RHI_CompareOperation_Equal:          result = MTLCompareFunctionEqual;        break;
    case RHI_CompareOperation_NotEqual:       result = MTLCompareFunctionNotEqual;     break;
    case RHI_CompareOperation_Less:           result = MTLCompareFunctionLess;         break;
    case RHI_CompareOperation_LessOrEqual:    result = MTLCompareFunctionLessEqual;    break;
    case RHI_CompareOperation_Greater:        result = MTLCompareFunctionGreater;      break;
    case RHI_CompareOperation_GreaterOrEqual: result = MTLCompareFunctionGreaterEqual; break;
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
  _rhi_metal_context.semaphores = RHI_Metal_SemaphoreArrayAllocate(_rhi_metal_context.arena, 128);
  _rhi_metal_context.semaphores.length = _rhi_metal_context.semaphores.capacity;
  _rhi_metal_context.data_buffers = RHI_Metal_BufferArrayAllocate(_rhi_metal_context.arena, 32);
  _rhi_metal_context.graphics_pipelines = RHI_Metal_GraphicsPipelineArrayAllocate(_rhi_metal_context.arena, 32);

  _rhi_metal_context.device = MTLCreateSystemDefaultDevice();
  Assert(_rhi_metal_context.device != nil);
  LogInfo("Metal. Device name: %s\n", [[_rhi_metal_context.device name]UTF8String]);

  _rhi_metal_context.command_queue = [_rhi_metal_context.device newMTL4CommandQueue];
  MTLResidencySetDescriptor* residency_set_descriptor = [[MTLResidencySetDescriptor new] init];
  residency_set_descriptor.initialCapacity = 42;
  _rhi_metal_context.residency_set = [_rhi_metal_context.device newResidencySetWithDescriptor:residency_set_descriptor error:0];

  OS_MacOS_Window* macos_window = (OS_MacOS_Window*)window;
  OS_MacOS_View* view = (__bridge OS_MacOS_View*)macos_window->ns_view;
  Assert(view != nil);

  CAMetalLayer* metal_layer = [view MetalLayer];
  metal_layer.device = _rhi_metal_context.device;
  metal_layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
  metal_layer.displaySyncEnabled = 1;
  metal_layer.framebufferOnly = 1;
  metal_layer.drawableSize = NSSizeToCGSize([view bounds].size);

  _rhi_metal_context.drawable_texture_format = MTLPixelFormatBGRA8Unorm;

  _rhi_metal_context.drawable_count = [metal_layer maximumDrawableCount];
  _rhi_metal_context.textures = RHI_Metal_TextureArrayAllocate(_rhi_metal_context.arena, 512);
  _rhi_metal_context.textures.length = _rhi_metal_context.drawable_count;

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
