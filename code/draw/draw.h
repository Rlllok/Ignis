#pragma once

#include "assets/mesh.h"
#include "render/r_include.h"

struct D_State
{
  Arena* arena;

  // -- 2D
  R_GraphicsPipeline square_pipeline;
  R_GraphicsPipeline font_pipeline;

  // -- 3D
  R_GraphicsPipeline line_3d_pipeline;
} _d_state;

func void D_Init(U64 arena_size);
func void D_PreparePipelines();

func void D_DrawRect(R_CommandBuffer command_buffer, R_Buffer buffer, RectI32 viewport, RectF32 rect, Vec4 border_radius, Vec4 color, Vec4F32 border_color);
func void D_DrawText(R_CommandBuffer command_buffer, R_Buffer buffer, R_TextureSampler sampler, RectI32 viewport, FontBitmap font, Str8 text, U32 font_size, Vec2F32 position, Vec4F32 color);
