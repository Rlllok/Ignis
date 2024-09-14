#include "r_core.h"

#include "vulkan/r_vk_core.h"
#include "vulkan/r_vk_core.cpp"

func B32 
R_Init(OS_Window* window)
{
  r_render_state = {};
  R_CreateBackend();

  r_render_state.backend.Init(window);

  return true;
}

func B32
R_CreateBackend()
{
  R_Backend backend = {};
  backend.Init            = R_VK_Init;
  backend.CreatePipeline  = R_VK_CreatePipeline;
  backend.BeginFrame      = R_VK_BeginFrame;
  backend.EndFrame        = R_VK_EndFrame;
  backend.BeginRenderPass = R_VK_BeginRenderPass;
  backend.EndRenderPass   = R_VK_EndRenderPass;
  backend.Draw            = R_VK_Draw;
  backend.BindPipeline    = R_VK_BindPipeline;

  r_render_state.backend = backend;
  
  return true;
}

func B32
R_DestroyBackend()
{
  return true;
}

// --AlNov: Bind backend functions -----------------------------------
func B32   R_CreatePipeline(R_Pipeline* pipeline)                                    { return r_render_state.backend.CreatePipeline(pipeline); }
func void R_BeginFrame()                                                            { r_render_state.backend.BeginFrame(); }
func void R_EndFrame()                                                              { r_render_state.backend.EndFrame(); }
func void R_BeginRenderPass(Vec4f clear_color, F32 clear_depth, F32 clear_stencil)  { r_render_state.backend.BeginRenderPass(clear_color, clear_depth, clear_stencil); }
func void R_EndRenderPass()                                                         { r_render_state.backend.EndRenderPass(); }
func void R_DrawSceneObject(R_DrawInfo* info)                                       { r_render_state.backend.Draw(info); }
func void R_BindPipeline(R_Pipeline* pipeline)                                      { r_render_state.backend.BindPipeline(pipeline); }
