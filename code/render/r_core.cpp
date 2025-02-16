#include "r_core.h"

// @TODO Should it be there
#include "vulkan/r_vk_core.h"
#include "vulkan/r_vk_core.cpp"
#include "vulkan/r_vk_buffer.cpp"
#include "vulkan/r_vk_render_pass.cpp"
#include "vulkan/r_vk_swapchain.cpp"
#include "vulkan/r_vk_command_buffer.cpp"
#include "vulkan/r_vk_pipeline.cpp"

func B32
R_InitRenderer()
{
  Renderer = {};
  Renderer.Init = R_VK_Init;
  Renderer.CreatePipeline = R_VK_CreatePipeline;
  Renderer.BeginFrame = R_VK_BeginFrame;
  Renderer.EndFrame = R_VK_EndFrame;
  Renderer.PresentFrame = R_VK_PresentFrame;
  Renderer.BeginRenderPass = R_VK_BeginRenderPass;
  Renderer.EndRenderPass = R_VK_EndRenderPass;
  Renderer.Draw = R_VK_Draw;
  Renderer.BindPipeline = R_VK_BindPipeline;
  Renderer.CreateBuffer = _VK_CreateBuffer;
  
  return true;
}

func B32 
R_Init(OS_Window* window)
{
  R_InitRenderer();

  Renderer.Init(window);

  return true;
}
