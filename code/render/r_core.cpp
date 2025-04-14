#include "r_core.h"

// @TODO Should it be there
#include "base/base_logger.h"
#include "vulkan/r_vk.h"
#include "vulkan/r_vk.cpp"

func B32
R_InitRenderer(R_RendererType type)
{
  Renderer = {};

  if (type == R_RENDERER_TYPE_VULKAN)
  {
    Renderer.Init = R_VK_Init;
    Renderer.Shutdown = R_VK_Shutdown;
    // Renderer.CreatePipeline = R_VK_CreatePipeline;
    // Renderer.BeginFrame = R_VK_BeginFrame;
    // Renderer.EndFrame = R_VK_EndFrame;
    // Renderer.PresentFrame = R_VK_PresentFrame;
    // Renderer.BeginRenderPass = R_VK_BeginRenderPass;
    // Renderer.EndRenderPass = R_VK_EndRenderPass;
    // Renderer.Draw = R_VK_Draw;
    Renderer.DrawTriangle = R_VK_Draw;
    // Renderer.BindPipeline = R_VK_BindPipeline;
    // Renderer.CreateBuffer = _VK_CreateBuffer;
  }
  else
  {
    AssertMessage(0, "Wrong Render Type\n");
    return false;
  }
  
  return true;
}

func B32 
R_Init(R_RendererType type, OS_Window* window)
{
  R_InitRenderer(type);

  Renderer.Init(window);

  return true;
}

func B32
R_Shutdown()
{
  B32 result = Renderer.Shutdown();

  Renderer = {};

  return result;
}
