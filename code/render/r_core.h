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
  u32     vertex_count;
  u32*    indecies;
  u32     index_count;
};

struct R_FrameInfo
{
  f32 delta_time;
};

struct R_DrawInfo
{
  R_Pipeline* pipeline;

  u32   vertex_count;
  u32   vertex_size;
  void* vertecies;

  u32   index_count;
  u32   index_size;
  void* indecies;

  void* uniform_data;
  u32   uniform_data_size;
};


struct R_Backend
{
  b8    (*Init)             (OS_Window* window);
  b8    (*DrawFrame)        ();
  b8    (*CreatePipeline)   (R_Pipeline* pipeline);
  void  (*BeginFrame)       ();
  void  (*EndFrame)         ();
  void  (*BeginRenderPass)  (Vec4f clear_color, f32 clear_depth, f32 clear_stencil);
  void  (*EndRenderPass)    ();
  void  (*Draw)             (R_DrawInfo* info);
  void  (*BindPipeline)     (R_Pipeline* pipeline);
};

struct R_RendererState
{
  R_Backend backend;
};
global R_RendererState r_render_state;

func b8 R_Init(OS_Window* window);
func b8 R_CreateBackend();
func b8 R_DestroyBackend();

// --AlNov: Binded backend functions -----------------------------------
func b8   R_CreatePipeline(R_Pipeline* pipeline);
func void R_BeginFrame();
func void R_EndFrame();
func void R_BeginRenderPass(Vec4f clear_color, f32 clear_depth, f32 clear_stencil);
func void R_EndRenderPass();
func void R_DrawSceneObject(R_DrawInfo* info);
func void R_BindPipeline(R_Pipeline* pipeline);