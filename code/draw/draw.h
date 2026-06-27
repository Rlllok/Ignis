#pragma once

#include "rhi/rhi_include.h"

struct D_State {
  Arena* arena;

  RHI_ResourceTable rhi_resource_table;

  // -- 2D Pipelines
  RHI_GraphicsPipeline rectangle_pipeline;
  RHI_GraphicsPipeline text_pipeline;
  // -- 3D Pipelines
  RHI_GraphicsPipeline line_3d_pipeline;
} _d_state;

func void D_Init(U64 arena_size);
func void D_PreparePipelines();

func void D_DrawRectInternal(RHI_CommandBuffer command_buffer, RHI_Buffer buffer, RectF32 rect, Vec4F32 radius, Vec4F32 top_left_color, Vec4F32 top_right_color, Vec4F32 bottom_right_color, Vec4F32 bottom_left_color, F32 border_width, Vec4F32 border_color);
#define D_DrawRect(command_buffer, buffer, rect, radius, color) D_DrawRectInternal(command_buffer, buffer, rect, radius, color, color, color, color, 0.0f, color)
#define D_DrawRectWithBorder(command_buffer, buffer, rect, radius, color, border_width, border_color) D_DrawRectInternal(command_buffer, buffer, rect, radius, color, color, color, color, border_width, border_color)
#define D_DrawRectGradient(command_buffer, buffer, rect, radius, top_left_color, top_right_color, bottom_right_color, bottom_left_color) D_DrawRectInternal(command_buffer, buffer, rect, radius, top_left_color, top_right_color, bottom_right_color, bottom_left_color, 0.0f, (Vec4F32){0.0f, 0.0f, 0.0f, 0.0f})
