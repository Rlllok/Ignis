#pragma once

#include "render/r_include.h"

struct D_State
{
  R_Pipeline box_pipeline;
  R_Pipeline circle_pipeline;

  R_VertexBuffer quad_vertex_buffer = {};
  R_IndexBuffer  quad_index_buffer = {};
};
global D_State _d_state;

func void D_Init(Arena* arena);

func void D_DrawRectangle(RectI, Vec3f color);
func void D_DrawCircle(Vec2I position, I32 radius, Vec3f color);
