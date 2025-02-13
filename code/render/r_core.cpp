#include "r_core.h"

#include "vulkan/r_vk_main.h"
#include "vulkan/r_vk_main.cpp"

func void 
R_InitBackend(OS_Window* window)
{
  Renderer = {};
  Renderer.Init = R_VK_Init;
  Renderer.Destroy = R_VK_Destroy;
  Renderer.CreatePipeline = R_VK_CreatePipeline;
  // Renderer.BeginFrame = R_VK_BeginFrame;
  // Renderer.EndFrame = R_VK_EndFrame;
  // Renderer.BeginRenderPass = R_VK_BeginRenderPass;
  // Renderer.EndRenderPass = R_VK_EndRenderPass;
  // Renderer.Draw = R_VK_Draw;
  // Renderer.BindPipeline = R_VK_BindPipeline;
  // Renderer.CreateBuffer = _VK_CreateBuffer;

  Renderer.Init(window);
}
