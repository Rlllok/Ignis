#include "ignis_r.h"

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"

func void
Ignis_R_Init(R_RendererType type, OS_Window* window)
{
  R_Init(type, window);
  D_Init(Megabytes(16));

  R_BufferUsageFlags triangle_buffer_usage_flags = R_BUFFER_USAGE_FLAG_VERTEX|R_BUFFER_USAGE_FLAG_INDEX|R_BUFFER_USAGE_FLAG_UNIFORM;

  _ignis_r_state.arena           = AllocateArena(Megabytes(32));
  _ignis_r_state.window          = window;
  _ignis_r_state.data_buffer     = R_CreateBuffer(Megabytes(64), triangle_buffer_usage_flags, R_BUFFER_PROPERTY_FLAG_HOST_COHERENT);
  _ignis_r_state.transfer_buffer = R_CreateBuffer(Megabytes(128), R_BUFFER_USAGE_FLAG_TRANSFER, R_BUFFER_PROPERTY_FLAG_HOST_COHERENT);
  _ignis_r_state.command_buffer  = R_GetCommandBuffer();

  Ignis_R_PrepareTextures();
  Ignis_R_PreparePipelines();
}

func R_Texture
Ignis_R_CreateLoadTexture(R_Buffer buffer, Str8 path, R_TextureFormat format)
{
  R_Texture result = {0};

  I32 tex_width = 0;
  I32 tex_height = 0;
  I32 tex_channels = 0;
  U8* tex_pixels = stbi_load(CFromStr8(path), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);

  if (!tex_pixels)
  {
    LOG_ERROR("Cannot load texture %s\n", CFromStr8(path));
  }
  I32 texture_size = tex_width * tex_height * 4;

  result = R_CreateTexture(&(R_TextureCreateInfo){
    .type = R_TEXTURE_TYPE_2D,
    .format = format,
    .usage_flags = R_TEXTURE_USAGE_FLAG_SAMPLED | R_TEXTURE_USAGE_FLAG_TRANSFER_DST,
    .width = tex_width,
    .height = tex_height,
    .depth = 1,
    .num_levels = 1,
  });

  U64 texture_offset = R_PushBuffer(buffer, tex_pixels, texture_size);
  R_CopyBufferToTexture(0, buffer, texture_offset, texture_size, result);

  return result;
}

