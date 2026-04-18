#include "base/base_include.h"
#include "os/os_include.h"
#include "rhi/rhi_include.h"

#include "base/base_include.c"
#include "os/os_include.c"
#include "rhi/rhi_include.c"

typedef struct Vertex Vertex;
struct Vertex {
  Vec3F32 position;
};

typedef struct AppContext AppContext;
struct AppContext {
  Arena* arena;
  Arena* frame_arena;
  B32    finished;
  F32    dt;

  OS_Window* window;

  RHI_GraphicsPipeline pipeline;
  RHI_Buffer           storage_buffer;
  U64                  vertecies_offset;
} app_context;

func void Draw(RHI_CommandBuffer command_buffer, F32 dt);

I32 main() {
  LogInfo("Hello MacOS\n");

  app_context.arena = AllocateArena(Gigabytes(4), Kilobytes(16));
  app_context.frame_arena = AllocateArena(Gigabytes(4), Kilobytes(16));
  app_context.finished = 0;
  app_context.dt = 0.0f;

  OS_Init(Megabytes(16));

  Vec2U32 window_size = MakeVec2U32(1280, 720);
  app_context.window = OS_CreateWindow(Str8C("Simple Triangle Test (MacOS)"), window_size);

  RHI_Init(RHI_RendererKind_Metal, app_context.window);

  OS_ShowWindow(app_context.window);

  app_context.storage_buffer = RHI_CreateBuffer(Megabytes(16), RHI_BufferUsageFlag_Vertex, RHI_BufferPropertyFlag_HostCoherent|RHI_BufferPropertyFlag_HostVisible);
  Vertex vertecies[] = {
    MakeVec3F32(0.0f, 0.5f, 0.0f),
    MakeVec3F32(0.5f, -0.5f, 0.0f),
    MakeVec3F32(-0.5f, -0.5f, 0.0f),
  };
  app_context.vertecies_offset = RHI_PushBuffer(app_context.storage_buffer, (U8*)(vertecies), sizeof(Vertex)*ArrayLength(vertecies));

  RHI_CommandBuffer command_buffer = RHI_GetCommandBuffer();
  RHI_GraphicsPipelineCreateInfo pipeline_info = {
    .vertex_attributes_count = 1,
    .vertex_attributes = &(RHI_VertexAttribute) {
      .location = 0,
      .format = RHI_VertexAttributeFormat_Vec3F32,
      .offset = offsetof(Vertex, position),
    },
    .color_targets_count = 1,
    .color_target_infos = &(RHI_GraphicsPipelineColorTargetInfo) {
      .format = RHI_GetSwapchainTextureFormat(),
    },
  };
  app_context.pipeline = RHI_CreateGraphicsPipeline(&pipeline_info);

  U64 start_ts = OS_GetTimeTicks();
  while (!app_context.finished) {
    OS_EventList event_list = OS_GetEventList(app_context.frame_arena, app_context.window);

    if (OS_KeyPressed(OS_KEY_ESC)) {
      app_context.finished = 1;
    }

    Draw(command_buffer, app_context.dt);

    ResetArena(app_context.frame_arena);

    U64 end_ts = OS_GetTimeTicks();
    app_context.dt = ((F32)(end_ts - start_ts))*0.001f;
  }

  return 0;
}

func void
Draw(RHI_CommandBuffer command_buffer, F32 dt) {
  RHI_BeginCommandBuffer(command_buffer);
    RHI_Texture swapchain_texture = RHI_AcquireSwapchainTexture(command_buffer);

    RHI_ColorTarget color_target = {
      .texture = swapchain_texture,
      .load_operation = RHI_AttachmentLoadOperation_Clear,
      .store_operation = RHI_AttachmentStoreOperation_Store,
      .clear_color = MakeVec4F32(sinf(dt*0.003f), 0.09f, 0.18f, 1.0f),
    };

    RHI_RenderPass* render_pass = RHI_BeginRenderPass(command_buffer, 1, &color_target, 0);
      RectI32 viewport = {
        .x = 0,
        .y = 0,
        .w = app_context.window->size.w,
        .h = app_context.window->size.h,
      };
      RHI_SetViewport(command_buffer, viewport);
      RHI_BindGraphicsPipeline(command_buffer, app_context.pipeline);
      RHI_BindVertexBuffer(command_buffer, app_context.storage_buffer, app_context.vertecies_offset);
      RHI_DrawPrimitives(command_buffer, 3, 1, 0, 0);
    RHI_EndRenderPass(command_buffer, render_pass);
  RHI_SubmitCommandBuffer(command_buffer);
}
