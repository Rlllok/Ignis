#include "base/base_include.h"
#include "os/os_include.h"
#include "rhi/rhi_include.h"
#include "ui_new/ui_include.h"

#include "base/base_include.c"
#include "os/os_include.c"
#include "rhi/rhi_include.c"
#include "ui_new/ui_include.c"

static struct {
  OS_Window*           window;
  RHI_Buffer           gpu_buffer;
  RHI_GraphicsPipeline rectangle_pipeline;

  UI_Context*     ui_context;
  UI_DrawCommand* ui_draw_commands;

  F32 dt;

  Vec4F32 colors[4];
  I32     current_color_index;
  B32     color_animation;
  F32     animation_duration;
  F32     animation_time;

  F32 slider_value;
} ui_demo;

func void
UI_DemoTopBarItem() {
  UI_WidgetBlock({
    .label = Str8C("TopBarItem"),
    .flags = UI_WidgetFlag_DrawBackground,
    .layout = {
      .width = UI_PixelSize(50.0f),
      .height = UI_PercentSize(1.0f),
    },
    .style = {
      .background_color = MakeVec4F32(0.3f, 0.3f, 0.3f, 1.0f),
    }
  }) {
  }
}

func void
UI_DemoTopBar() {
  UI_WidgetBlock({
    .label = Str8C("TopBar"),
    .flags = UI_WidgetFlag_DrawBackground,
    .layout = {
      .width = UI_PercentSize(1.0f),
      .height = UI_PixelSize(25.0f),
      .direction = UI_Axis_X,
    },
    .style = {
      .background_color = MakeVec4F32(0.1f, 0.12f, 0.16f, 1.0f),
    }
  }) {
    for (I32 topbar_item_index = 0; topbar_item_index < 3; topbar_item_index += 1) {
      UI_DemoTopBarItem();
    }
  }
}

func void
UI_DemoFileButton() {
  UI_WidgetBlock({
    .label = Str8C("File"),
    .flags = UI_WidgetFlag_DrawBackground,
    .layout = {
      .width = UI_PercentSize(1.0f),
      .height = UI_PixelSize(30.0f),
      .direction = UI_Axis_Y,
    },
    .style = {
      .background_color = MakeVec4F32(0.2f, 0.22f, 0.26f, 1.0f),
    }
  }) {
  }
}

func void
UI_DemoSizeBar() {
  UI_WidgetBlock({
    .label = Str8C("SideBar"),
    .flags = UI_WidgetFlag_DrawBackground,
    .layout = {
      .width = UI_PercentSize(0.25f),
      .height = UI_PercentSize(1.0f),
      .direction = UI_Axis_Y,
      .paddings = UI_PaddingAll(8.0f),
      .child_gap = 5.0f,
    },
    .style = {
      .background_color = MakeVec4F32(0.14f, 0.12f, 0.16f, 1.0f),
    }
  }) {
    for (I32 file_button_index = 0; file_button_index < 5; file_button_index += 1) {
      UI_DemoFileButton();
    }
  }
}

func void
Demo_BuildUI(OS_Window* window) {
  Vec2F32 mouse_position = OS_MousePosition(window);
  Vec2F32 mouse_scroll = MakeVec2F32(0.0f, 0.0f);

  UI_SelectContext(ui_demo.ui_context);
  UI_BeginFrame(ui_demo.dt, OS_MousePosition(window), MakeVec2F32(0.0f, 0.0f)); {
    UI_WidgetBlock({
      .label = Str8C("MainCanvas"),
      .flags = UI_WidgetFlag_DrawBackground,
      .layout = {
        .width = UI_PixelSize(window->size.w),
        .height = UI_PixelSize(window->size.h),
        .direction = UI_Axis_Y,
      },
      .style = {
        .background_color = MakeVec4F32(0.0f, 0.0f, 0.0f, 1.0f),
      }
    }) {
      UI_DemoTopBar();
      UI_DemoSizeBar();
    }
  }
  ui_demo.ui_draw_commands = UI_EndFrame();
}

