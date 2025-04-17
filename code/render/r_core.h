#pragma once

#include "../base/base_include.h"
#include "base/base_math.h"
#include "assets/mesh.h"

#include "r_pipeline.h"
#include "r_buffer.h"

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

enum R_RendererType
{
  R_RENDERER_TYPE_NONE,

  R_RENDERER_TYPE_VULKAN,

  R_RENDERER_TYPE_COUNT
};

typedef B32  _RendererInit(OS_Window* window);
typedef B32  _RendererShutdown();

typedef B32  _RendererDrawFrame(R_Pipeline* pipeline);

typedef void _RendererPushGeometry(AST_Geometry* geometry);

typedef B32  _RendererDrawGeometry(AST_Geometry* geometry);

struct R_Renderer
{
  _RendererInit*            Init;
  _RendererShutdown*        Shutdown;
  _RendererPushGeometry*    PushGeometry;
  _RendererDrawGeometry*    DrawGeometry;
} Renderer;

func B32 R_InitRenderer();
func B32 R_Init(R_RendererType type, OS_Window* window);
func B32 R_Shutdown();
