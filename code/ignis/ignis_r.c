#include "ignis_r.h"

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"

func void
Ignis_R_Init(RHI_RendererKind kind, OS_Window* window)
{
  RHI_Init(kind, window);
  D_Init(Megabytes(16));

  RHI_BufferUsageFlags triangle_buffer_usage_flags = RHI_BufferUsageFlag_Vertex|RHI_BufferUsageFlag_Index|RHI_BufferUsageFlag_Uniform;

  _ignis_r_state.arena = AllocateArena(Gigabytes(32), Kilobytes(64));
  _ignis_r_state.window = window;
  _ignis_r_state.data_buffer = RHI_CreateBuffer(Megabytes(64), triangle_buffer_usage_flags, RHI_BufferPropertyFlag_HostCoherent);
  _ignis_r_state.transfer_buffer = RHI_CreateBuffer(Megabytes(128), RHI_BufferUsageFlag_Transfer, RHI_BufferPropertyFlag_HostCoherent);
  _ignis_r_state.command_buffer = RHI_GetCommandBuffer();

  Ignis_R_PrepareTextures();
  Ignis_R_PreparePipelines();
}

func RHI_Texture
Ignis_R_CreateLoadTexture(RHI_Buffer buffer, Str8 path, RHI_TextureFormat format)
{
  RHI_Texture result = {0};

  I32 tex_width = 0;
  I32 tex_height = 0;
  I32 tex_channels = 0;
  U8* tex_pixels = stbi_load(CFromStr8(path), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);

  if (!tex_pixels)
  {
    LogError("Cannot load texture %s\n", CFromStr8(path));
  }
  I32 texture_size = tex_width * tex_height * 4;

  result = RHI_CreateTexture(&(RHI_TextureCreateInfo){
    .kind = RHI_TextureKind_2D,
    .format = format,
    .usage_flags = RHI_TEXTURE_USAGE_FLAG_SAMPLED | RHI_TEXTURE_USAGE_FLAG_TRANSFER_DST,
    .width = tex_width,
    .height = tex_height,
    .depth = 1,
    .num_levels = 1,
  });

  U64 texture_offset = RHI_PushBuffer(buffer, tex_pixels, texture_size);
  RHI_CopyBufferToTexture(0, buffer, texture_offset, texture_size, result);

  return result;
}

