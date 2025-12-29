#include "draw.h"
#include "base/base_core.h"
#include "base/base_math.h"
#include "base/base_memory.h"

func void
D_Init(U64 arena_size)
{
  _d_state.arena = AllocateArena(arena_size);
  D_PreparePipelines();
}

func void
D_PreparePipelines()
{
  // 3D Line Pipeline
  {
    R_Shader line_vertex_shader = R_CreateShader(
      _d_state.arena,
      &(R_ShaderCreateInfo){
        .file_name = Str8C("./data/shaders/line3d.vs.glsl"),
        .type = R_SHADER_TYPE_VERTEX,
        .global_uniforms_count = 1,
        .instance_uniforms_count = 1,
      }
    );

    R_Shader line_fragment_shader = R_CreateShader(
      _d_state.arena,
      &(R_ShaderCreateInfo){
        .file_name = Str8C("./data/shaders/line3d.fs.glsl"),
        .type = R_SHADER_TYPE_FRAGMENT,
      }
    );

    _d_state.line_3d_pipeline = R_CreateGraphicsPipeline(
      &(R_GraphicsPipelineCreateInfo){
        .vertex_shader = line_vertex_shader,
        .fragment_shader = line_fragment_shader,
        .color_targets_count = 1,
        .color_target_infos = &(R_GraphicsPipelineColorTargetInfo){
          .format = R_GetSwapchainTextureFormat(),
        },
    });
  }

  // Font Pipeline
  {
    R_Shader font_vertex_shader = R_CreateShader(
      _d_state.arena,
      &(R_ShaderCreateInfo){
        .file_name = Str8C("./data/shaders/font.vs.glsl"),
        .type = R_SHADER_TYPE_VERTEX,
        .global_uniforms_count = 1,
      }
    );

    R_Shader font_fragment_shader = R_CreateShader(
      _d_state.arena,
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
    _d_state.font_pipeline = R_CreateGraphicsPipeline(&font_pipeline_info);
  }

  // Square Pipeline
  {
    R_Shader square_vertex_shader = R_CreateShader(
      _d_state.arena,
      &(R_ShaderCreateInfo){
        .file_name = Str8C("./data/shaders/square.vs.glsl"),
        .type = R_SHADER_TYPE_VERTEX,
        .global_uniforms_count = 1,
        .instance_uniforms_count = 1,
      }
    );
    R_Shader square_fragment_shader = R_CreateShader(
      _d_state.arena,
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
    };

    _d_state.square_pipeline = R_CreateGraphicsPipeline(&square_pipeline_info);
  }
}

func void
D_DrawRect(R_CommandBuffer command_buffer, R_Buffer buffer, RectI32 viewport, RectF32 rect, Vec4 border_radius, Vec4 color, Vec4F32 border_color)
{
  R_BindGraphicsPipeline(command_buffer, _d_state.square_pipeline);
  struct
  {
    Mat4 projection;
  } square_global_vertex_data;
  square_global_vertex_data.projection = MakeOrthographicMat4(
    viewport.x, viewport.w,
    viewport.y, viewport.h,
    -1.0f, 1.0f
  );
  U64 square_global_vertex_data_offset = R_PushBuffer(buffer, (U8*)&square_global_vertex_data, sizeof(square_global_vertex_data));
  R_UniformBufferBindingInfo square_vertex_shader_global_uniform = {
    .buffer = buffer,
    .offset = square_global_vertex_data_offset,
    .size = sizeof(square_global_vertex_data),
  };
  R_BindGlobalVertexShaderData(command_buffer, 1, &square_vertex_shader_global_uniform, 0, 0);

  struct
  {
    Vec2 position;
    Vec2 size;
  } square_instance_vertex_data;
  square_instance_vertex_data.position = rect.position;
  square_instance_vertex_data.size = rect.size;
  U64 square_instance_vertex_data_offset = R_PushBuffer(buffer, (U8*)&square_instance_vertex_data, sizeof(square_instance_vertex_data));
  R_UniformBufferBindingInfo square_vertex_shader_instance_uniform = {
    .buffer = buffer,
    .offset = square_instance_vertex_data_offset,
    .size = sizeof(square_instance_vertex_data),
  };
  R_BindInstanceVertexShaderData(command_buffer, 1, &square_vertex_shader_instance_uniform, 0, 0);

  struct
  {
    Vec4F32 color;
    Vec4F32 border_color;
    Vec4F32 border_radius;
  } square_instance_fragment_data;
  square_instance_fragment_data.color = color;
  square_instance_fragment_data.border_color = border_color;
  square_instance_fragment_data.border_radius = border_radius;
  U64 square_instance_fragment_shader_data_offset = R_PushBuffer(buffer, (U8*)&square_instance_fragment_data, sizeof(square_instance_fragment_data));
  R_UniformBufferBindingInfo square_fragment_shader_instance_uniform = {
    .buffer = buffer,
    .offset = square_instance_fragment_shader_data_offset,
    .size = sizeof(square_instance_fragment_data),
  };
  R_BindInstanceFragmentShaderData(command_buffer, 1, &square_fragment_shader_instance_uniform, 0, 0);

  R_DrawPrimitives(command_buffer, 6, 1, 0, 0);

}

func void
D_DrawText(R_CommandBuffer command_buffer, R_Buffer buffer, R_TextureSampler sampler, RectI32 viewport, FontBitmap font, Str8 text, U32 font_size, Vec2F32 position, Vec4F32 color)
{
  TextVertex vertecies[1028] = {0};
  U32 vertecies_count = 0;
  U16 indecies[1028] = {0};
  U32 indecies_count = 0;

  I32 line_count = 0;
  I32 symbols_on_line = 0;

  for (I32 i = 0; i < text.length; i += 1)
  {
    if (text.data[i] == '\n')
    {
      line_count += 1;
      symbols_on_line = 0;
      continue;
    }

    I32 glyph_id = text.data[i] - '!' + 1;
    
    Vec2 glyph_position = AddVec2(position, MakeVec2(symbols_on_line*(F32)font_size*0.5f, line_count*font_size*0.7f));
    Vec2 glyph_grid_xy = MakeVec2(glyph_id%font.glyphs_per_row, glyph_id/font.glyphs_per_row);
    Vec2 glyph_uv_size = DivVec2(MakeVec2(font.glyph_size.x, font.glyph_size.y), MakeVec2(font.bitmap_size.x, font.bitmap_size.y));
    
    vertecies[vertecies_count].position = glyph_position;;
    vertecies[vertecies_count].uv = MulVec2(glyph_grid_xy, glyph_uv_size);
    vertecies_count += 1;
    vertecies[vertecies_count].position = AddVec2(glyph_position, MakeVec2(font_size, 0.0f));
    vertecies[vertecies_count].uv = AddVec2(MulVec2(glyph_grid_xy, glyph_uv_size), MakeVec2(glyph_uv_size.x, 0.0f));
    vertecies_count += 1;
    vertecies[vertecies_count].position = AddVec2(glyph_position, MakeVec2(font_size, font_size));
    vertecies[vertecies_count].uv = AddVec2(MulVec2(glyph_grid_xy, glyph_uv_size), glyph_uv_size);
    vertecies_count += 1;
    vertecies[vertecies_count].position = AddVec2(glyph_position, MakeVec2(0.0f, font_size));
    vertecies[vertecies_count].uv = AddVec2(MulVec2(glyph_grid_xy, glyph_uv_size), MakeVec2(0.0f, glyph_uv_size.y));
    vertecies_count += 1;

    U16 offset = i*4;
    indecies[indecies_count] = 0 + offset;
    indecies_count += 1;
    indecies[indecies_count] = 2 + offset;
    indecies_count += 1;
    indecies[indecies_count] = 1 + offset;
    indecies_count += 1;
    indecies[indecies_count] = 2 + offset;
    indecies_count += 1;
    indecies[indecies_count] = 0 + offset;
    indecies_count += 1;
    indecies[indecies_count] = 3 + offset;
    indecies_count += 1;

    symbols_on_line += 1;
  }
  U64 vertex_buffer_offset = R_PushBuffer(buffer, (U8*)vertecies, sizeof(vertecies[0])*vertecies_count);
  U64 index_buffer_offset = R_PushBuffer(buffer, (U8*)indecies, sizeof(indecies[0])*indecies_count);

  R_BindGraphicsPipeline(command_buffer, _d_state.font_pipeline);

  struct
  {
    Mat4 projection;
    Vec4 text_color;
  } font_vertex_shader_global_uniform_data;
  font_vertex_shader_global_uniform_data.projection = MakeOrthographicMat4(
    viewport.x, viewport.w,
    viewport.y, viewport.h,
    -1.0f, 1.0f
  );
  font_vertex_shader_global_uniform_data.text_color = color;
  U64 font_vertex_shader_global_uniform_data_offset = R_PushBuffer(buffer, (U8*)&font_vertex_shader_global_uniform_data, sizeof(font_vertex_shader_global_uniform_data));
  R_UniformBufferBindingInfo font_vertex_shader_global_uniform = 
  {
    .buffer = buffer,
    .offset = font_vertex_shader_global_uniform_data_offset,
    .size = sizeof(font_vertex_shader_global_uniform_data),
  };
  R_BindGlobalVertexShaderData(command_buffer, 1, &font_vertex_shader_global_uniform, 0, 0);

  R_SamplerBindingInfo font_sampler_binding = {
    .sampler = sampler,
    .texture = font.bitmap,
  };
  R_BindGlobalFragmentShaderData(command_buffer, 0, 0, 1, &font_sampler_binding);

  R_BindVertexBuffer(command_buffer, buffer, vertex_buffer_offset);
  R_BindIndexBuffer(command_buffer, buffer, index_buffer_offset, R_INDEX_SIZE_U16);
  // R_DrawPrimitives(command_buffer, 6, 1, 0, 0);
  R_DrawIndexedPrimitives(command_buffer, indecies_count, 1, 0, 0, 0);

}
