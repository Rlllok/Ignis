#include "r_core.h"

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
  backend.Init      = R_VK_Init;
  backend.DrawFrame = R_VK_DrawFrame;
  backend.EndFrame  = R_VK_EndFrame;

  r_render_state.backend = backend;
  
  return true;
}

func b8 R_DestroyBackend()
{
  return true;
}

func b8 R_DrawFrame(R_FrameInfo* frame_info)
{
  // if (!r_render_state.backend) { return false; }

  r_render_state.backend.DrawFrame();
  r_render_state.backend.EndFrame();

  return true;
}
