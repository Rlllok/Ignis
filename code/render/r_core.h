#pragma once

#include "../base/base_include.h"
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

struct R_DrawInfo
{
  R_Pipeline* pipeline;
  R_VertexBuffer* vertex_buffer;
  R_IndexBuffer* index_buffer;

  void* uniform_data;
  U32   uniform_data_size;

  void* scene_data;
  U32   scene_data_size;

  void* draw_vs_data;
  U32   draw_vs_data_size;
  
  void* draw_fs_data;
  U32   draw_fs_data_size;
};

typedef B32  _RendererInit(OS_Window* window);
typedef B32  _RendererDrawFrame(R_Pipeline* pipeline);
typedef B32  _RendererCreatePipeline(R_Pipeline* pipeline);
typedef void _RendererBeginFrame();
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
