#include "ignis_r.h"

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"

func void
Ignis_R_Init(R_RendererType type, OS_Window* window)
{
  R_Init(type, window);

  R_BufferUsageFlags triangle_buffer_usage_flags = R_BUFFER_USAGE_FLAG_VERTEX|R_BUFFER_USAGE_FLAG_INDEX|R_BUFFER_USAGE_FLAG_UNIFORM;

  _ignis_r_state.arena           = AllocateArena(Megabytes(32));
  _ignis_r_state.window          = window;
  _ignis_r_state.data_buffer     = R_CreateBuffer(Megabytes(64), triangle_buffer_usage_flags, R_BUFFER_PROPERTY_FLAG_HOST_COHERENT);
  _ignis_r_state.transfer_buffer = R_CreateBuffer(Megabytes(128), R_BUFFER_USAGE_FLAG_TRANSFER, R_BUFFER_PROPERTY_FLAG_HOST_COHERENT);

  Ignis_R_PreparePipelines();
  Ignis_R_PrepareTextures();
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

  // 3D Line Pipeline
  {
    R_Shader line_vertex_shader = R_CreateShader(
      _ignis_r_state.arena,
      &(R_ShaderCreateInfo){
        .file_name = Str8C("./data/shaders/line3d.vs.glsl"),
        .type = R_SHADER_TYPE_VERTEX,
        .global_uniforms_count = 1,
        .instance_uniforms_count = 1,
      }
    );

    R_Shader line_fragment_shader = R_CreateShader(
      _ignis_r_state.arena,
      &(R_ShaderCreateInfo){
        .file_name = Str8C("./data/shaders/line3d.fs.glsl"),
        .type = R_SHADER_TYPE_FRAGMENT,
      }
    );

    _ignis_r_state.line_3d_pipeline = R_CreateGraphicsPipeline(
      &(R_GraphicsPipelineCreateInfo){
        .vertex_shader = line_vertex_shader,
        .fragment_shader = line_fragment_shader,
        .color_targets_count = 1,
        .color_target_infos = &(R_GraphicsPipelineColorTargetInfo){
          .format = R_GetSwapchainTextureFormat(),
        },
        .depth_stencil_state = {
          .depth_test_enable = 0,
          .depth_write_enable = 0,
          .depth_compare_operation = R_COMPARE_OPERATION_GREATER,
          .depth_target_format = R_GetTextureFormat(_ignis_r_state.depth_texture),
        },
      }
    );
  }

  // Font Pipeline
  {
    R_Shader font_vertex_shader = R_CreateShader(
      _ignis_r_state.arena,
      &(R_ShaderCreateInfo){
        .file_name = Str8C("./data/shaders/font.vs.glsl"),
        .type = R_SHADER_TYPE_VERTEX,
        .global_uniforms_count = 1,
      }
    );

    R_Shader font_fragment_shader = R_CreateShader(
      _ignis_r_state.arena,
      &(R_ShaderCreateInfo){
        .file_name = Str8C("./data/shaders/font.fs.glsl"),
        .type = R_SHADER_TYPE_FRAGMENT,
        .global_samplers_count = 1,
      }
    );

    R_VertexAttribute font_vertex_attributes[] = {
      {
        .location = 0,
        .format = R_VERTEX_ATTRIBUTE_FORMAT_VEC2F32,
        .offset = offsetof(TextVertex, position),
      },
      {
        .location = 1,
        .format = R_VERTEX_ATTRIBUTE_FORMAT_VEC2F32,
        .offset = offsetof(TextVertex, uv),
      },
    };
    R_GraphicsPipelineColorTargetInfo font_pipeline_color_target = {
      .format = R_GetSwapchainTextureFormat(),
      .blend_enable = 1,
    };
    R_GraphicsPipelineCreateInfo font_pipeline_info = {
      .vertex_shader = font_vertex_shader,
      .fragment_shader = font_fragment_shader,
      .vertex_attributes_count = CountArrayElements(font_vertex_attributes),
      .vertex_attributes = font_vertex_attributes,
      .color_targets_count = 1,
      .color_target_infos = &font_pipeline_color_target,
      .depth_stencil_state = {
        .depth_test_enable = 0,
      },
    };
    _ignis_r_state.font_pipeline = R_CreateGraphicsPipeline(&font_pipeline_info);
  }

  // Square Pipeline
  {
    R_Shader square_vertex_shader = R_CreateShader(
      _ignis_r_state.arena,
      &(R_ShaderCreateInfo){
        .file_name = Str8C("./data/shaders/square.vs.glsl"),
        .type = R_SHADER_TYPE_VERTEX,
        .global_uniforms_count = 1,
        .instance_uniforms_count = 1,
      }
    );
    R_Shader square_fragment_shader = R_CreateShader(
      _ignis_r_state.arena,
      &(R_ShaderCreateInfo){
        .file_name = Str8C("./data/shaders/square.fs.glsl"),
        .type = R_SHADER_TYPE_FRAGMENT,
        .global_uniforms_count = 0,
        .instance_uniforms_count = 1,
      }
    );

    R_GraphicsPipelineColorTargetInfo square_pipeline_color_target_infos[] = {
      {
        .format = R_GetSwapchainTextureFormat(),
        .blend_enable = 1,
      },
    };

    R_GraphicsPipelineCreateInfo square_pipeline_info = {
      .vertex_shader = square_vertex_shader,
      .fragment_shader = square_fragment_shader,
      .color_targets_count = CountArrayElements(square_pipeline_color_target_infos),
      .color_target_infos = square_pipeline_color_target_infos,
      .depth_stencil_state = {
        .depth_test_enable = 0,
        .depth_write_enable = 0,
        .depth_compare_operation = R_COMPARE_OPERATION_GREATER,
        .depth_target_format = R_GetTextureFormat(_ignis_r_state.depth_texture),
      },
    };
    _ignis_r_state.square_pipeline = R_CreateGraphicsPipeline(&square_pipeline_info);
  }

  // Mesh Pipeline
  {
    R_Shader mesh_vertex_shader = R_CreateShader(
      _ignis_r_state.arena,
      &(R_ShaderCreateInfo){
        .file_name = Str8C("./data/shaders/mesh.vs.glsl"),
        .type = R_SHADER_TYPE_VERTEX,
        .global_uniforms_count = 1,
        .instance_uniforms_count = 1,
      }
    );
    R_Shader mesh_fragment_shader = R_CreateShader(
      _ignis_r_state.arena,
      &(R_ShaderCreateInfo){
        .file_name = Str8C("./data/shaders/mesh.fs.glsl"),
        .type = R_SHADER_TYPE_FRAGMENT,
        .global_uniforms_count = 0,
        .instance_uniforms_count = 1,
        .instance_samplers_count = 2,
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
      {
        .format = R_GetTextureFormat(_ignis_r_state.test_texture),
        .blend_enable = 0,
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
