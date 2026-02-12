#include "draw.h"
#include "base/base_core.h"
#include "base/base_math.h"
#include "base/base_memory.h"

func void
D_Init(U64 arena_size)
{
  // --AlNov 7 January 2026: @TODO Should use arena_size?
  _d_state.arena = AllocateArena(Gigabytes(4), Kilobytes(64));
  D_PreparePipelines();
}

func void
D_PreparePipelines()
{
  // 3D Line Pipeline
  {
    RHI_Shader line_vertex_shader = RHI_CreateShader(
      _d_state.arena,
      &(RHI_ShaderCreateInfo){
        .file_name = Str8C("./data/shaders/line3d.vs.glsl"),
        .kind = RHI_ShaderKind_Vertex,
        .global_uniforms_count = 1,
        .instance_uniforms_count = 1,
      }
    );

    RHI_Shader line_fragment_shader = RHI_CreateShader(
      _d_state.arena,
      &(RHI_ShaderCreateInfo){
        .file_name = Str8C("./data/shaders/line3d.fs.glsl"),
        .kind = RHI_ShaderKind_Fragment,
      }
    );

    _d_state.line_3d_pipeline = RHI_CreateGraphicsPipeline(
      &(RHI_GraphicsPipelineCreateInfo){
        .vertex_shader = line_vertex_shader,
        .fragment_shader = line_fragment_shader,
        .color_targets_count = 1,
        .color_target_infos = &(RHI_GraphicsPipelineColorTargetInfo){
          .format = RHI_GetSwapchainTextureFormat(),
        },
    });
  }

  // Font Pipeline
  {
    RHI_Shader font_vertex_shader = RHI_CreateShader(
      _d_state.arena,
      &(RHI_ShaderCreateInfo){
        .file_name = Str8C("./data/shaders/font.vs.glsl"),
        .kind = RHI_ShaderKind_Vertex,
        .global_uniforms_count = 1,
      }
    );

    RHI_Shader font_fragment_shader = RHI_CreateShader(
      _d_state.arena,
      &(RHI_ShaderCreateInfo){
        .file_name = Str8C("./data/shaders/font.fs.glsl"),
        .kind = RHI_ShaderKind_Fragment,
        .global_samplers_count = 1,
      }
    );

    RHI_VertexAttribute font_vertex_attributes[] = {
      {
        .location = 0,
        .format = RHI_VertexAttributeFormat_Vec2F32,
        .offset = offsetof(TextVertex, position),
      },
      {
        .location = 1,
        .format = RHI_VertexAttributeFormat_Vec2F32,
        .offset = offsetof(TextVertex, uv),
      },
    };
    RHI_GraphicsPipelineColorTargetInfo font_pipeline_color_target = {
      .format = RHI_GetSwapchainTextureFormat(),
      .blend_enable = 1,
    };
    RHI_GraphicsPipelineCreateInfo font_pipeline_info = {
      .vertex_shader = font_vertex_shader,
      .fragment_shader = font_fragment_shader,
      .vertex_attributes_count = ArrayLength(font_vertex_attributes),
      .vertex_attributes = font_vertex_attributes,
      .color_targets_count = 1,
      .color_target_infos = &font_pipeline_color_target,
      .depth_stencil_state = {
        .depth_test_enable = 0,
      },
    };
    _d_state.font_pipeline = RHI_CreateGraphicsPipeline(&font_pipeline_info);
  }

  // Square Pipeline
  {
    RHI_Shader square_vertex_shader = RHI_CreateShader(
      _d_state.arena,
      &(RHI_ShaderCreateInfo){
        .file_name = Str8C("./data/shaders/square.vs.glsl"),
        .kind = RHI_ShaderKind_Vertex,
        .global_uniforms_count = 1,
        .instance_uniforms_count = 1,
      }
    );
    RHI_Shader square_fragment_shader = RHI_CreateShader(
      _d_state.arena,
      &(RHI_ShaderCreateInfo){
        .file_name = Str8C("./data/shaders/square.fs.glsl"),
        .kind = RHI_ShaderKind_Fragment,
        .global_uniforms_count = 0,
        .instance_uniforms_count = 1,
      }
    );

    RHI_GraphicsPipelineColorTargetInfo square_pipeline_color_target_infos[] = {
      {
        .format = RHI_GetSwapchainTextureFormat(),
        .blend_enable = 1,
      },
    };

    RHI_GraphicsPipelineCreateInfo square_pipeline_info = {
      .vertex_shader = square_vertex_shader,
      .fragment_shader = square_fragment_shader,
      .color_targets_count = ArrayLength(square_pipeline_color_target_infos),
      .color_target_infos = square_pipeline_color_target_infos,
    };

    _d_state.square_pipeline = RHI_CreateGraphicsPipeline(&square_pipeline_info);
  }
}