func void
Ignis_R_PreparePipelines()
{
  // --AlNov: Word Grid
  {
    RHI_Shader grid_vertex_shader = RHI_CreateShader(
      _ignis_r_state.arena,
      &(RHI_ShaderCreateInfo){
        .file_name = Str8C("./data/shaders/grid.vs.glsl"),
        .kind = RHI_ShaderKind_Vertex,
        .global_uniforms_count = 1,
        .instance_uniforms_count = 1,
      }
    );

    RHI_Shader grid_fragment_shader = RHI_CreateShader(
      _ignis_r_state.arena,
      &(RHI_ShaderCreateInfo){
        .file_name = Str8C("./data/shaders/grid.fs.glsl"),
        .kind = RHI_ShaderKind_Fragment,
        .global_uniforms_count = 1,
        .instance_uniforms_count = 0,
      }
    );

    _ignis_r_state.grid_pipeline = RHI_CreateGraphicsPipeline(
      &(RHI_GraphicsPipelineCreateInfo){
        .vertex_shader = grid_vertex_shader,
        .fragment_shader = grid_fragment_shader,
        .color_targets_count = 1,
        .color_target_infos = &(RHI_GraphicsPipelineColorTargetInfo){
          .format = RHI_GetSwapchainTextureFormat(),
          .blend_enable = 1,
        },
        .depth_stencil_state = {
          .depth_test_enable = 1,
          .depth_write_enable = 0,
          .depth_compare_operation = RHI_CompareOperation_Greater,
          .depth_target_format = RHI_GetTextureFormat(_ignis_r_state.depth_texture),
        },
      }
    );
  }

  // Depth Prepass
  {
    RHI_Shader vertex_shader = RHI_CreateShader(_ignis_r_state.arena, &(RHI_ShaderCreateInfo){
      .file_name = Str8C("./data/shaders/ignis/depth_prepass.vs.glsl"),
      .kind = RHI_ShaderKind_Vertex,
      .instance_uniforms_count = 1,
    });

    RHI_Shader fragment_shader = RHI_CreateShader(_ignis_r_state.arena, &(RHI_ShaderCreateInfo){
      .file_name = Str8C("./data/shaders/ignis/depth_prepass.fs.glsl"),
      .kind = RHI_ShaderKind_Fragment,
    });

    RHI_VertexAttribute vertex_attributes[] = {
      {
        .location = 0,
        .format = RHI_VertexAttributeFormat_Vec3F32,
        .offset = offsetof(AST_Vertex, position),
      },
      {
        .location = 1,
        .format = RHI_VertexAttributeFormat_Vec3F32,
        .offset = offsetof(AST_Vertex, normal),
      },
      {
        .location = 2,
        .format = RHI_VertexAttributeFormat_Vec3F32,
        .offset = offsetof(AST_Vertex, tangent),
      },
      {
        .location = 3,
        .format = RHI_VertexAttributeFormat_Vec2F32,
        .offset = offsetof(AST_Vertex, uv),
      },
      {
        .location = 4,
        .format = RHI_VertexAttributeFormat_Vec4I32,
        .offset = offsetof(AST_Vertex, joint_ids),
      },
      {
        .location = 5,
        .format = RHI_VertexAttributeFormat_Vec4F32,
        .offset = offsetof(AST_Vertex, joint_weights),
      },
    };

    _ignis_r_state.depth_prepass_pipeline = RHI_CreateGraphicsPipeline(&(RHI_GraphicsPipelineCreateInfo){
      .vertex_shader = vertex_shader,
      .fragment_shader = fragment_shader,
      .vertex_attributes_count = ArrayLength(vertex_attributes),
      .vertex_attributes = vertex_attributes,
      .depth_stencil_state = {
        .depth_test_enable = 1,
        .depth_write_enable = 1,
        .depth_compare_operation = RHI_CompareOperation_Greater,
        .depth_target_format = RHI_GetTextureFormat(_ignis_r_state.depth_texture),
      },
    });
  }

  {
    RHI_Shader vertex_shader = RHI_CreateShader(
      _ignis_r_state.arena,
      &(RHI_ShaderCreateInfo){
        .file_name = Str8C("./data/shaders/ignis/shadow_map.vs.glsl"),
        .kind = RHI_ShaderKind_Vertex,
        .instance_uniforms_count = 1,
      }
    );
    RHI_Shader fragment_shader = RHI_CreateShader(
      _ignis_r_state.arena,
      &(RHI_ShaderCreateInfo){
        .file_name = Str8C("./data/shaders/ignis/shadow_map.fs.glsl"),
        .kind = RHI_ShaderKind_Fragment,
      }
    );

    RHI_VertexAttribute vertex_attributes[] = {
      {
        .location = 0,
        .format = RHI_VertexAttributeFormat_Vec3F32,
        .offset = offsetof(AST_Vertex, position),
      },
      {
        .location = 1,
        .format = RHI_VertexAttributeFormat_Vec3F32,
        .offset = offsetof(AST_Vertex, normal),
      },
      {
        .location = 2,
        .format = RHI_VertexAttributeFormat_Vec3F32,
        .offset = offsetof(AST_Vertex, tangent),
      },
      {
        .location = 3,
        .format = RHI_VertexAttributeFormat_Vec2F32,
        .offset = offsetof(AST_Vertex, uv),
      },
      {
        .location = 4,
        .format = RHI_VertexAttributeFormat_Vec4I32,
        .offset = offsetof(AST_Vertex, joint_ids),
      },
      {
        .location = 5,
        .format = RHI_VertexAttributeFormat_Vec4F32,
        .offset = offsetof(AST_Vertex, joint_weights),
      },
    };

    _ignis_r_state.shadow_map_pipeline = RHI_CreateGraphicsPipeline(
      &(RHI_GraphicsPipelineCreateInfo){
        .vertex_shader = vertex_shader,
        .fragment_shader = fragment_shader,
        .vertex_attributes_count = ArrayLength(vertex_attributes),
        .vertex_attributes = vertex_attributes,
        .depth_stencil_state = {
          .depth_test_enable = 1,
          .depth_write_enable = 1,
          .depth_compare_operation = RHI_CompareOperation_Greater,
          .depth_target_format = RHI_GetTextureFormat(_ignis_r_state.shadow_map),
        },
      }
    );
  }

  // Mesh Pipeline
  {
    RHI_Shader mesh_vertex_shader = RHI_CreateShader(
      _ignis_r_state.arena,
      &(RHI_ShaderCreateInfo){
        .file_name = Str8C("./data/shaders/ignis/mesh.vs.glsl"),
        .kind = RHI_ShaderKind_Vertex,
        .instance_uniforms_count = 1,
      }
    );
    RHI_Shader mesh_fragment_shader = RHI_CreateShader(
      _ignis_r_state.arena,
      &(RHI_ShaderCreateInfo){
        .file_name = Str8C("./data/shaders/ignis/mesh.fs.glsl"),
        .kind = RHI_ShaderKind_Fragment,
        .global_uniforms_count = 1,
        .global_samplers_count = 0,
      }
    );

    RHI_VertexAttribute mesh_vertex_attributes[] = {
      {
        .location = 0,
        .format = RHI_VertexAttributeFormat_Vec3F32,
        .offset = offsetof(AST_Vertex, position),
      },
      {
        .location = 1,
        .format = RHI_VertexAttributeFormat_Vec3F32,
        .offset = offsetof(AST_Vertex, normal),
      },
      {
        .location = 2,
        .format = RHI_VertexAttributeFormat_Vec3F32,
        .offset = offsetof(AST_Vertex, tangent),
      },
      {
        .location = 3,
        .format = RHI_VertexAttributeFormat_Vec2F32,
        .offset = offsetof(AST_Vertex, uv),
      },
      {
        .location = 4,
        .format = RHI_VertexAttributeFormat_Vec4I32,
        .offset = offsetof(AST_Vertex, joint_ids),
      },
      {
        .location = 5,
        .format = RHI_VertexAttributeFormat_Vec4F32,
        .offset = offsetof(AST_Vertex, joint_weights),
      },
    };

    RHI_GraphicsPipelineColorTargetInfo mesh_pipeline_color_target_infos[] = {
      {
        .format = RHI_GetSwapchainTextureFormat(),
        .blend_enable = 1,
      },
    };
    RHI_GraphicsPipelineCreateInfo mesh_pipeline_info = {
      .vertex_shader = mesh_vertex_shader,
      .fragment_shader = mesh_fragment_shader,
      .vertex_attributes_count = ArrayLength(mesh_vertex_attributes),
      .vertex_attributes = mesh_vertex_attributes,
      .color_targets_count = ArrayLength(mesh_pipeline_color_target_infos),
      .color_target_infos = mesh_pipeline_color_target_infos,
      .depth_stencil_state = {
        .depth_test_enable = 1,
        .depth_write_enable = 0,
        .depth_compare_operation = RHI_CompareOperation_Equal,
        .depth_target_format = RHI_GetTextureFormat(_ignis_r_state.depth_texture),
      },
    };
    _ignis_r_state.mesh_pipeline = RHI_CreateGraphicsPipeline(&mesh_pipeline_info);
  }
}