func void
Ignis_R_PreparePipelines()
{
  // --AlNov: Word Grid
  {
    R_Shader grid_vertex_shader = R_CreateShader(
      _ignis_r_state.arena,
      &(R_ShaderCreateInfo){
        .file_name = Str8C("./data/shaders/grid.vs.glsl"),
        .type = R_SHADER_TYPE_VERTEX,
        .global_uniforms_count = 1,
        .instance_uniforms_count = 1,
      }
    );

    R_Shader grid_fragment_shader = R_CreateShader(
      _ignis_r_state.arena,
      &(R_ShaderCreateInfo){
        .file_name = Str8C("./data/shaders/grid.fs.glsl"),
        .type = R_SHADER_TYPE_FRAGMENT,
        .global_uniforms_count = 1,
        .instance_uniforms_count = 0,
      }
    );

    _ignis_r_state.grid_pipeline = R_CreateGraphicsPipeline(
      &(R_GraphicsPipelineCreateInfo){
        .vertex_shader = grid_vertex_shader,
        .fragment_shader = grid_fragment_shader,
        .color_targets_count = 1,
        .color_target_infos = &(R_GraphicsPipelineColorTargetInfo){
          .format = R_GetSwapchainTextureFormat(),
          .blend_enable = 1,
        },
        .depth_stencil_state = {
          .depth_test_enable = 1,
          .depth_write_enable = 0,
          .depth_compare_operation = R_COMPARE_OPERATION_GREATER,
          .depth_target_format = R_GetTextureFormat(_ignis_r_state.depth_texture),
        },
      }
    );
  }


  // Mesh Pipeline
  {
    R_Shader mesh_vertex_shader = R_CreateShader(
      _ignis_r_state.arena,
      &(R_ShaderCreateInfo){
        .file_name = Str8C("./data/shaders/ignis/mesh.vs.glsl"),
        .type = R_SHADER_TYPE_VERTEX,
        .instance_uniforms_count = 1,
      }
    );
    R_Shader mesh_fragment_shader = R_CreateShader(
      _ignis_r_state.arena,
      &(R_ShaderCreateInfo){
        .file_name = Str8C("./data/shaders/ignis/mesh.fs.glsl"),
        .type = R_SHADER_TYPE_FRAGMENT,
        .global_uniforms_count = 1,
      }
    );

    R_VertexAttribute mesh_vertex_attributes[] = {
      {
        .location = 0,
        .format = R_VERTEX_ATTRIBUTE_FORMAT_VEC3F32,
        .offset = offsetof(AST_Vertex, position),
      },
      {
        .location = 1,
        .format = R_VERTEX_ATTRIBUTE_FORMAT_VEC3F32,
        .offset = offsetof(AST_Vertex, normal),
      },
      {
        .location = 2,
        .format = R_VERTEX_ATTRIBUTE_FORMAT_VEC3F32,
        .offset = offsetof(AST_Vertex, tangent),
      },
      {
        .location = 3,
        .format = R_VERTEX_ATTRIBUTE_FORMAT_VEC2F32,
        .offset = offsetof(AST_Vertex, uv),
      },
      {
        .location = 4,
        .format = R_VERTEX_ATTRIBUTE_FORMAT_VEC4I32,
        .offset = offsetof(AST_Vertex, joint_ids),
      },
      {
        .location = 5,
        .format = R_VERTEX_ATTRIBUTE_FORMAT_VEC4F32,
        .offset = offsetof(AST_Vertex, joint_weights),
      },
    };

    R_GraphicsPipelineColorTargetInfo mesh_pipeline_color_target_infos[] = {
      {
        .format = R_GetSwapchainTextureFormat(),
        .blend_enable = 1,
      },
    };
    R_GraphicsPipelineCreateInfo mesh_pipeline_info = {
      .vertex_shader = mesh_vertex_shader,
      .fragment_shader = mesh_fragment_shader,
      .vertex_attributes_count = CountArrayElements(mesh_vertex_attributes),
      .vertex_attributes = mesh_vertex_attributes,
      .color_targets_count = CountArrayElements(mesh_pipeline_color_target_infos),
      .color_target_infos = mesh_pipeline_color_target_infos,
      .depth_stencil_state = {
        .depth_test_enable = 1,
        .depth_write_enable = 1,
        .depth_compare_operation = R_COMPARE_OPERATION_GREATER,
        .depth_target_format = R_GetTextureFormat(_ignis_r_state.depth_texture),
      },
    };
    _ignis_r_state.mesh_pipeline = R_CreateGraphicsPipeline(&mesh_pipeline_info);
  }
}