func void
D_DrawRect(RHI_CommandBuffer command_buffer, RHI_Buffer buffer, RectI32 viewport, RectF32 rect, Vec4F32 border_radius, Vec4F32 color, Vec4F32 border_color)
{
  RHI_BindGraphicsPipeline(command_buffer, _d_state.square_pipeline);
  struct
  {
    Mat4F32 projection;
  } square_global_vertex_data;
  square_global_vertex_data.projection = MakeOrthographicMat4F32(
    viewport.x, viewport.w,
    viewport.y, viewport.h,
    -1.0f, 1.0f
  );
  U64 square_global_vertex_data_offset = RHI_PushBuffer(buffer, (U8*)&square_global_vertex_data, sizeof(square_global_vertex_data));
  RHI_UniformBufferBindingInfo square_vertex_shader_global_uniform = {
    .buffer = buffer,
    .offset = square_global_vertex_data_offset,
    .size = sizeof(square_global_vertex_data),
  };
  RHI_BindGlobalVertexShaderData(command_buffer, 1, &square_vertex_shader_global_uniform, 0, 0);

  struct
  {
    Vec2F32 position;
    Vec2F32 size;
  } square_instance_vertex_data;
  square_instance_vertex_data.position = rect.position;
  square_instance_vertex_data.size = rect.size;
  U64 square_instance_vertex_data_offset = RHI_PushBuffer(buffer, (U8*)&square_instance_vertex_data, sizeof(square_instance_vertex_data));
  RHI_UniformBufferBindingInfo square_vertex_shader_instance_uniform = {
    .buffer = buffer,
    .offset = square_instance_vertex_data_offset,
    .size = sizeof(square_instance_vertex_data),
  };
  RHI_BindInstanceVertexShaderData(command_buffer, 1, &square_vertex_shader_instance_uniform, 0, 0);

  struct
  {
    Vec4F32 color;
    Vec4F32 border_color;
    Vec4F32 border_radius;
  } square_instance_fragment_data;
  square_instance_fragment_data.color = color;
  square_instance_fragment_data.border_color = border_color;
  square_instance_fragment_data.border_radius = border_radius;
  U64 square_instance_fragment_shader_data_offset = RHI_PushBuffer(buffer, (U8*)&square_instance_fragment_data, sizeof(square_instance_fragment_data));
  RHI_UniformBufferBindingInfo square_fragment_shader_instance_uniform = {
    .buffer = buffer,
    .offset = square_instance_fragment_shader_data_offset,
    .size = sizeof(square_instance_fragment_data),
  };
  RHI_BindInstanceFragmentShaderData(command_buffer, 1, &square_fragment_shader_instance_uniform, 0, 0);

  RHI_DrawPrimitives(command_buffer, 6, 1, 0, 0);
}