func void
Ignis_R_PrepareTextures()
{
  RHI_Buffer transfer_buffer = _ignis_r_state.transfer_buffer;

  _ignis_r_state.texture_sampler = RHI_CreateTextureSampler(
    &(RHI_TextureSamplerCreateInfo){
      .mag_filter = RHI_FilterKind_Linear,
      .min_filter = RHI_FilterKind_Linear,
      .address_mode_u = RHI_SamplerAddressMode_Repeat,
      .address_mode_v = RHI_SamplerAddressMode_Repeat,
      .address_mode_w = RHI_SamplerAddressMode_Repeat,
      .mipmap_mode = RHI_SamplerMipmapMode_Linear,
    }
  );

  _ignis_r_state.default_color_texture = Ignis_R_CreateLoadTexture(transfer_buffer, Str8C("./data/uv_checker.png"), RHI_TextureFormat_R8G8B8A8_SRGB);
  _ignis_r_state.mesh_color_texture    = Ignis_R_CreateLoadTexture(transfer_buffer, Str8C("./data/sphere_gltf/RockyColor.png"), RHI_TextureFormat_R8G8B8A8_SRGB);
  _ignis_r_state.mesh_normal_texture   = Ignis_R_CreateLoadTexture(transfer_buffer, Str8C("./data/sphere_gltf/RockyNormal.png"), RHI_TextureFormat_R8G8B8A8_UNORM);

  _ignis_r_state.test_texture = RHI_CreateTexture(
    &(RHI_TextureCreateInfo){
      .kind = RHI_TextureKind_2D,
      .format = RHI_TextureFormat_R16_UINT,
      .usage_flags = RHI_TEXTURE_USAGE_FLAG_COLOR_ATTACHMENT | RHI_TEXTURE_USAGE_FLAG_TRANSFER_SRC,
      .width = _ignis_r_state.window->size.w,
      .height = _ignis_r_state.window->size.h,
      .depth = 1,
      .num_levels = 1
    }
  );

  _ignis_r_state.depth_texture = RHI_CreateTexture(
    &(RHI_TextureCreateInfo){
      .kind = RHI_TextureKind_2D,
      .format = RHI_TextureFormat_D16_UNORM,
      .usage_flags = RHI_TEXTURE_USAGE_FLAG_DEPTH_STENCIL_ATTACHMENT,
      .width = _ignis_r_state.window->size.w,
      .height = _ignis_r_state.window->size.h,
      .depth = 1,
      .num_levels = 1,
    }
  );

  _ignis_r_state.shadow_map = RHI_CreateTexture(
    &(RHI_TextureCreateInfo){
      .kind = RHI_TextureKind_2D,
      .format = RHI_TextureFormat_D16_UNORM,
      .usage_flags = RHI_TEXTURE_USAGE_FLAG_SAMPLED | RHI_TEXTURE_USAGE_FLAG_DEPTH_STENCIL_ATTACHMENT,
      .width = 1024,
      .height = 1024,
      .depth = 1,
      .num_levels = 1,
    }
  );
}