func void
Ignis_R_PrepareTextures()
{
  R_Buffer transfer_buffer = _ignis_r_state.transfer_buffer;

  _ignis_r_state.texture_sampler = R_CreateTextureSampler(
    &(R_TextureSamplerCreateInfo){
      .mag_filter = R_FILTER_TYPE_LINEAR,
      .min_filter = R_FILTER_TYPE_LINEAR,
      .address_mode_u = R_SAMPLER_ADDRESS_MODE_REPEAT,
      .address_mode_v = R_SAMPLER_ADDRESS_MODE_REPEAT,
      .address_mode_w = R_SAMPLER_ADDRESS_MODE_REPEAT,
      .mipmap_mode = R_SAMPLER_MIPMAP_MODE_LINEAR,
    }
  );

  _ignis_r_state.default_color_texture = Ignis_R_CreateLoadTexture(transfer_buffer, Str8C("./data/uv_checker.png"), R_TEXTURE_FORMAT_R8G8B8A8_SRGB);
  _ignis_r_state.mesh_color_texture    = Ignis_R_CreateLoadTexture(transfer_buffer, Str8C("./data/sphere_gltf/RockyColor.png"), R_TEXTURE_FORMAT_R8G8B8A8_SRGB);
  _ignis_r_state.mesh_normal_texture   = Ignis_R_CreateLoadTexture(transfer_buffer, Str8C("./data/sphere_gltf/RockyNormal.png"), R_TEXTURE_FORMAT_R8G8B8A8_UNORM);

  _ignis_r_state.test_texture = R_CreateTexture(
    &(R_TextureCreateInfo){
      .type = R_TEXTURE_TYPE_2D,
      .format = R_TEXTURE_FORMAT_R16_UINT,
      .usage_flags = R_TEXTURE_USAGE_FLAG_COLOR_ATTACHMENT | R_TEXTURE_USAGE_FLAG_TRANSFER_SRC,
      .width = _ignis_r_state.window->size.w,
      .height = _ignis_r_state.window->size.h,
      .depth = 1,
      .num_levels = 1
    }
  );

  _ignis_r_state.depth_texture = R_CreateTexture(
    &(R_TextureCreateInfo){
      .type = R_TEXTURE_TYPE_2D,
      .format = R_TEXTURE_FORMAT_D16_UNORM,
      .usage_flags = R_TEXTURE_USAGE_FLAG_DEPTH_STENCIL_ATTACHMENT,
      .width = _ignis_r_state.window->size.w,
      .height = _ignis_r_state.window->size.h,
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
    R_VK_HandleResize(_ignis_r_state.window);

    R_VK_DestroyTexture(_ignis_r_state.depth_texture);
    R_TextureCreateInfo depth_texture_info = {
      .type = R_TEXTURE_TYPE_2D,
      .format = R_TEXTURE_FORMAT_D16_UNORM,
      .usage_flags = R_TEXTURE_USAGE_FLAG_DEPTH_STENCIL_ATTACHMENT,
      .width = _ignis_r_state.window->size.w,
      .height = _ignis_r_state.window->size.h,
      .depth = 1,
      .num_levels = 1
    };
    _ignis_r_state.depth_texture = R_CreateTexture(&depth_texture_info);

    R_VK_DestroyTexture(_ignis_r_state.test_texture);
    R_TextureCreateInfo test_texture_info = {
      .type = R_TEXTURE_TYPE_2D,
      .format = R_TEXTURE_FORMAT_R16_UINT,
      .usage_flags = R_TEXTURE_USAGE_FLAG_COLOR_ATTACHMENT | R_TEXTURE_USAGE_FLAG_TRANSFER_SRC | R_TEXTURE_USAGE_FLAG_TRANSFER_DST,
      .width = _ignis_r_state.window->size.w,
      .height = _ignis_r_state.window->size.h,
      .depth = 1,
      .num_levels = 1
    };
    _ignis_r_state.test_texture = R_CreateTexture(&test_texture_info);
  }
}

// -------------------------------------------------------------------
// -- Render ---------------------------------------------------------
func void
Ignis_R_BeginFrame()
{
  R_CommandBuffer command_buffer = _ignis_r_state.command_buffer;

  R_ResetBuffer(_ignis_r_state.data_buffer);
  R_ResetBuffer(_ignis_r_state.transfer_buffer);

  _ignis_r_state.swapchain = R_AcquireSwapchainTexture(_ignis_r_state.command_buffer);
  R_BeginCommandBuffer(command_buffer);
}

func void
Ignis_R_EndFrame()
{
  R_CommandBuffer command_buffer = _ignis_r_state.command_buffer;
  R_Texture swapchain_texture = _ignis_r_state.swapchain;

  R_SubmitCommandBuffer(command_buffer);
  R_PresentTexture(command_buffer, swapchain_texture);
}

func void
Ignis_R_RenderScene(Ignis_Scene* scene)
{
  R_CommandBuffer command_buffer = _ignis_r_state.command_buffer;
  R_Texture swapchain_texture = _ignis_r_state.swapchain;
  Ignis_Entity* camera = Ignis_GetCamera(scene);

  {
    RectI32 viewport = {
      .x = 0,
      .y = 0,
      .w = (I32)_ignis_r_state.window->size.x,
      .h = (I32)_ignis_r_state.window->size.y,
    };
    RectI32 scissor = viewport;
    R_SetViewport(command_buffer, viewport);
    R_SetScissor(command_buffer, scissor);

    R_ColorTarget entity_color_targets[] = {
      {
        .texture = swapchain_texture,
        .load_operation = R_ATTACHMENT_LOAD_OPERATION_CLEAR,
        .store_operation = R_ATTACHMENT_STORE_OPERATION_STORE,
        .clear_color = RGBAFromHex(0x111111FF),
      },
    };

    R_DepthStencilTarget entity_depth_target = {
      .texture = _ignis_r_state.depth_texture,
      .depth_load_operation = R_ATTACHMENT_LOAD_OPERATION_CLEAR,
      .depth_store_operation = R_ATTACHMENT_STORE_OPERATION_STORE,
      .clear_depth = 0.0f,
    };

    R_BeginRenderPass(command_buffer, CountArrayElements(entity_color_targets), entity_color_targets, &entity_depth_target);
    {
      for (I32 i = 0; i < scene->entities.length; i += 1)
      {
        Ignis_Entity* entity = Ignis_EntityArrayGetPointer(&scene->entities, i);
        if (entity->type == Ignis_EntityType_Actor)
        {
          Ignis_R_RenderEntity(camera, entity);
        }
      }
    }
    R_EndRenderPass(command_buffer, 0);

    R_ColorTarget color_target = {
      .texture = swapchain_texture,
      .load_operation = R_ATTACHMENT_LOAD_OPERATION_LOAD,
      .store_operation = R_ATTACHMENT_STORE_OPERATION_STORE,
    };

    R_DepthStencilTarget depth_target = {
      .texture = _ignis_r_state.depth_texture,
      .depth_load_operation = R_ATTACHMENT_LOAD_OPERATION_LOAD,
      .depth_store_operation = R_ATTACHMENT_STORE_OPERATION_DONT_CARE,
    };

    R_BeginRenderPass(command_buffer, 1, &color_target, &depth_target);
    {
      Ignis_R_RenderGrid(scene, color_target, depth_target);
    }
    R_EndRenderPass(command_buffer, 0);
  }
}

func void
Ignis_R_RenderUI(UI_DrawCommandArray commands)
{
  R_CommandBuffer command_buffer = _ignis_r_state.command_buffer;
  R_Texture swapchain_texture = _ignis_r_state.swapchain;
  R_Buffer data_buffer = _ignis_r_state.data_buffer;

  RectI32 viewport = {
    .x = 0,
    .y = 0,
    .w = _ignis_r_state.window->size.w,
    .h = _ignis_r_state.window->size.h,
  };
  RectI32 scissor = viewport;
  R_SetViewport(command_buffer, viewport);
  R_SetScissor(command_buffer, scissor);

  R_ColorTarget color_target = {
    .texture = swapchain_texture,
    .load_operation = R_ATTACHMENT_LOAD_OPERATION_LOAD,
    .store_operation = R_ATTACHMENT_STORE_OPERATION_STORE,
  };

  R_BeginRenderPass(command_buffer, 1, &color_target, 0);
  {
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
          R_SetScissor(command_buffer, (RectI32){
            .x = (I32)command->scissor.bound.x,
            .y = (I32)command->scissor.bound.y,
            .w = (I32)command->scissor.bound.w,
            .h = (I32)command->scissor.bound.h,
          });
        } break;

        case UI_DrawCommandType_ScissorEnd:
        {
          R_SetScissor(command_buffer, (RectI32){
            .x = 0,
            .y = 0,
            .w = (I32)_ignis_r_state.window->size.w,
            .h = (I32)_ignis_r_state.window->size.h,
          });
        } break;
      }
    }
  }
  R_EndRenderPass(command_buffer, 0);
}

