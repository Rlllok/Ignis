#include "base/base_include.h"
#include "os/os_include.h"
#include "rhi/rhi_include.h"

#include "base/base_include.c"
#include "os/os_include.c"
#include "rhi/rhi_include.c"

func void Draw(RHI_CommandBuffer command_buffer, F32 dt);

I32 main() {
  LogInfo("Hello MacOS\n");

  Arena* arena = AllocateArena(Gigabytes(4), Kilobytes(16));
  Arena* frame_arena = AllocateArena(Gigabytes(4), Kilobytes(16));
  B32 finished = 0;

  OS_Init(Megabytes(16));

  Vec2U32 window_size = MakeVec2U32(1280, 720);
  OS_Window* window = OS_CreateWindow(Str8C("Simple Triangle Test (MacOS)"), window_size);

  RHI_Init(RHI_RendererKind_Metal, window);

  OS_ShowWindow(window);

  RHI_CommandBuffer command_buffer = RHI_GetCommandBuffer();

  while (!finished) {
    OS_EventList event_list = OS_GetEventList(frame_arena, window);

    if (OS_KeyPressed(OS_KEY_ESC)) {
      finished = 1;
    }

    if (OS_KeyPressed(OS_KEY_M)) {
      Vec2F32 p = OS_MousePosition(window);
      LogInfo("MousePosition: %fx %fy\n", p.x, p.y);
    }

    Draw(command_buffer, 0.0f);

    ResetArena(frame_arena);
  }

  return 0;
}

func void
Draw(RHI_CommandBuffer command_buffer, F32 dt) {
  RHI_BeginCommandBuffer(command_buffer);
    RHI_Texture swapchain_texture = RHI_AcquireSwapchainTexture(command_buffer);

    RHI_RenderPass* render_pass = RHI_BeginRenderPass(command_buffer, 0, 0, 0);
    RHI_EndRenderPass(command_buffer, render_pass);
  RHI_SubmitCommandBuffer(command_buffer);
}
