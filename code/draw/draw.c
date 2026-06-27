#include "draw.h"
#include "base/base_core.h"
#include "base/base_math.h"
#include "base/base_memory.h"

func void
D_Init(U64 arena_size) {
  // --AlNov 7 January 2026: @TODO Should use arena_size?
  _d_state.arena = AllocateArena(Gigabytes(4), Kilobytes(64));
  _d_state.rhi_resource_table = RHI_CreateResourceTable(_d_state.arena, 256, 1);
  D_PreparePipelines();
}

func void
D_PreparePipelines() {
  // 2D
  // Rectangle
  {
    RHI_ShaderArgumentKind vertex_shader_arguments[] = {
      RHI_ShaderArgumentKind_BufferAddress,
    };
    RHI_Shader vertex_shader = RHI_CreateShader(_d_state.arena, &(RHI_ShaderCreateInfo) {
      .file_name = Str8C("./data/shaders/draw/rectangle.vs"),
      .kind = RHI_ShaderKind_Vertex,
      .arguments = vertex_shader_arguments,
      .arguments_count = ArrayLength(vertex_shader_arguments),
    });

    RHI_ShaderArgumentKind fragment_shader_arguments[] = {
      RHI_ShaderArgumentKind_BufferAddress,
    };
    RHI_Shader fragment_shader = RHI_CreateShader(_d_state.arena, &(RHI_ShaderCreateInfo) {
      .file_name = Str8C("./data/shaders/draw/rectangle.fs"),
      .kind = RHI_ShaderKind_Fragment,
      .arguments = fragment_shader_arguments,
      .arguments_count = ArrayLength(fragment_shader_arguments),
    });

    _d_state.rectangle_pipeline = RHI_CreateGraphicsPipeline(&(RHI_GraphicsPipelineCreateInfo) {
      .vertex_shader = &vertex_shader,
      .fragment_shader = &fragment_shader,
      .color_targets_count = 1,
      .color_target_infos = &(RHI_GraphicsPipelineColorTargetInfo) {
        .format = RHI_GetSwapchainTextureFormat(),
        .blend_enable = 1,
      }
    });
  }
  // Font
  {
  }
}

func void
D_DrawRectInternal(RHI_CommandBuffer command_buffer, RHI_Buffer buffer, RectF32 rect, Vec4F32 radius, Vec4F32 top_left_color, Vec4F32 top_right_color, Vec4F32 bottom_right_color, Vec4F32 bottom_left_color, F32 border_width, Vec4F32 border_color) {
  RectI32 viewport = RHI_GetViewport(command_buffer);

  struct {
    Mat4F32 projection;
    Vec4F32 position_size;
    Vec4F32 radius;
    Vec4F32 top_left_color;
    Vec4F32 top_right_color;
    Vec4F32 bottom_right_color;
    Vec4F32 bottom_left_color;
    Vec4F32 border_color;
    F32     border_width;
  } data = {
    .projection = MakeOrthographicMat4F32(0.0f, viewport.w, viewport.h, 0.0f, -1.0f, 1.0f),
    .position_size = MakeVec4F32(rect.x, rect.y, rect.w, rect.h),
    .radius = radius,
    .top_left_color = top_left_color,
    .top_right_color = top_right_color,
    .bottom_right_color = bottom_right_color,
    .bottom_left_color = bottom_left_color,
    .border_color = border_color,
    .border_width = border_width,
  };
  U64 data_offset = RHI_PushBuffer(buffer, (U8*)&data, sizeof(data));

  RHI_ShaderArgument arguments[] = {
    {
      .kind = RHI_ShaderArgumentKind_BufferAddress,
      .address = RHI_BufferDeviceAddress(buffer) + data_offset,
    }
  };
  RHI_BindGraphicsPipeline(command_buffer, _d_state.rectangle_pipeline);
  // RHI_BindResourceTable(command_buffer, _d_state.resource_table);
  RHI_BindShaderArguments(command_buffer, RHI_ShaderKind_Vertex|RHI_ShaderKind_Fragment, arguments, ArrayLength(arguments));
  RHI_DrawPrimitives(command_buffer, 6, 1, 0, 0);
}
