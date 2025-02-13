#pragma once

#include "../base/base_include.h"
#include "base/base_math.h"
#include "r_pipeline.h"
#include "r_buffer.h"

struct R_SceneObject
{
  struct Vertex
  {
    Vec3f position;
    Vec3f normal;
    Vec2f uv;
  };

  R_SceneObject* next;
  R_SceneObject* previous;

  Vertex* vertecies;
  U32     vertex_count;
  U32*    indecies;
  U32     index_count;
};

struct R_FrameInfo
{
  F32 delta_time;
};

struct R_BindingGroup
{
  void* data;
  U64   data_size;
};

struct R_DrawInfo
{
  R_Pipeline* pipeline;
  R_VertexBuffer* vertex_buffer;
  R_IndexBuffer* index_buffer;

  R_BindingGroup scene_group;
  R_BindingGroup instance_group;

  RectI viewport;
  RectI scissor;
};

typedef B32  _RendererInit(OS_Window* window);
typedef B32  _RendererDrawFrame(R_Pipeline* pipeline);
typedef B32  _RendererCreatePipeline(R_Pipeline* pipeline);
typedef B32  _RendererBeginFrame();
typedef void _RendererEndFrame();
typedef void _RendererBeginRenderPass(Vec4f clear_color, F32 clear_depth, F32 clear_stencil);
typedef void _RendererEndRenderPass();
typedef void _RendererDraw(R_DrawInfo* draw_info);
typedef void _RendererBindPipeline(R_Pipeline* pipeline);

struct R_Renderer
{
  _RendererInit*            Init;
  _RendererDrawFrame*       DrawFrame;
  _RendererCreatePipeline*  CreatePipeline;
  _RendererBeginFrame*      BeginFrame;
  _RendererEndFrame*        EndFrame;
  _RendererBeginRenderPass* BeginRenderPass;
  _RendererEndRenderPass*   EndRenderPass;
  _RendererDraw*            Draw;
  _RendererBindPipeline*    BindPipeline;
  
  R_Buffer (*CreateBuffer) (
      U64 size,
      BufferUsageFlags usage_flags,
      BufferPropertyFlags flags);
} Renderer;

func B32 R_InitRenderer();
func B32 R_Init(OS_Window* window);
