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
    Renderer.CreatePipeline = R_VK_CreateGraphicsPipeline;
    Renderer.BindPipeline = R_VK_BindPipeline;
    Renderer.BeginFrame = R_VK_BeginFrame;
    Renderer.EndFrame = R_VK_EndFrame;
    Renderer.BeginRenderPass = R_VK_BeginRenderPass;
    Renderer.EndRenderPass = R_VK_EndRenderPass;
    Renderer.PushGeometry = R_VK_PushGeometry;
    Renderer.DrawGeometry = R_VK_DrawGeometry;
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