func void
Ignis_R_RenderGrid(Ignis_Scene* scene, R_ColorTarget color, R_DepthStencilTarget depth)
{
  R_CommandBuffer command_buffer = _ignis_r_state.command_buffer;
  R_Buffer data_buffer = _ignis_r_state.data_buffer;
  Ignis_Entity* camera = Ignis_GetCamera(scene);

  struct
  {
    Mat4 view_matrix;
    Mat4 projection_matrix;
  } grid_global_vertex_data;
  grid_global_vertex_data.view_matrix = MakeLookAtMat4(
    camera->transform.translation,
    AddVec3(camera->transform.translation, camera->camera.front),
    camera->camera.up
  );
  grid_global_vertex_data.projection_matrix = MakePerspectiveMat4(
    45.0f, (F32)_ignis_r_state.window->size.x/(F32)_ignis_r_state.window->size.y,
    0.1f, 100.0f
  );
  struct
  {
    Vec3 position;
    F32 grid_scale;
  } grid_instance_vertex_data;
  grid_instance_vertex_data.position = MakeVec3(camera->transform.translation.x, 0.0f, camera->transform.translation.z);
  grid_instance_vertex_data.grid_scale = 2000.0f;

  struct
  {
    Vec4 color;
  } grid_global_fragment_data;
  grid_global_fragment_data.color = RGBAFromHex(0x95B8D1AA);

  U64 grid_global_vertex_data_offset = R_PushBuffer(data_buffer, (U8*)&grid_global_vertex_data, sizeof(grid_global_vertex_data));
  U64 grid_instance_vertex_data_offset = R_PushBuffer(data_buffer, (U8*)&grid_instance_vertex_data, sizeof(grid_instance_vertex_data));
  U64 grid_global_fragment_data_offset = R_PushBuffer(data_buffer, (U8*)&grid_global_fragment_data, sizeof(grid_global_fragment_data));

  R_UniformBufferBindingInfo grid_vertex_shader_global_uniform = {
    .buffer = data_buffer,
    .offset = grid_global_vertex_data_offset,
    .size = sizeof(grid_global_vertex_data),
  };
  R_UniformBufferBindingInfo grid_vertex_shader_instance_uniform = {
    .buffer = data_buffer,
    .offset = grid_instance_vertex_data_offset,
    .size = sizeof(grid_instance_vertex_data),
  };
  R_UniformBufferBindingInfo grid_fragment_shader_global_uniform = {
    .buffer = data_buffer,
    .offset = grid_global_fragment_data_offset,
    .size = sizeof(grid_global_fragment_data),
  };

  R_BindGraphicsPipeline(command_buffer, _ignis_r_state.grid_pipeline);
  R_BindGlobalVertexShaderData(command_buffer, 1, &grid_vertex_shader_global_uniform, 0, 0);
  R_BindInstanceVertexShaderData(command_buffer, 1, &grid_vertex_shader_instance_uniform, 0, 0);
  R_BindGlobalFragmentShaderData(command_buffer, 1, &grid_fragment_shader_global_uniform, 0, 0);
  R_DrawPrimitives(command_buffer, 6, 1, 0, 0);
}

