#pragma once

#include "../base/base_include.h"
#include "base/base_math.h"
#include "assets/mesh.h"

#include "r_texture.h"
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

struct R_DrawGeometryInfo
{
  R_Pipeline* pipeline;
  RectI viewport;
  RectI scissor;
  U8* uniform_data;
  U32 uniform_data_size;
  AST_Geometry* geometry;
};

enum R_RendererType
{
  R_RENDERER_TYPE_NONE,

  R_RENDERER_TYPE_VULKAN,

  R_RENDERER_TYPE_COUNT
};

enum R_AttachmentLoadOperation
{
  R_ATTACHMENT_LOAD_OPERATION_DONT_CARE,
  R_ATTACHMENT_LOAD_OPERATION_LOAD,
  R_ATTACHMENT_LOAD_OPERATION_CLEAR
};

typedef B32  _RendererInit(OS_Window* window);
typedef B32  _RendererShutdown();

typedef void _RendererHandleResize(OS_Window* window);

typedef void _RendererGraphicsShaderCreate(R_Pipeline* pipeline);
typedef void _RendererPipelineBind(R_Pipeline* pipeline);

typedef void _RendererFrameBegin();
typedef void _RendererFrameEnd();

typedef void _RendererRenderPassBegin(R_AttachmentLoadOperation load_operation, Vec4f clear_color);
typedef void _RendererRenderPassEnd();

typedef void _RendererGeometryPrepare(AST_Geometry* geometry);
typedef B32  _RendererGeometryDraw(R_DrawGeometryInfo* geometry);

struct R_Renderer
{
  _RendererInit*            Init;
  _RendererShutdown*        Shutdown;

  _RendererHandleResize*    HandleResize;

  _RendererGraphicsShaderCreate*  CreatePipeline;
  _RendererPipelineBind*    BindPipeline;
  
  _RendererFrameBegin*      BeginFrame;
  _RendererFrameEnd*        EndFrame;

  _RendererRenderPassBegin* BeginRenderPass;
  _RendererRenderPassEnd*   EndRenderPass;
  
  _RendererGeometryPrepare* PrepareGeometry;
  _RendererGeometryDraw*    DrawGeometry;
} Renderer;

func B32 R_InitRenderer();
func B32 R_Init(R_RendererType type, OS_Window* window);
func B32 R_Shutdown();
