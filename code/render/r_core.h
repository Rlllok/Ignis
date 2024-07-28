#pragma once

#include "../base/base_include.h"
#include "r_pipeline.h"

struct R_FrameInfo
{
  f32 delta_time;
};

struct R_Backend
{
  b8 (*Init)            (OS_Window* window);
  b8 (*DrawFrame)       ();
  b8 (*EndFrame)        ();
  b8 (*CreatePipeline)  (R_Shader* vertex_shader, R_Shader* fragment_shader);
};

struct R_RendererState
{
  R_Backend backend;
};
global R_RendererState r_render_state;

func b8 R_Init(OS_Window* window);
func b8 R_CreateBackend();
func b8 R_DestroyBackend();

func b8 R_DrawFrame(R_FrameInfo* frame_info);

func b8 R_CreatePipeline(R_Shader* vertex_shader, R_Shader* fragment_shader);