func void
Ignis_R_Resize(Vec2I32 size)
{
  if (size.x != 0 && size.y != 0)
  {
    RHI_VK_HandleResize(_ignis_r_state.window);

    RHI_VK_DestroyTexture(_ignis_r_state.depth_texture);
    RHI_TextureCreateInfo depth_texture_info = {
      .kind = RHI_TextureKind_2D,
      .format = RHI_TextureFormat_D16_UNORM,
      .usage_flags = RHI_TEXTURE_USAGE_FLAG_DEPTH_STENCIL_ATTACHMENT,
      .width = _ignis_r_state.window->size.w,
      .height = _ignis_r_state.window->size.h,
      .depth = 1,
      .num_levels = 1
    };
    _ignis_r_state.depth_texture = RHI_CreateTexture(&depth_texture_info);

    RHI_VK_DestroyTexture(_ignis_r_state.test_texture);
    RHI_TextureCreateInfo test_texture_info = {
      .kind = RHI_TextureKind_2D,
      .format = RHI_TextureFormat_R16_UINT,
      .usage_flags = RHI_TEXTURE_USAGE_FLAG_COLOR_ATTACHMENT | RHI_TEXTURE_USAGE_FLAG_TRANSFER_SRC | RHI_TEXTURE_USAGE_FLAG_TRANSFER_DST,
      .width = _ignis_r_state.window->size.w,
      .height = _ignis_r_state.window->size.h,
      .depth = 1,
      .num_levels = 1
    };
    _ignis_r_state.test_texture = RHI_CreateTexture(&test_texture_info);
  }
}

// -------------------------------------------------------------------
// -- Render ---------------------------------------------------------
func void
Ignis_R_BeginFrame()
{
  RHI_CommandBuffer command_buffer = _ignis_r_state.command_buffer;

  RHI_ResetBuffer(_ignis_r_state.data_buffer);
  RHI_ResetBuffer(_ignis_r_state.transfer_buffer);

  RHI_BeginCommandBuffer(command_buffer);
  _ignis_r_state.swapchain = RHI_AcquireSwapchainTexture(command_buffer);
}

func void
Ignis_R_EndFrame()
{
  RHI_CommandBuffer command_buffer = _ignis_r_state.command_buffer;
  RHI_Texture swapchain_texture = _ignis_r_state.swapchain;

  RHI_SubmitCommandBuffer(command_buffer);
  // RHI_PresentTexture(command_buffer, swapchain_texture);
}

