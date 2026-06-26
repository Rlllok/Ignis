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

func void D_DrawRectWithBorder(RHI_CommandBuffer command_buffer, RHI_Buffer buffer, RectF32 rect, Vec4F32 radius, Vec4F32 color, F32 border_width, Vec4F32 border_color);
#define D_DrawRect(command_buffer, buffer, rect, radius) D_DrawRectWithBorder(command_buffer, buffer, rect, radius, color, 0.0f, color)
