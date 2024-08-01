#include "r_core.h"

#include "vulkan/r_vk_core.h"
#include "vulkan/r_vk_core.cpp"

func b8 R_Init(OS_Window* window)
{
  r_render_state = {};
  R_CreateBackend();

  r_render_state.backend.Init(window);

  return true;
}

func b8 R_CreateBackend()
{
  R_Backend backend = {};
  backend.Init            = R_VK_Init;
  backend.CreatePipeline  = R_VK_CreatePipeline;
  backend.BeginFrame      = TMP_BeginFrame;
  backend.EndFrame        = TMP_EndFrame;
  backend.BeginRenderPass = TMP_BeginRenderPass;
  backend.EndRenderPass   = TMP_EndRenderPass;
  backend.DrawMeshes      = TMP_DrawMeshes;

  r_render_state.backend = backend;
  
  return true;
}

func b8 R_DestroyBackend()
{
  return true;
}

// --AlNov: Bind backend functions -----------------------------------
func b8   R_CreatePipeline(R_Pipeline* pipeline)  { return r_render_state.backend.CreatePipeline(pipeline); }
func void R_BeginFrame()                          { r_render_state.backend.BeginFrame(); }
func void R_EndFrame()                            { r_render_state.backend.EndFrame(); }
func void R_BeginRenderPass()                     { r_render_state.backend.BeginRenderPass(); }
func void R_EndRenderPass()                       { r_render_state.backend.EndRenderPass(); }
func void R_DrawMeshes()                          { r_render_state.backend.DrawMeshes(); }