func void
Ignis_R_RenderScene(Ignis_Scene* scene)
{
  RHI_CommandBuffer command_buffer = _ignis_r_state.command_buffer;
  RHI_Texture swapchain_texture = _ignis_r_state.swapchain;
  Ignis_Entity* camera = Ignis_GetCamera(scene);

  RectI32 viewport = {
    .x = 0,
    .y = 0,
    .w = RHI_GetTextureDimension(_ignis_r_state.shadow_map).x,
    .h = RHI_GetTextureDimension(_ignis_r_state.shadow_map).y,
  };
  RectI32 scissor = viewport;
#if 0
  RHI_DepthStencilTarget shadow_map_pass_target = {
    .texture = _ignis_r_state.shadow_map,
    .load_operation = RHI_ATTACHMENT_LOAD_OPERATION_CLEAR,
    .store_operation = RHI_ATTACHMENT_STORE_OPERATION_STORE,
    .clear_depth = 0.0f,
  };
  RHI_SetViewport(command_buffer, viewport);
  RHI_SetScissor(command_buffer, scissor);
  RHI_BeginRenderPass(command_buffer, 0, 0, &shadow_map_pass_target);
  {
    for (I32 i = 1; i < scene->entities.length; i += 1)
    {
      Ignis_Entity* entity = Ignis_EntityArrayGetPointer(&scene->entities, i);
      if (entity->type == Ignis_EntityType_Actor)
      {
        Ignis_R_ShadowMapPass(scene, entity);
      }
    }
  }
  RHI_EndRenderPass(command_buffer, 0);
#endif

  RHI_DepthStencilTarget depth_prepass_target = {
    .texture = _ignis_r_state.depth_texture,
    .load_operation = RHI_AttachmentLoadOperation_Clear,
    .store_operation = RHI_AttachmentStoreOperation_Store,
    .clear_depth = 0.0f,
  };

  viewport = (RectI32){
    .x = 0,
    .y = 0,
    .w = (I32)_ignis_r_state.window->size.x,
    .h = (I32)_ignis_r_state.window->size.y,
  };
  scissor = viewport;
  RHI_SetViewport(command_buffer, viewport);
  RHI_SetScissor(command_buffer, scissor);
  RHI_BeginRenderPass(command_buffer, 0, 0, &depth_prepass_target);
  {
    for (I32 i = 0; i < scene->entities.length; i += 1)
    {
      Ignis_Entity* entity = Ignis_EntityArrayGetPointer(&scene->entities, i);
      if (entity->type == Ignis_EntityType_Actor)
      {
        Ignis_R_RenderEntityPrepass(camera, entity);
      }
    }
  }
  RHI_EndRenderPass(command_buffer, 0);

  RHI_ColorTarget entity_color_targets[] = {
    {
      .texture = swapchain_texture,
      .load_operation = RHI_AttachmentLoadOperation_Clear,
      .store_operation = RHI_AttachmentStoreOperation_Store,
      .clear_color = RGBAFromHex(0x111111FF),
    },
  };

  RHI_DepthStencilTarget entity_depth_target = {
    .texture = _ignis_r_state.depth_texture,
      .load_operation = RHI_AttachmentLoadOperation_Clear,
      .store_operation = RHI_AttachmentStoreOperation_DontCare,
  };

  viewport = (RectI32){
    .x = 0,
    .y = 0,
    .w = (I32)_ignis_r_state.window->size.x,
    .h = (I32)_ignis_r_state.window->size.y,
  };
  scissor = viewport;
  RHI_SetViewport(command_buffer, viewport);
  RHI_SetScissor(command_buffer, scissor);
  RHI_BeginRenderPass(command_buffer, ArrayLength(entity_color_targets), entity_color_targets, &entity_depth_target);
  {
    for (I32 i = 0; i < scene->entities.length; i += 1)
    {
      Ignis_Entity* entity = Ignis_EntityArrayGetPointer(&scene->entities, i);
      if (entity->type == Ignis_EntityType_Actor)
      {
        // -- AlNov 3 January 2026: @TODO How to draw seleceted entity
        Ignis_R_RenderEntity(camera, entity, Ignis_EntityIDEqual(entity->id, scene->selected_entity_id));
      }
    }
  }
  RHI_EndRenderPass(command_buffer, 0);

  RHI_ColorTarget color_target = {
    .texture = swapchain_texture,
    .load_operation = RHI_AttachmentLoadOperation_Load,
    .store_operation = RHI_AttachmentStoreOperation_Store,
  };

  RHI_DepthStencilTarget depth_target = {
    .texture = _ignis_r_state.depth_texture,
    .load_operation = RHI_AttachmentLoadOperation_Load,
    .store_operation = RHI_AttachmentStoreOperation_DontCare,
  };

  Vei_BeginPoint(RenderGridPass);
  RHI_BeginRenderPass(command_buffer, 1, &color_target, &depth_target);
  {
    Ignis_R_RenderGrid(scene, color_target, depth_target);
  }
  RHI_EndRenderPass(command_buffer, 0);
  Vei_EndPoint(RenderGridPass);
}

func void
Ignis_R_RenderUI(UI_DrawCommandArray commands)
{
  RHI_CommandBuffer command_buffer = _ignis_r_state.command_buffer;
  RHI_Texture swapchain_texture = _ignis_r_state.swapchain;
  RHI_Buffer data_buffer = _ignis_r_state.data_buffer;

  RHI_ColorTarget color_target = {
    .texture = swapchain_texture,
    .load_operation = RHI_AttachmentLoadOperation_Load,
    .store_operation = RHI_AttachmentStoreOperation_Store,
  };

  RHI_BeginRenderPass(command_buffer, 1, &color_target, 0);
  {
    RectI32 viewport = {
      .x = 0,
      .y = 0,
      .w = _ignis_r_state.window->size.w,
      .h = _ignis_r_state.window->size.h,
    };
    RectI32 scissor = viewport;
    RHI_SetViewport(command_buffer, viewport);
    RHI_SetScissor(command_buffer, scissor);

    for (I32 i = 0; i < commands.length; i += 1)
    {
      UI_DrawCommand* command = UI_DrawCommandArrayGetPointer(&commands, i);

      switch (command->type)
      {
        default: {}break;

        case UI_DrawCommandType_Rectangle:
        {
          D_DrawRect(command_buffer, data_buffer, viewport, command->rectangle.bound, command->rectangle.radius, command->rectangle.color, command->rectangle.border_color);
        } break;

        case UI_DrawCommandType_Text:
        {
          D_DrawText(command_buffer, data_buffer, _ignis_r_state.texture_sampler, viewport, command->text.font, command->text.content, command->text.font_size, command->text.position, command->text.color);
        } break;

        // --AlNov 24 December 2025: @TODO
        // Doesn't save previous scissor rectangle, so it is lost.
        case UI_DrawCommandType_ScissorBegin:
        {
          RHI_SetScissor(command_buffer, (RectI32){
            .x = (I32)command->scissor.bound.x,
            .y = (I32)command->scissor.bound.y,
            .w = (I32)command->scissor.bound.w,
            .h = (I32)command->scissor.bound.h,
          });
        } break;

        case UI_DrawCommandType_ScissorEnd:
        {
          RHI_SetScissor(command_buffer, (RectI32){
            .x = 0,
            .y = 0,
            .w = (I32)_ignis_r_state.window->size.w,
            .h = (I32)_ignis_r_state.window->size.h,
          });
        } break;
      }
    }
  }
  RHI_EndRenderPass(command_buffer, 0);
}

