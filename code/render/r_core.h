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

struct R_Geometry
{
  U8* index_data;
  U64 index_size;
  U64 index_count;
  U64 index_backend_offset;
  
  U8* vertex_data;
  U64 vertex_size;
  U64 vertex_count;
  U64 vertex_backend_offset;
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

enum R_RendererType
{
  R_RENDERER_TYPE_NONE,

  R_RENDERER_TYPE_VULKAN,

  R_RENDERER_TYPE_COUNT
};

typedef B32  _RendererInit(OS_Window* window);
typedef B32  _RendererShutdown();

typedef B32  _RendererDrawFrame(R_Pipeline* pipeline);

typedef void _RendererPushGeometry(R_Geometry* geometry);

typedef B32  _RendererDrawGeometry(R_Geometry* geometry);

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