func void
D_DrawText(RHI_CommandBuffer command_buffer, RHI_Buffer buffer, RHI_TextureSampler sampler, RectI32 viewport, FontBitmap font, Str8 text, U32 font_size, Vec2F32 position, Vec4F32 color)
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
    
    Vec2F32 glyph_position = AddVec2F32(position, MakeVec2F32(symbols_on_line*(F32)font_size*0.5f, line_count*font_size*0.7f));
    Vec2F32 glyph_grid_xy = MakeVec2F32(glyph_id%font.glyphs_per_row, glyph_id/font.glyphs_per_row);
    Vec2F32 glyph_uv_size = DivVec2F32(MakeVec2F32(font.glyph_size.x, font.glyph_size.y), MakeVec2F32(font.bitmap_size.x, font.bitmap_size.y));
    
    vertecies[vertecies_count].position = glyph_position;;
    vertecies[vertecies_count].uv = MulVec2F32(glyph_grid_xy, glyph_uv_size);
    vertecies_count += 1;
    vertecies[vertecies_count].position = AddVec2F32(glyph_position, MakeVec2F32(font_size, 0.0f));
    vertecies[vertecies_count].uv = AddVec2F32(MulVec2F32(glyph_grid_xy, glyph_uv_size), MakeVec2F32(glyph_uv_size.x, 0.0f));
    vertecies_count += 1;
    vertecies[vertecies_count].position = AddVec2F32(glyph_position, MakeVec2F32(font_size, font_size));
    vertecies[vertecies_count].uv = AddVec2F32(MulVec2F32(glyph_grid_xy, glyph_uv_size), glyph_uv_size);
    vertecies_count += 1;
    vertecies[vertecies_count].position = AddVec2F32(glyph_position, MakeVec2F32(0.0f, font_size));
    vertecies[vertecies_count].uv = AddVec2F32(MulVec2F32(glyph_grid_xy, glyph_uv_size), MakeVec2F32(0.0f, glyph_uv_size.y));
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
  U64 vertex_buffer_offset = RHI_PushBuffer(buffer, (U8*)vertecies, sizeof(vertecies[0])*vertecies_count);
  U64 index_buffer_offset = RHI_PushBuffer(buffer, (U8*)indecies, sizeof(indecies[0])*indecies_count);

  RHI_BindGraphicsPipeline(command_buffer, _d_state.font_pipeline);

  struct
  {
    Mat4F32 projection;
    Vec4F32 text_color;
  } font_vertex_shader_global_uniform_data;
  font_vertex_shader_global_uniform_data.projection = MakeOrthographicMat4F32(
    viewport.x, viewport.w,
    viewport.y, viewport.h,
    -1.0f, 1.0f
  );
  font_vertex_shader_global_uniform_data.text_color = color;
  U64 font_vertex_shader_global_uniform_data_offset = RHI_PushBuffer(buffer, (U8*)&font_vertex_shader_global_uniform_data, sizeof(font_vertex_shader_global_uniform_data));
  RHI_UniformBufferBindingInfo font_vertex_shader_global_uniform = 
  {
    .buffer = buffer,
    .offset = font_vertex_shader_global_uniform_data_offset,
    .size = sizeof(font_vertex_shader_global_uniform_data),
  };
  RHI_BindGlobalVertexShaderData(command_buffer, 1, &font_vertex_shader_global_uniform, 0, 0);

  RHI_SamplerBindingInfo font_sampler_binding = {
    .sampler = sampler,
    .texture = font.bitmap,
  };
  RHI_BindGlobalFragmentShaderData(command_buffer, 0, 0, 1, &font_sampler_binding);

  RHI_BindVertexBuffer(command_buffer, buffer, vertex_buffer_offset);
  RHI_BindIndexBuffer(command_buffer, buffer, index_buffer_offset, RHI_IndexSize_U16);
  // RHI_DrawPrimitives(command_buffer, 6, 1, 0, 0);
  RHI_DrawIndexedPrimitives(command_buffer, indecies_count, 1, 0, 0, 0);

}