func void
Ignis_R_RenderGrid(Ignis_Scene* scene, RHI_ColorTarget color, RHI_DepthStencilTarget depth)
{
  RHI_CommandBuffer command_buffer = _ignis_r_state.command_buffer;
  RHI_Buffer data_buffer = _ignis_r_state.data_buffer;
  Ignis_Entity* camera = Ignis_GetCamera(scene);

  struct
  {
    Mat4F32 view_matrix;
    Mat4F32 projection_matrix;
  } grid_global_vertex_data;
  grid_global_vertex_data.view_matrix = MakeLookAtMat4F32(
    camera->transform.translation,
    AddVec3F32(camera->transform.translation, camera->camera.front),
    camera->camera.up
  );
  grid_global_vertex_data.projection_matrix = MakePerspectiveMat4F32(
    45.0f, (F32)_ignis_r_state.window->size.x/(F32)_ignis_r_state.window->size.y,
    0.1f, 100.0f
  );
  struct
  {
    Vec3F32 position;
    F32 grid_scale;
  } grid_instance_vertex_data;
  grid_instance_vertex_data.position = MakeVec3F32(camera->transform.translation.x, 0.0f, camera->transform.translation.z);
  grid_instance_vertex_data.grid_scale = 2000.0f;

  struct
  {
    Vec4F32 color;
  } grid_global_fragment_data;
  grid_global_fragment_data.color = RGBAFromHex(0x95B8D1AA);

  U64 grid_global_vertex_data_offset = RHI_PushBuffer(data_buffer, (U8*)&grid_global_vertex_data, sizeof(grid_global_vertex_data));
  U64 grid_instance_vertex_data_offset = RHI_PushBuffer(data_buffer, (U8*)&grid_instance_vertex_data, sizeof(grid_instance_vertex_data));
  U64 grid_global_fragment_data_offset = RHI_PushBuffer(data_buffer, (U8*)&grid_global_fragment_data, sizeof(grid_global_fragment_data));

  RHI_UniformBufferBindingInfo grid_vertex_shader_global_uniform = {
    .buffer = data_buffer,
    .offset = grid_global_vertex_data_offset,
    .size = sizeof(grid_global_vertex_data),
  };
  RHI_UniformBufferBindingInfo grid_vertex_shader_instance_uniform = {
    .buffer = data_buffer,
    .offset = grid_instance_vertex_data_offset,
    .size = sizeof(grid_instance_vertex_data),
  };
  RHI_UniformBufferBindingInfo grid_fragment_shader_global_uniform = {
    .buffer = data_buffer,
    .offset = grid_global_fragment_data_offset,
    .size = sizeof(grid_global_fragment_data),
  };

  RHI_BindGraphicsPipeline(command_buffer, _ignis_r_state.grid_pipeline);
  RHI_BindGlobalVertexShaderData(command_buffer, 1, &grid_vertex_shader_global_uniform, 0, 0);
  RHI_BindInstanceVertexShaderData(command_buffer, 1, &grid_vertex_shader_instance_uniform, 0, 0);
  RHI_BindGlobalFragmentShaderData(command_buffer, 1, &grid_fragment_shader_global_uniform, 0, 0);
  RHI_DrawPrimitives(command_buffer, 6, 1, 0, 0);
}

