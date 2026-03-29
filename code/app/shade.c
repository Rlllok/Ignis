#include "base/base_include.h"
#include "os/os_include.h"
#include "rhi/rhi_include.h"
#include "ui/ui_include.h"

#include "base/base_include.c"
#include "os/os_include.c"
#include "rhi/rhi_include.c"
#include "ui/ui_include.c"

typedef struct Shade_State Shade_State;
struct Shade_Context {
  Arena*     arena;
  Arena*     frame_arena;
  OS_Window* window;

  B32 finished;

  RHI_CommandBuffer command_buffer;
  RHI_GraphicsPipeline main_pipeline;
} shade_context;

func void Shade_Init();
func void Shade_Shutdown();
func void Shade_Draw();

I32 main() {
  Shade_Init();

  OS_ShowWindow(shade_context.window);

  while (!shade_context.finished) {
    OS_EventList event_list = OS_GetEventList(shade_context.frame_arena, shade_context.window);
    
    Shade_Draw();

    if (OS_KeyPressed(OS_KEY_ESC)) {
      shade_context.finished = 1;
    }

    ResetArena(shade_context.frame_arena);
  }
  
  return 0;
}

func void
Shade_Init() {
  shade_context.arena = AllocateArena(Gigabytes(4), Kilobytes(64));
  shade_context.frame_arena = AllocateArena(Gigabytes(4), Kilobytes(64));

  OS_Init(Megabytes(32));
  shade_context.window = OS_CreateWindow(Str8C("Shade"), MakeVec2U32(1280, 720));

  RHI_Init(RHI_RendererKind_Vulkan, shade_context.window);
  shade_context.command_buffer = RHI_GetCommandBuffer();

  RHI_Shader vertex_shader = RHI_CreateShader(
    shade_context.arena,
    &(RHI_ShaderCreateInfo) {
      .file_name = Str8C("./data/shaders/shade.vs.glsl"),
      .kind = RHI_ShaderKind_Vertex
    }
  );

  RHI_Shader fragment_shader = RHI_CreateShader(
    shade_context.arena,
    &(RHI_ShaderCreateInfo) {
      .file_name = Str8C("./data/shaders/shade.fs.glsl"),
      .kind = RHI_ShaderKind_Fragment,
    }
  );

  shade_context.main_pipeline = RHI_CreateGraphicsPipeline(
    &(RHI_GraphicsPipelineCreateInfo) {
      .vertex_shader = vertex_shader,
      .fragment_shader = fragment_shader,
      .color_targets_count = 1,
      .color_target_infos = &(RHI_GraphicsPipelineColorTargetInfo) {
        .format = RHI_GetSwapchainTextureFormat(),
      },
    }
  );
}

func void
Shade_Shutdown() {
}

func void
Shade_Draw() {
  RHI_BeginCommandBuffer(shade_context.command_buffer); {
    RHI_Texture swapchain_texture = RHI_AcquireSwapchainTexture(shade_context.command_buffer);
    RHI_ColorTarget color_target = {
      .texture = swapchain_texture,
      .load_operation = RHI_AttachmentLoadOperation_Clear,
      .store_operation = RHI_AttachmentStoreOperation_Store,
      .clear_color = MakeVec4F32(0.08f, 0.09f, 0.18f, 1.0f),
    };

    RHI_RenderPass* render_pass = RHI_BeginRenderPass(shade_context.command_buffer, 1, &color_target, 0); {
      RectI32 rect = {
        .x = 0,
        .y = 0,
        .w = shade_context.window->size.x,
        .h = shade_context.window->size.y,
      };
      RHI_SetViewport(shade_context.command_buffer, rect);
      RHI_SetScissor(shade_context.command_buffer, rect);

      RHI_BindGraphicsPipeline(shade_context.command_buffer, shade_context.main_pipeline);
      RHI_DrawPrimitives(shade_context.command_buffer, 6, 1, 0, 0);
    }
    RHI_EndRenderPass(shade_context.command_buffer, render_pass);
  }
  RHI_SubmitCommandBuffer(shade_context.command_buffer);
}
