#pragma once

#include "../base/base_include.h"
#include "r_pipeline.h"

struct R_SceneObject
{
  struct R_SceneObjectVertex
  {
    Vec3f position;
    Vec3f normal;
    Vec2f uv;
  };

  R_SceneObject* next;
  R_SceneObject* previous;

  R_SceneObjectVertex*  vertecies;
  u32                   vertex_count;
  u32*                  indecies;
  u32                   index_count;
};

struct R_FrameInfo
{
  f32 delta_time;
};

struct R_Backend
{
  b8    (*Init)             (OS_Window* window);
  b8    (*DrawFrame)        ();
  b8    (*CreatePipeline)   (R_Pipeline* pipeline);
  void  (*BeginFrame)       ();
  void  (*EndFrame)         ();
  void  (*BeginRenderPass)  ();
  void  (*EndRenderPass)    ();
  void  (*DrawSceneObject)  (R_SceneObject* object, void* uniform_data, u32 uniform_size);
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
func void R_BeginRenderPass();
func void R_EndRenderPass();
func void R_DrawSceneObject(R_SceneObject* object, void* uniform_data, u32 uniform_size);
func void R_BindPipeline(R_Pipeline* pipeline);