func void
Ignis_R_ShadowMapPass(Ignis_Scene* scene, Ignis_Entity* entity)
{
  RHI_CommandBuffer command_buffer = _ignis_r_state.command_buffer;
  RHI_Buffer buffer = _ignis_r_state.data_buffer;
  Vec3F32 light_position = MakeVec3F32(10.0f, -10.0f, 0.0f);
  Vec3F32 light_direction = NormalizeVec3F32(MakeVec3F32(1.0f, -1.0f, -1.0f));

  for (AST_GeometryListNode* geometry_node = entity->actor.mesh.geometry_list.first; geometry_node; geometry_node = geometry_node->next)
  {
    AST_Geometry* geometry = &geometry_node->data;

    Mat4F32 view_matrix = MakeLookAtMat4F32(
      light_position,
      AddVec3F32(light_position, light_direction),
      MakeVec3F32(0.0f, 1.0f, 0.0f)
    );
    Mat4F32 projection_matrix = MakePerspectiveMat4F32(
      45.0f, (F32)_ignis_r_state.window->size.x/(F32)_ignis_r_state.window->size.y,
      0.1f, 100.0f
    );

    struct
    {
      Mat4F32 mvp;
    } instance_vertex_data;
    instance_vertex_data.mvp = MulMat4F32(projection_matrix, MulMat4F32(view_matrix, Mat4F32FromTransform(entity->transform)));

    U64 instance_vertex_data_offset = RHI_PushBuffer(buffer, (U8*)&instance_vertex_data, sizeof(instance_vertex_data));
    U64 vertex_data_offset = RHI_PushBuffer(buffer, (U8*)&geometry->vertecies, geometry->vertecies_count*sizeof(AST_Vertex));
    U64 index_data_offset = RHI_PushBuffer(buffer, (U8*)&geometry->index_data, geometry->index_size*geometry->index_count);

    RHI_UniformBufferBindingInfo vertex_shader_instance_uniform = {
      .buffer = buffer,
      .offset = instance_vertex_data_offset,
      .size = sizeof(instance_vertex_data),
    };

    RHI_BindGraphicsPipeline(command_buffer, _ignis_r_state.shadow_map_pipeline);

    RHI_BindInstanceVertexShaderData(command_buffer, 1, &vertex_shader_instance_uniform, 0, 0);
    RHI_BindVertexBuffer(command_buffer, buffer, vertex_data_offset);
    RHI_BindIndexBuffer(command_buffer, buffer, index_data_offset, RHI_IndexSize_U16);
    RHI_DrawIndexedPrimitives(command_buffer, geometry->index_count, 1, 0, 0, 0);
  }
}

func void
Ignis_R_RenderEntityPrepass(Ignis_Entity* camera, Ignis_Entity* entity)
{
  Vei_BeginPoint(Ignis_R_RenderEntityPrepass);
  RHI_CommandBuffer command_buffer = _ignis_r_state.command_buffer;
  RHI_Buffer buffer = _ignis_r_state.data_buffer;

  for (AST_GeometryListNode* geometry_node = entity->actor.mesh.geometry_list.first; geometry_node; geometry_node = geometry_node->next)
  {
    AST_Geometry* geometry = &geometry_node->data;

    Mat4F32 view_matrix = MakeLookAtMat4F32(
      camera->transform.translation,
      AddVec3F32(camera->transform.translation, camera->camera.front),
      camera->camera.up
    );
    Mat4F32 projection_matrix = MakePerspectiveMat4F32(
      45.0f, (F32)_ignis_r_state.window->size.x/(F32)_ignis_r_state.window->size.y,
      0.1f, 100.0f
    );

    struct
    {
      Mat4F32 mvp;
    } mesh_instance_vertex_data;
    mesh_instance_vertex_data.mvp = MulMat4F32(projection_matrix, MulMat4F32(view_matrix, Mat4F32FromTransform(entity->transform)));

    U64 mesh_instance_vertex_data_offset = RHI_PushBuffer(buffer, (U8*)&mesh_instance_vertex_data, sizeof(mesh_instance_vertex_data));

    U64 mesh_vertex_data_offset = RHI_PushBuffer(buffer, (U8*)geometry->vertecies, geometry->vertecies_count*sizeof(AST_Vertex));
    U64 mesh_index_data_offset = RHI_PushBuffer(buffer, geometry->index_data, geometry->index_size*geometry->index_count);

    RHI_UniformBufferBindingInfo mesh_vertex_shader_instance_uniform = {
      .buffer = buffer,
      .offset = mesh_instance_vertex_data_offset,
      .size = sizeof(mesh_instance_vertex_data),
    };

    RHI_BindGraphicsPipeline(command_buffer, _ignis_r_state.depth_prepass_pipeline);

    RHI_BindInstanceVertexShaderData(command_buffer, 1, &mesh_vertex_shader_instance_uniform, 0, 0);
    RHI_BindVertexBuffer(command_buffer, buffer, mesh_vertex_data_offset);
    RHI_BindIndexBuffer(command_buffer, buffer, mesh_index_data_offset, RHI_IndexSize_U16);
    RHI_DrawIndexedPrimitives(command_buffer, geometry->index_count, 1, 0, 0, 0);
  }
  Vei_EndPoint(Ignis_R_RenderEntityPrepass);
}

