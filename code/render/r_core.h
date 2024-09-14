#pragma once

#include "../base/base_include.h"
#include "r_pipeline.h"

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

  U32   vertex_count;
  U32   vertex_size;
  void* vertecies;

  U32   index_count;
  U32   index_size;
  void* indecies;

  void* uniform_data;
  U32   uniform_data_size;
};


struct R_Backend
{
  B32    (*Init)             (OS_Window* window);
  B32    (*DrawFrame)        ();
  B32    (*CreatePipeline)   (R_Pipeline* pipeline);
  void  (*BeginFrame)       ();
  void  (*EndFrame)         ();
  void  (*BeginRenderPass)  (Vec4f clear_color, F32 clear_depth, F32 clear_stencil);
  void  (*EndRenderPass)    ();
  void  (*Draw)             (R_DrawInfo* info);
  void  (*BindPipeline)     (R_Pipeline* pipeline);
};

struct R_RendererState
{
  R_Backend backend;
};
global R_RendererState r_render_state;

func B32 R_Init(OS_Window* window);
func B32 R_CreateBackend();
func B32 R_DestroyBackend();

// --AlNov: Binded backend functions -----------------------------------
func B32   R_CreatePipeline(R_Pipeline* pipeline);
func void R_BeginFrame();
func void R_EndFrame();
func void R_BeginRenderPass(Vec4f clear_color, F32 clear_depth, F32 clear_stencil);
func void R_EndRenderPass();
func void R_DrawSceneObject(R_DrawInfo* info);
func void R_BindPipeline(R_Pipeline* pipeline);