func void
Demo_Render(RHI_CommandBuffer command_buffer) {
  RHI_BeginCommandBuffer(command_buffer); {
    RHI_ColorTarget color_target = {
      .texture = RHI_AcquireSwapchainTexture(command_buffer),
      .load_operation = RHI_AttachmentLoadOperation_Clear,
      .store_operation = RHI_AttachmentStoreOperation_Store,
      .clear_color = MakeVec4F32(0.1f, 0.2f, 0.1f, 1.0f),
    };
    RHI_RenderPass* render_pass = RHI_BeginRenderPass(command_buffer, 1, &color_target, 0, 0, 0); {
      RHI_SetViewport(command_buffer, (RectI32){.x = 0, .y = 0, .w = ui_demo.window->size.x, .h = ui_demo.window->size.h});
      RHI_SetScissor(command_buffer, (RectI32){.x = 0, .y = 0, .w = ui_demo.window->size.x, .h = ui_demo.window->size.h});

      for (UI_DrawCommand* draw_command = ui_demo.ui_draw_commands; draw_command != 0; draw_command = draw_command->next) {

        switch (draw_command->kind) {
          default: {
            Assert(0 && "Unsupported UI draw command");
          } break;

          case UI_DrawCommandKind_Rectangle: {
            RectF32 bound = draw_command->rectangle.bounding_box;

            struct {
              Mat4F32 projection;
              Vec4F32 position_size;
              Vec4F32 color;
            } data = {
              .projection = MakeOrthographicMat4F32(0.0f, ui_demo.window->size.w, ui_demo.window->size.h, 0.0f, -1.0f, 1.0f),
              .position_size = MakeVec4F32(bound.x, bound.y, bound.w, bound.h),
              .color = draw_command->rectangle.background_color,
            };
            U64 data_offset = RHI_PushBuffer(ui_demo.gpu_buffer, (U8*)&data, sizeof(data));

            RHI_ShaderArgument arguments[] = {
              {
                .kind = RHI_ShaderArgumentKind_BufferAddress,
                .address = RHI_BufferDeviceAddress(ui_demo.gpu_buffer) + data_offset,
              }
            };
            RHI_BindGraphicsPipeline(command_buffer, ui_demo.rectangle_pipeline);
            RHI_BindShaderArguments(command_buffer, RHI_ShaderKind_Vertex|RHI_ShaderKind_Fragment, arguments, ArrayLength(arguments));
            RHI_DrawPrimitives(command_buffer, 6, 1, 0, 0);
          } break;
        }
      }
    }
    RHI_EndRenderPass(command_buffer, render_pass);
  }
  RHI_EndCommandBuffer(command_buffer);

  RHI_SubmitCommandBuffer(command_buffer, 0, 0, 0, 0);
  RHI_Present(command_buffer);
}

I32 main() {
  Arena* global_arena = AllocateArena(Gigabytes(8), Kilobytes(64));
  Arena* frame_arena = AllocateArena(Gigabytes(8), Kilobytes(64));
  B32 finished = 0;

  OS_Init(Megabytes(64));
  ui_demo.window = OS_CreateWindow(Str8C("UI Demo"), MakeVec2U32(1280, 720));

  RHI_Init(ui_demo.window);
  RHI_CommandBuffer command_buffer = RHI_GetCommandBuffer();
  ui_demo.gpu_buffer = RHI_CreateBuffer(Str8C("UI_Demo_Buffer"), Megabytes(16), RHI_BufferUsageFlag_Storage|RHI_BufferUsageFlag_Address, RHI_BufferPropertyFlag_HostVisible|RHI_BufferPropertyFlag_HostCoherent);
  {
    RHI_ShaderArgumentKind vertex_shader_arguments[] = {
      RHI_ShaderArgumentKind_BufferAddress,
    };
    RHI_Shader vertex_shader = RHI_CreateShader(global_arena, &(RHI_ShaderCreateInfo) {
      .file_name = Str8C("./data/shaders/draw/rectangle.vs"),
      .kind = RHI_ShaderKind_Vertex,
      .arguments = vertex_shader_arguments,
      .arguments_count = ArrayLength(vertex_shader_arguments),
    });

    RHI_ShaderArgumentKind fragment_shader_arguments[] = {
      RHI_ShaderArgumentKind_BufferAddress,
    };
    RHI_Shader fragment_shader = RHI_CreateShader(global_arena, &(RHI_ShaderCreateInfo) {
      .file_name = Str8C("./data/shaders/draw/rectangle.fs"),
      .kind = RHI_ShaderKind_Fragment,
      .arguments = fragment_shader_arguments,
      .arguments_count = ArrayLength(fragment_shader_arguments),
    });

    ui_demo.rectangle_pipeline = RHI_CreateGraphicsPipeline(&(RHI_GraphicsPipelineCreateInfo) {
      .vertex_shader = &vertex_shader,
      .fragment_shader = &fragment_shader,
      .color_targets_count = 1,
      .color_target_infos = &(RHI_GraphicsPipelineColorTargetInfo) {
        .format = RHI_GetSwapchainTextureFormat(),
      }
    });
  }

  ui_demo.ui_context = UI_CreateContext();

  ui_demo.colors[0] = MakeVec4F32(0.0f, 0.50f, 0.24f, 1.0f),
  ui_demo.colors[1] = MakeVec4F32(0.08f, 0.25f, 0.12f, 1.0f),
  ui_demo.colors[2] = MakeVec4F32(0.18f, 0.15f, 0.32f, 1.0f),
  ui_demo.colors[3] = MakeVec4F32(0.48f, 0.65f, 0.21f, 1.0f),
  ui_demo.current_color_index = 0;
  ui_demo.color_animation = 0;
  ui_demo.animation_duration = 2.0f;
  ui_demo.animation_time = 2.0f;

  ui_demo.slider_value = 5.0f;

  OS_ShowWindow(ui_demo.window);

  U64 start_ts = OS_GetTimeTicks();
  while (!finished) {
    OS_EventList events = OS_DispatchEvents(frame_arena, ui_demo.window);

    if (OS_KeyPressed(OS_KEY_ESC)) {
      finished = 1;
    }

    Demo_BuildUI(ui_demo.window);

    Demo_Render(command_buffer);

    U64 end_ts = OS_GetTimeTicks();
    ui_demo.dt = (F32)(end_ts - start_ts)*0.001f;
    start_ts = end_ts;
  }

  return 0;
}
