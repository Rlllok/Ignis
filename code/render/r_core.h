#pragma once

#include "../base/base_include.h"
#include "r_pipeline.h"

struct R_FrameInfo
{
  f32 delta_time;
};

struct R_Backend
{
  b8    (*Init)            (OS_Window* window);
  b8    (*DrawFrame)       ();
  b8    (*CreatePipeline)  (R_Pipeline* pipeline);
  void  (*BeginFrame)      ();
  void  (*EndFrame)        ();
  void  (*BeginRenderPass) ();
  void  (*EndRenderPass)   ();
  void  (*DrawMeshes)      ();
};

struct R_RendererState
{
  R_Backend backend;
};
global R_RendererState r_render_state;

func b8 R_Init(OS_Window* window);
func b8 R_CreateBackend();
func b8 R_DestroyBackend();

func b8 R_CreatePipeline(R_Pipeline* pipeline);

func void R_BeginFrame();
func void R_EndFrame();
func void R_BeginRenderPass();
func void R_EndRenderPass();
func void R_DrawMeshes();