#include "base/base_include.h"
#include "os/os_include.h"
#include "rhi/rhi_include.h"
#include "draw/d_include.h"
#include "ui/ui_include.h"

#include "base/base_include.c"
#include "os/os_include.c"
#include "rhi/rhi_include.c"
#include "draw/d_include.c"
#include "ui/ui_include.c"

typedef struct Shade_Context Shade_Context;
struct Shade_Context {
  Arena*     arena;
  Arena*     frame_arena;
  OS_Window* window;

  B32   finished;
  UI_ID canvas_id;

  RHI_CommandBuffer    command_buffer;
  RHI_Buffer           uniform_buffer;
  RHI_GraphicsPipeline main_pipeline;
  RHI_GraphicsPipeline ui_rectangle_pipeline;
} shade_context;

func void Shade_Init();
func void Shade_Shutdown();
func void Shade_Draw();

I32 main() {
  Shade_Init();

  OS_ShowWindow(shade_context.window);

  while (!shade_context.finished) {
    OS_DispatchEvents(shade_context.frame_arena, shade_context.window);

    if (OS_KeyPressed(OS_KEY_ESC)) {
      shade_context.finished = 1;
    }

    // --AlNov: @TODO
    // There are bug or bad API design in the UI_Layer.
    // For some reson I have to add other "full screen" widget to be able to
    // draw background for Shade_UI_TopBar (there was no draw_command for this widget).
    // But, it seems, layout calculation is working fine.
    UI_BeginFrame(OS_MousePosition(shade_context.window), OS_MouseScroll());
    UI_WidgetBlock({
      .name = Str8C("UI_MainWidget"),
      .layout = {
        .width = UI_PixelSize(shade_context.window->size.w),
        .height = UI_PixelSize(shade_context.window->size.h),
        .direction = UI_LayoutDirection_TopToBottom,
      },
    }) {
     UI_WidgetBlock({
      .name = Str8C("Shade_UI_Context"),
      .layout = {
        .width = UI_PercentSize(1.0f),
        .height = UI_PercentSize(1.0f),
        .direction = UI_LayoutDirection_TopToBottom,
      },
     }) {
        UI_WidgetBlock({
          .name = Str8C("Shade_UI_TopBar"),
          .flags = UI_WidgetFlag_DrawBackground,
          .layout = {
            .width = UI_PercentSize(1.0f),
            .height = UI_PercentSize(0.05f),
          },
          .rectangle = {
            .color = RGBAFromHex(0x0000f1ff),
          },
        }) {
        }

        UI_WidgetBlock({
          .name = Str8C("Shade_UI_Canvas"),
          .layout = {
            .width = UI_PercentSize(1.0f),
            .height = UI_PercentSize(0.95f),
          },
        }) {
          shade_context.canvas_id = UI_GetID();
        }
      }
    }
    UI_EndFrame();
    
    Shade_Draw();

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
  shade_context.uniform_buffer = RHI_CreateBuffer(Megabytes(16), RHI_BufferUsageFlag_Uniform, RHI_BufferPropertyFlag_HostCoherent);

  {
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
        .global_uniforms_count = 1,
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

  UI_Init(shade_context.arena, 1024);
  D_Init(Kilobytes(64));
}

func void
Shade_Shutdown() {
}

func void
Shade_Draw() {
  RHI_ResetBuffer(shade_context.uniform_buffer);

  RHI_BeginCommandBuffer(shade_context.command_buffer); {
    RHI_Texture swapchain_texture = RHI_AcquireSwapchainTexture(shade_context.command_buffer);
    RHI_ColorTarget color_target = {
      .texture = swapchain_texture,
      .load_operation = RHI_AttachmentLoadOperation_Clear,
      .store_operation = RHI_AttachmentStoreOperation_Store,
      .clear_color = MakeVec4F32(0.08f, 0.09f, 0.18f, 1.0f),
    };

    RHI_RenderPass* render_pass = RHI_BeginRenderPass(shade_context.command_buffer, 1, &color_target, 0); {
      RectF32 canvas_rect = UI_GetRectangle(shade_context.canvas_id);
      RectI32 viewport = {
        .x = (I32)canvas_rect.x,
        .y = (I32)canvas_rect.y,
        .w = (I32)canvas_rect.w,
        .h = (I32)canvas_rect.h,
      };
      RHI_SetViewport(shade_context.command_buffer, viewport);
      RectI32 scissor = {
        .x = 0,
        .y = 0,
        .w = shade_context.window->size.w,
        .h = shade_context.window->size.h,
      };
      RHI_SetScissor(shade_context.command_buffer, scissor);

      RHI_BindGraphicsPipeline(shade_context.command_buffer, shade_context.main_pipeline);
      struct {
        Vec2F32 resolution;
      } global_fragment_data = {
        .resolution = canvas_rect.size,
      };

      RHI_UniformBufferBindingInfo global_fragment_data_uniform = {
        .buffer = shade_context.uniform_buffer,
        .offset = RHI_PushBuffer(shade_context.uniform_buffer, (U8*)&global_fragment_data, sizeof(global_fragment_data)),
        .size = sizeof(global_fragment_data),
      };
      RHI_BindGlobalFragmentShaderData(shade_context.command_buffer, 1, &global_fragment_data_uniform, 0, 0);

      RHI_DrawPrimitives(shade_context.command_buffer, 6, 1, 0, 0);

      viewport = (RectI32){
        .x = 0,
        .y = 0,
        .w = (I32)shade_context.window->size.w,
        .h = (I32)shade_context.window->size.h,
      };
      RHI_SetViewport(shade_context.command_buffer, viewport);
      for (I32 draw_command_index = 0; draw_command_index < ui_context.draw_commands.length; draw_command_index += 1) {
        UI_DrawCommand* command = UI_DrawCommandArrayGetPointer(&ui_context.draw_commands, draw_command_index);

        switch (command->kind) {
          default: {} break;

          case UI_DrawCommandKind_Rectangle: {
            D_DrawRect(shade_context.command_buffer, shade_context.uniform_buffer, viewport, command->rectangle.bound, command->rectangle.radius, command->rectangle.color, command->rectangle.border_color);
          } break;
        }
      }
    }
    RHI_EndRenderPass(shade_context.command_buffer, render_pass);
  }
  RHI_SubmitCommandBuffer(shade_context.command_buffer);
}