func void
Ignis_R_RenderEntity(Ignis_Entity* camera, Ignis_Entity* entity)
{
  R_CommandBuffer command_buffer = _ignis_r_state.command_buffer;
  R_Buffer buffer = _ignis_r_state.data_buffer;

  for (AST_GeometryListNode* geometry_node = entity->actor.mesh.geometry_list.first; geometry_node; geometry_node = geometry_node->next)
  {
    AST_Geometry* geometry = &geometry_node->data;

    Mat4F32 view_matrix = MakeLookAtMat4(
      camera->transform.translation,
      AddVec3(camera->transform.translation, camera->camera.front),
      camera->camera.up
    );
    Mat4F32 projection_matrix = MakePerspectiveMat4(
      45.0f, (F32)_ignis_r_state.window->size.x/(F32)_ignis_r_state.window->size.y,
      0.1f, 100.0f
    );

    U64 mesh_vertex_data_offset = R_PushBuffer(buffer, (U8*)geometry->vertecies, geometry->vertecies_count*sizeof(AST_Vertex));
    U64 mesh_index_data_offset = R_PushBuffer(buffer, geometry->index_data, geometry->index_size*geometry->index_count);

    struct
    {
      Mat4 mvp;
      Mat4 bone_transform[64];
    } mesh_instance_vertex_data;
    mesh_instance_vertex_data.mvp = MulMat4F32(projection_matrix, MulMat4F32(view_matrix, Mat4F32FromTransform(entity->transform)));
    for (I32 i = 0; i < entity->actor.mesh.skeleton.joints.length; i += 1)
    {
      Joint joint = JointArrayGet(&entity->actor.mesh.skeleton.joints, i);
      mesh_instance_vertex_data.bone_transform[i] = Mat4F32FromTransform(joint.global_transform);
    }

    U64 mesh_instance_vertex_data_offset = R_PushBuffer(buffer, (U8*)&mesh_instance_vertex_data, sizeof(mesh_instance_vertex_data));

    R_UniformBufferBindingInfo mesh_vertex_shader_instance_uniform = {
      .buffer = buffer,
      .offset = mesh_instance_vertex_data_offset,
      .size = sizeof(mesh_instance_vertex_data),
    };

    struct
    {
      Vec3F32 light_direction;
      F32 light_direction_padding;
      Vec3F32 ambient_color;
    } mesh_global_fragment_data = {
      .light_direction = NormalizeVec3F32(MakeVec3F32(1.0f, -1.0f, -1.0f)),
      .ambient_color = MakeVec3F32(0.05f, 0.05f, 0.05f),
    };

    U64 mesh_global_fragment_data_offset = R_PushBuffer(buffer, (U8*)&mesh_global_fragment_data, sizeof(mesh_global_fragment_data));

    R_UniformBufferBindingInfo mesh_fragment_shader_global_uniform = {
      .buffer = buffer,
      .offset = mesh_global_fragment_data_offset,
      .size = sizeof(mesh_global_fragment_data),
    };

    R_BindGraphicsPipeline(command_buffer, _ignis_r_state.mesh_pipeline);
    R_BindInstanceVertexShaderData(command_buffer, 1, &mesh_vertex_shader_instance_uniform, 0, 0);
    R_BindGlobalFragmentShaderData(command_buffer, 1, &mesh_fragment_shader_global_uniform, 0, 0);
    R_BindVertexBuffer(command_buffer, buffer, mesh_vertex_data_offset);
    R_BindIndexBuffer(command_buffer, buffer, mesh_index_data_offset, R_INDEX_SIZE_U16);
    R_DrawIndexedPrimitives(command_buffer, geometry->index_count, 1, 0, 0, 0);
  }
}
