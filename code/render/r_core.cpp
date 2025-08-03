#include "r_core.h"

#include "base/base_logger.h"

func B32
R_InitRenderer(R_RendererType type)
{
  Renderer = {};

  if (type == R_RENDERER_TYPE_VULKAN)
  {
    Renderer.Init = R_VK_Init;
    Renderer.Shutdown = R_VK_Shutdown;
    Renderer.HandleResize = R_VK_HandleResize;
    Renderer.GraphicsShaderCreate = R_VK_GraphicsShaderCreate;
    Renderer.PipelineBind = R_VK_PipelineBind;
    Renderer.FrameBegin = R_VK_FrameBegin;
    Renderer.FrameEnd = R_VK_FrameEnd;
    Renderer.RenderPassBegin = R_VK_RenderPassBegin;
    Renderer.RenderPassEnd = R_VK_RenderPassEnd;
    Renderer.GeometryPrepare = R_VK_GeometryPrepare;
    Renderer.GeometryDraw = R_VK_GeometryDraw;
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
