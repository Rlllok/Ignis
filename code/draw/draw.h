#pragma once

#include "assets/mesh.h"
#include "rhi/rhi_include.h"

struct D_State
{
  Arena* arena;

  // -- 2D
  RHI_GraphicsPipeline square_pipeline;
  RHI_GraphicsPipeline font_pipeline;

  // -- 3D
  RHI_GraphicsPipeline line_3d_pipeline;
} _d_state;

func void D_Init(U64 arena_size);
func void D_PreparePipelines();

func void D_DrawRect(RHI_CommandBuffer command_buffer, RHI_Buffer buffer, RectI32 viewport, RectF32 rect, Vec4F32 border_radius, Vec4F32 color, Vec4F32 border_color);
func void D_DrawText(RHI_CommandBuffer command_buffer, RHI_Buffer buffer, RHI_TextureSampler sampler, RectI32 viewport, FontBitmap font, Str8 text, U32 font_size, Vec2F32 position, Vec4F32 color);
