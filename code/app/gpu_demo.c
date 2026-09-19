#include "base/base_include.h"
#include "os/os_include.h"
#include "rhi/rhi_include.h"
#include "draw/draw.h"

#include "base/base_include.c"
#include "os/os_include.c"
#include "rhi/rhi_include.c"
#include "draw/draw.c"

struct {
  Arena* global_arena;
  Arena* frame_arena;

  OS_Window* window;

  RHI_CommandBuffer rhi_command_buffer;

  B32 finished;
} gpu_demo_context;

I32 main() {
  gpu_demo_context.global_arena = AllocateArena(Gigabytes(1), Kilobytes(4));
  gpu_demo_context.frame_arena = AllocateArena(Gigabytes(1), Kilobytes(4));

  OS_Init(Megabytes(4));
  gpu_demo_context.window = OS_CreateWindow(Str8C("GPU Demo"), MakeVec2U32(1280, 720));
  OS_ShowWindow(gpu_demo_context.window);

  RHI_Init(gpu_demo_context.window);
  gpu_demo_context.rhi_command_buffer = RHI_GetCommandBuffer();

  while (!gpu_demo_context.finished) {
    OS_DispatchEvents(gpu_demo_context.frame_arena, gpu_demo_context.window);

    if (OS_KeyPressed(OS_KEY_ESC)) {
      gpu_demo_context.finished = 1;
    }

    RHI_BeginCommandBuffer(gpu_demo_context.rhi_command_buffer); {
      RHI_Texture swapchain_texture = RHI_AcquireSwapchainTexture(gpu_demo_context.rhi_command_buffer);
      RHI_ColorTarget color_targets[] = {
        {
          .texture = swapchain_texture,
          .load_operation = RHI_AttachmentLoadOperation_Clear,
          .store_operation = RHI_AttachmentStoreOperation_Store,
          .clear_color = RGBAFromHex(0xfbf0d9ff),
        }
      };
      RHI_RenderPassInfo render_pass_info = {
        .render_area = MakeRectI32(0, 0, swapchain_texture.size.x, swapchain_texture.size.y),
        .color_targets = color_targets,
        .color_targets_count = ArrayLength(color_targets),
        .depth_target = 0,
      };
      RHI_BeginRenderPass(gpu_demo_context.rhi_command_buffer, render_pass_info); {
        RectI32 viewport = { .x = 0, .y = 0, .w = gpu_demo_context.window->size.x, .h = gpu_demo_context.window->size.y };
        RHI_SetViewport(gpu_demo_context.rhi_command_buffer, viewport);
        RHI_SetScissor(gpu_demo_context.rhi_command_buffer, viewport);
      }
      RHI_EndRenderPass(gpu_demo_context.rhi_command_buffer);

      RHI_Present(gpu_demo_context.rhi_command_buffer);
    }
    RHI_EndCommandBuffer(gpu_demo_context.rhi_command_buffer);
    RHI_SubmitCommandBuffer(gpu_demo_context.rhi_command_buffer, 0, 0, 0, 0);

    ResetArena(gpu_demo_context.frame_arena);
  }

  return 0;
}