func void
Ignis_R_RenderEntity(Ignis_Entity* camera, Ignis_Entity* entity, B32 selected)
{
  Vei_BeginPoint(Ignis_R_RenderEntity);
  RHI_CommandBuffer command_buffer = _ignis_r_state.command_buffer;
  RHI_Buffer buffer = _ignis_r_state.data_buffer;

  for (AST_GeometryListNode* geometry_node = entity->actor.mesh.geometry_list.first; geometry_node; geometry_node = geometry_node->next)
  {
    Vei_BeginPoint(PrepareVertexData);
    AST_Geometry* geometry = &geometry_node->data;

    Mat4F32 view_matrix = MakeLookAtMat4F32(
      camera->transform.translation,
      AddVec3F32(camera->transform.translation, camera->camera.front),
      camera->camera.up
    );
    Mat4F32 projection_matrix = MakePerspectiveMat4F32(
      45.0f, (F32)_ignis_r_state.window->size.x/(F32)_ignis_r_state.window->size.y,
      0.1f, 100.0f
    );

    U64 mesh_vertex_data_offset = RHI_PushBuffer(buffer, (U8*)geometry->vertecies, geometry->vertecies_count*sizeof(AST_Vertex));
    U64 mesh_index_data_offset = RHI_PushBuffer(buffer, geometry->index_data, geometry->index_size*geometry->index_count);

    struct
    {
      Mat4F32 mvp;
      Mat4F32 bone_transform[64];
    } mesh_instance_vertex_data;
    mesh_instance_vertex_data.mvp = MulMat4F32(projection_matrix, MulMat4F32(view_matrix, Mat4F32FromTransform(entity->transform)));
    for (I32 i = 0; i < entity->actor.mesh.skeleton.joints.length; i += 1)
    {
      Joint joint = JointArrayGet(&entity->actor.mesh.skeleton.joints, i);
      mesh_instance_vertex_data.bone_transform[i] = Mat4F32FromTransform(joint.global_transform);
    }

    U64 mesh_instance_vertex_data_offset = RHI_PushBuffer(buffer, (U8*)&mesh_instance_vertex_data, sizeof(mesh_instance_vertex_data));

    RHI_UniformBufferBindingInfo mesh_vertex_shader_instance_uniform = {
      .buffer = buffer,
      .offset = mesh_instance_vertex_data_offset,
      .size = sizeof(mesh_instance_vertex_data),
    };
    Vei_EndPoint(PrepareVertexData);

    struct
    {
      Vec3F32 light_direction;
      F32 light_direction_padding;
      Vec3F32 ambient_color;
      F32 ambient_color_padding;
      Vec3F32 diffuse_color;
    } mesh_global_fragment_data = {
      .light_direction = NormalizeVec3F32(MakeVec3F32(1.0f, -1.0f, -1.0f)),
      .ambient_color = MakeVec3F32(0.1f, 0.1f, 0.1f),
      .diffuse_color = (selected) ? MakeVec3F32(0.13f, 0.78f, 0.09f) : MakeVec3F32(0.87f, 0.85f, 0.42f),
    };

    U64 mesh_global_fragment_data_offset = RHI_PushBuffer(buffer, (U8*)&mesh_global_fragment_data, sizeof(mesh_global_fragment_data));

    RHI_UniformBufferBindingInfo mesh_fragment_shader_global_uniform = {
      .buffer = buffer,
      .offset = mesh_global_fragment_data_offset,
      .size = sizeof(mesh_global_fragment_data),
    };

    RHI_SamplerBindingInfo mesh_fragment_shader_global_samplers[] = {
    };

    RHI_BindGraphicsPipeline(command_buffer, _ignis_r_state.mesh_pipeline);

    RHI_BindInstanceVertexShaderData(command_buffer, 1, &mesh_vertex_shader_instance_uniform, 0, 0);
    RHI_BindGlobalFragmentShaderData(command_buffer, 1, &mesh_fragment_shader_global_uniform, 0, 0);
    RHI_BindVertexBuffer(command_buffer, buffer, mesh_vertex_data_offset);
    RHI_BindIndexBuffer(command_buffer, buffer, mesh_index_data_offset, RHI_IndexSize_U16);
    RHI_DrawIndexedPrimitives(command_buffer, geometry->index_count, 1, 0, 0, 0);
  }
  Vei_EndPoint(Ignis_R_RenderEntity);
}
