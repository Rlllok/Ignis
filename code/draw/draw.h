#pragma once

#include "assets/mesh.h"
#include "render/r_include.h"

struct D_State
{
  Arena* arena;

  R_Pipeline box_pipeline;
  R_Pipeline circle_pipeline;
  R_Pipeline bezier_pipeline;

  R_VertexBuffer quad_vertex_buffer = {};
  R_IndexBuffer  quad_index_buffer = {};

  AST_Geometry geometry;
} _d_state;

func void D_Init(Arena* arena);

func void D_DrawRectangle(OS_Window* window, RectI rectangle, Vec3f color, F32 rotation);
func void D_DrawCircle(Vec2I position, I32 radius, Vec3f color);
func void D_DrawBezier(Vec2I p0, Vec2I p1, Vec2I c0, Vec3f color);
