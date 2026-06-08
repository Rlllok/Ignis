#include "base/base_include.h"
#include "os/os_include.h"
#include "rhi/rhi_include.h"
#include "assets/font.h"
#include "ui_new/ui_include.h"

#include "base/base_include.c"
#include "os/os_include.c"
#include "rhi/rhi_include.c"
#include "assets/font.c"
#include "ui_new/ui_include.c"

typedef struct UI_DemoCategory UI_DemoCategory;
struct UI_DemoCategory {
  Str8 name;
  void (*BuildUI)();
};
#define UI_DemoCategory_Capacity 32

static struct {
  Arena*               global_arena;
  Arena*               frame_arena;
  OS_Window*           window;
  RHI_Buffer           gpu_buffer;
  RHI_Buffer           transfer_buffer;
  RHI_GraphicsPipeline rectangle_pipeline;
  RHI_GraphicsPipeline text_pipeline;
  RHI_ResourceTable    resource_table;
  RHI_TextureSampler   default_sampler;

  UI_Context*     ui_context;
  UI_DrawCommand* ui_draw_commands;

  F32 dt;

  struct {
    AST_Font font;
    Vec4F32  color;
    Vec4F32  accent_color;
    Vec4F32  background_color;
    Vec4F32  background_color_dim;
  } style;

  // --AlNov: @TODO For testing 
  RHI_TextureDeviceId glyph_texture_device_ids[96];
  RHI_SamplerDeviceId test_sampler_id;

  UI_DemoCategory categories[UI_DemoCategory_Capacity];
  I32             categories_length;
  I32             current_category_index;
} ui_demo;

func B32
UI_DemoCategoryButton(Str8 label, B32 active) {
  B32 result = 0;
  Vec4F32 color = active ? ui_demo.style.background_color : ui_demo.style.color;
  Vec4F32 background_color = active ? ui_demo.style.accent_color : ui_demo.style.background_color_dim;
  Vec4F32 border_color = active ? ui_demo.style.accent_color : ui_demo.style.background_color_dim;
  UI_WidgetBlock(
    label,
    {
      .flags = UI_WidgetFlag_MouseInteraction|UI_WidgetFlag_DrawText|UI_WidgetFlag_DrawBackground,
      .layout = {
        .width = UI_PercentSize(1.0f),
        .height = UI_PixelSize(40.0f),
      },
      .style = {
        .radius = MakeVec4F32(0.0f, 0.0f, 15.0f, 15.0f),
        .background_color = background_color,
        .border_width = 1.0f,
        .border_color = UI_IsHot() ? ui_demo.style.accent_color : border_color,
      },
      .text = {
        .font = &ui_demo.style.font,
        .alignment = UI_TextAlignment_Center,
        .color = (!active && UI_IsHot()) ? ui_demo.style.accent_color : color,
        .str = label,
      }
    }
  ) {
    if (UI_IsHot() && OS_MousePressed(OS_MouseButton_Left)) {
      result = 1;
    }
  }
  return result;
}

func void
UI_DemoSideBar() {
  UI_WidgetBlock(
    Str8C("SideBar"),
    {
      .flags = UI_WidgetFlag_DrawBackground,
      .layout = {
        .width = UI_PercentSize(0.25f),
        .height = UI_PercentSize(1.0f),
        .direction = UI_Axis_Y,
        .paddings = MakeVec4F32(0.0f, 30.0f, 20.0f, 20.0f),
        .child_gap = 10.0f,
      },
      .style = {
        .background_color = ui_demo.style.background_color,
      }
    }
  ) {
    for (I32 category_index = 0; category_index < ui_demo.categories_length; category_index += 1) {
      if (UI_DemoCategoryButton(ui_demo.categories[category_index].name, ui_demo.current_category_index == category_index)) {
        ui_demo.current_category_index = category_index;
      }
    }
  }
}

func void
UI_DemoCategoryLayoutDirectionX() {
  UI_WidgetBlock(
    Str8C("CategoryLayoutDirectionX"),
    {
      .layout = {
        .width = UI_PercentSize(1.0f),
        .height = UI_PercentSize(1.0f),
        .direction = UI_Axis_X,
        .child_gap = 15.0f,
      }
    }
  ) {
    UI_WidgetLayoutInfo layout_info = {
      .width = UI_FillSize(),
      .height = UI_PercentSize(1.0f),
    };
    UI_WidgetStyleInfo style_info = {
      .background_color = ui_demo.style.background_color_dim,
    };

    UI_WidgetBlock(
      Str8C("CategoryLayoutDirectionX_Child1"),
      {
        .flags = UI_WidgetFlag_DrawBackground,
        .layout = layout_info,
        .style = style_info,
      }
    ) {
    }
    UI_WidgetBlock(
      Str8C("CategoryLayoutDirectionX_Child2"),
      {
        .flags = UI_WidgetFlag_DrawBackground,
        .layout = layout_info,
        .style = style_info,
      }
    ) {
    }
    UI_WidgetBlock(
      Str8C("CategoryLayoutDirectionX_Child3"),
      {
        .flags = UI_WidgetFlag_DrawBackground,
        .layout = layout_info,
        .style = style_info,
      }
    ) {
    }
  }
}

func void
UI_DemoCategoryLayoutDirectionY() {
  UI_WidgetBlock(
    Str8C("CategoryLayoutDirectionY"),
    {
      .layout = {
        .width = UI_PercentSize(1.0f),
        .height = UI_PercentSize(1.0f),
        .direction = UI_Axis_Y,
        .child_gap = 15.0f,
      }
    }
  ) {
    UI_WidgetLayoutInfo layout_info = {
      .width = UI_PercentSize(1.0f),
      .height = UI_FillSize(),
    };
    UI_WidgetStyleInfo style_info = {
      .background_color = ui_demo.style.background_color_dim,
    };

    UI_WidgetBlock(
      Str8C("CategoryLayoutDirection_Child1"),
      {
        .flags = UI_WidgetFlag_DrawBackground,
        .layout = layout_info,
        .style = style_info,
      }
    ) {
    }
    UI_WidgetBlock(
      Str8C("CategoryLayoutDirection_Child2"),
      {
        .flags = UI_WidgetFlag_DrawBackground,
        .layout = layout_info,
        .style = style_info,
      }
    ) {
    }
    UI_WidgetBlock(
      Str8C("CategoryLayoutDirection_Child3"),
      {
        .flags = UI_WidgetFlag_DrawBackground,
        .layout = layout_info,
        .style = style_info,
      }
    ) {
    }
  }
}

func void
UI_DemoCategoryWidgets() {

  UI_WidgetBlock(
    Str8C("CategoryWidgets"),
    {
      .layout = {
        .width = UI_PercentSize(1.0f),
        .height = UI_PercentSize(1.0f),
        .direction = UI_Axis_Y,
        .child_gap = 10.0f,
      },
    }
  ) {
    UI_WidgetLayoutInfo button_layout = {
      .width = UI_PixelSize(100.0f),
      .height = UI_PixelSize(40.0f),
    };
    UI_WidgetStyleInfo button_style = {
      .radius = MakeVec4F32(3.0f, 3.0f, 3.0f, 3.0f),
      .background_color = ui_demo.style.background_color_dim,
    };
    Str8 label = Str8C("ButtonTest");
    UI_TextStyleInfo button_text = {
      .font = &ui_demo.style.font,
      .color = ui_demo.style.color,
      .str = label,
    };
    UI_Button(label, button_text, button_layout, button_style);

    UI_WidgetBlock(
      Str8C("RadioButtons"),
      {
        .layout = {
          .width = UI_PercentSize(1.0f),
          .height = UI_FitSize(),
          .child_gap = 5.0f,
        }
      }
    ) {
      UI_WidgetLayoutInfo radio_button_layout = {
        .width = UI_PixelSize(40.0f),
        .height = UI_PixelSize(40.0f),
        .paddings = UI_PaddingAll(7.0f),
      };
      local_persist I32 radio_button_value = 2;
      UI_RadioButton(Str8C("RadioButton_0"), &radio_button_value, 0, button_text, radio_button_layout, button_style);
      UI_RadioButton(Str8C("RadioButton_1"), &radio_button_value, 1, button_text, radio_button_layout, button_style);
      UI_RadioButton(Str8C("RadioButton_2"), &radio_button_value, 2, button_text, radio_button_layout, button_style);
      UI_RadioButton(Str8C("RadioButton_3"), &radio_button_value, 3, button_text, radio_button_layout, button_style);
    }

    UI_WidgetLayoutInfo slider_layout = {
      .width = UI_PercentSize(1.0f),
      .height = UI_PixelSize(40.0f),
      .paddings = UI_PaddingAll(5.0f),
    };
    local_persist F32 slider_f32_value = 0.0f;
    UI_TextStyleInfo slider_f32_text = button_text;
    slider_f32_text.str = FormatStr8(ui_demo.frame_arena, "%f", slider_f32_value);
    slider_f32_text.alignment = UI_TextAlignment_Center;
    UI_SliderF32(Str8C("SliderF32"), &slider_f32_value, -50.0f, 50.0f, slider_f32_text, slider_layout, button_style);
    local_persist I32 slider_i32_value = 0;
    UI_TextStyleInfo slider_i32_text = button_text;
    slider_i32_text.str = FormatStr8(ui_demo.frame_arena, "%i", slider_i32_value);
    slider_i32_text.alignment = UI_TextAlignment_Center;
    UI_SliderI32(Str8C("SliderI32"), &slider_i32_value, -5, 5, slider_i32_text, slider_layout, button_style);
  }
}

func void
Demo_BuildUI(OS_Window* window) {
  Vec2F32 mouse_position = OS_MousePosition(window);
  Vec2F32 mouse_scroll = MakeVec2F32(0.0f, 0.0f);

  UI_SelectContext(ui_demo.ui_context);
  UI_BeginFrame(ui_demo.dt, OS_MousePosition(window), MakeVec2F32(0.0f, 0.0f)); {
    UI_WidgetBlock(
      Str8C("MainCanvas"),
      {
        .flags = UI_WidgetFlag_DrawBackground,
        .layout = {
          .width = UI_PixelSize(window->size.w),
          .height = UI_PixelSize(window->size.h),
          .direction = UI_Axis_Y,
        },
        .style = {
          .background_color = MakeVec4F32(0.0f, 0.3f, 0.0f, 1.0f),
        }
      }
    ) {
      UI_WidgetBlock(
        Str8C("Body"),
        {
          .flags = UI_WidgetFlag_DrawBackground,
          .layout = {
            .width = UI_PercentSize(1.0f),
            .height = UI_FillSize(),
            .direction = UI_Axis_X,
            .child_gap = 0.0f,
          },
        }
      ) {
        UI_DemoSideBar();
        UI_WidgetBlock(
          Str8C("DemoCategory"),
          {
            .flags = UI_WidgetFlag_DrawBackground,
            .layout = {
              .width = UI_FillSize(),
              .height = UI_PercentSize(1.0f),
              .paddings = MakeVec4F32(0.0f, 30.0f, 20.0f, 20.0f),
            },
            .style = {
              .background_color = ui_demo.style.background_color,
            }
          }
        ) {
            Assert(ui_demo.categories[ui_demo.current_category_index].BuildUI != 0);
            ui_demo.categories[ui_demo.current_category_index].BuildUI();
        }
      }
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
              Vec4F32 radius;
              Vec4F32 color;
              Vec4F32 border_color;
              F32     border_width;
            } data = {
              .projection = MakeOrthographicMat4F32(0.0f, ui_demo.window->size.w, ui_demo.window->size.h, 0.0f, -1.0f, 1.0f),
              .position_size = MakeVec4F32(bound.x, bound.y, bound.w, bound.h),
              .radius = draw_command->rectangle.radius, 
              .color = draw_command->rectangle.background_color,
              .border_color = draw_command->rectangle.border_color,
              .border_width = draw_command->rectangle.border_width,
            };
            U64 data_offset = RHI_PushBuffer(ui_demo.gpu_buffer, (U8*)&data, sizeof(data));

            RHI_ShaderArgument arguments[] = {
              {
                .kind = RHI_ShaderArgumentKind_BufferAddress,
                .address = RHI_BufferDeviceAddress(ui_demo.gpu_buffer) + data_offset,
              }
            };
            RHI_BindGraphicsPipeline(command_buffer, ui_demo.rectangle_pipeline);
            RHI_BindResourceTable(command_buffer, ui_demo.resource_table);
            RHI_BindShaderArguments(command_buffer, RHI_ShaderKind_Vertex|RHI_ShaderKind_Fragment, arguments, ArrayLength(arguments));
            RHI_DrawPrimitives(command_buffer, 6, 1, 0, 0);
          } break;
          case UI_DrawCommandKind_Text: {
            F32 spacing = 0;
            RHI_BindGraphicsPipeline(command_buffer, ui_demo.text_pipeline);
            RHI_BindResourceTable(command_buffer, ui_demo.resource_table);
            for (I32 character_index = 0; character_index < draw_command->text.str.length; character_index += 1) {
              U8 character = draw_command->text.str.data[character_index];
              const AST_Font* font = draw_command->text.font;
              const AST_FontGlyph* glyph = font->glyphs + (character - 32);
              Vec2F32 position = draw_command->text.position;
              Vec2F32 size = MakeVec2F32(glyph->width, glyph->height);
              I32 x_offset = glyph->x_offset;
              I32 y_offset = glyph->y_offset;
              I32 ascent = font->ascent;
              F32 scale = font->scale;
              F32 advance = glyph->advance;

              if (character == ' ') {
                spacing += font->glyphs[' ' - 32].width;
              }

              struct {
                Mat4F32 projection;
                Vec4F32 position_size;
                Vec4F32 color;
                U32 texture_id;
              } glyph_data = {
                .projection = MakeOrthographicMat4F32(0.0f, ui_demo.window->size.w, ui_demo.window->size.y, 0.0f, -1.0f, 1.0f),
                .position_size = MakeVec4F32(position.x + x_offset + spacing, position.y + y_offset, size.x, size.y),
                .color = draw_command->text.color,
                .texture_id = ui_demo.glyph_texture_device_ids[character - 32],
              };
              U64 glyph_data_offset = RHI_PushBuffer(ui_demo.gpu_buffer, (U8*)&glyph_data, sizeof(glyph_data));

              RHI_ShaderArgument arguments[] = {
                {
                  .kind = RHI_ShaderArgumentKind_BufferAddress,
                  .address = RHI_BufferDeviceAddress(ui_demo.gpu_buffer) + glyph_data_offset,
                },
              };
              RHI_BindShaderArguments(command_buffer, RHI_ShaderKind_Vertex|RHI_ShaderKind_Fragment, arguments, ArrayLength(arguments));
              RHI_DrawPrimitives(command_buffer, 6, 1, 0, 0);

              spacing += (F32)(advance)*scale;
            }
          } break;
        }
      }
    }
    RHI_EndRenderPass(command_buffer, render_pass);
    RHI_Present(command_buffer);
  }
  RHI_EndCommandBuffer(command_buffer);

  RHI_SubmitCommandBuffer(command_buffer, 0, 0, 0, 0);
}

I32 main() {
  ui_demo.global_arena = AllocateArena(Gigabytes(8), Kilobytes(64));
  ui_demo.frame_arena = AllocateArena(Gigabytes(8), Kilobytes(64));
  B32 finished = 0;

  OS_Init(Megabytes(64));
  ui_demo.window = OS_CreateWindow(Str8C("UI Demo"), MakeVec2U32(1280, 720));

  RHI_Init(ui_demo.window);
  RHI_CommandBuffer command_buffer = RHI_GetCommandBuffer();
  ui_demo.gpu_buffer = RHI_CreateBuffer(Str8C("UI_Demo_Buffer"), Megabytes(16), RHI_BufferUsageFlag_Storage|RHI_BufferUsageFlag_Address, RHI_BufferPropertyFlag_HostVisible|RHI_BufferPropertyFlag_HostCoherent);
  ui_demo.transfer_buffer = RHI_CreateBuffer(Str8C("UI_Demo_TransferBuffer"), Megabytes(128), RHI_BufferUsageFlag_Transfer, RHI_BufferPropertyFlag_HostCoherent);
  {
    RHI_ShaderArgumentKind vertex_shader_arguments[] = {
      RHI_ShaderArgumentKind_BufferAddress,
    };
    RHI_Shader vertex_shader = RHI_CreateShader(ui_demo.global_arena, &(RHI_ShaderCreateInfo) {
      .file_name = Str8C("./data/shaders/draw/rectangle.vs"),
      .kind = RHI_ShaderKind_Vertex,
      .arguments = vertex_shader_arguments,
      .arguments_count = ArrayLength(vertex_shader_arguments),
    });

    RHI_ShaderArgumentKind fragment_shader_arguments[] = {
      RHI_ShaderArgumentKind_BufferAddress,
    };
    RHI_Shader fragment_shader = RHI_CreateShader(ui_demo.global_arena, &(RHI_ShaderCreateInfo) {
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
        .blend_enable = 1,
      }
    });
  }

  // Text Pipeline
  {
    RHI_ShaderArgumentKind arguments[] = {
      RHI_ShaderArgumentKind_BufferAddress,
    };

    RHI_Shader vertex_shader = RHI_CreateShader(
      ui_demo.global_arena,
      &(RHI_ShaderCreateInfo) {
        .file_name = Str8C("./data/TopDown/Shaders/text.vs"),
        .kind = RHI_ShaderKind_Vertex,
        .arguments = arguments,
        .arguments_count = ArrayLength(arguments),
      }
    );

    RHI_Shader fragment_shader = RHI_CreateShader(
      ui_demo.global_arena,
      &(RHI_ShaderCreateInfo) {
        .file_name = Str8C("./data/TopDown/Shaders/text.fs"),
        .kind = RHI_ShaderKind_Fragment,
        .arguments = arguments,
        .arguments_count = ArrayLength(arguments),
      }
    );

    ui_demo.text_pipeline = RHI_CreateGraphicsPipeline(
      &(RHI_GraphicsPipelineCreateInfo) {
        .vertex_shader = &vertex_shader,
        .fragment_shader = &fragment_shader,
        .color_targets_count = 1,
        .color_target_infos = &(RHI_GraphicsPipelineColorTargetInfo) {
          .format = RHI_GetSwapchainTextureFormat(),
          .blend_enable = 1,
        },
      }
    );
  }

  ui_demo.resource_table = RHI_CreateResourceTable(ui_demo.global_arena, 256, 1);
  ui_demo.default_sampler = RHI_CreateTextureSampler(
    &(RHI_TextureSamplerCreateInfo){
      .mag_filter = RHI_FilterKind_Linear,
      .min_filter = RHI_FilterKind_Linear,
      .address_mode_u = RHI_SamplerAddressMode_Repeat,
      .address_mode_v = RHI_SamplerAddressMode_Repeat,
      .address_mode_w = RHI_SamplerAddressMode_Repeat,
      .mipmap_mode = RHI_SamplerMipmapMode_Linear,
    }
  );
  ui_demo.test_sampler_id = RHI_ResourceTableAddSampler(ui_demo.resource_table, ui_demo.default_sampler);

  // setting up demo style
  ui_demo.style.font = AST_FontFromTTF(ui_demo.global_arena, command_buffer, ui_demo.transfer_buffer, Str8C("data/fonts/RobotoMono-Regular.ttf"), 24);
  ui_demo.style.color = RGBAFromHex(0xe0e0e0ff);
  ui_demo.style.accent_color = RGBAFromHex(0xc0fe04ff);
  ui_demo.style.background_color = RGBAFromHex(0x1b1b1bff);
  ui_demo.style.background_color_dim = RGBAFromHex(0x333333ff);

  for (I32 glyph_index = 0; glyph_index < ArrayLength(ui_demo.glyph_texture_device_ids); glyph_index += 1) {
    ui_demo.glyph_texture_device_ids[glyph_index] = RHI_ResourceTableAddTexture(ui_demo.resource_table, ui_demo.style.font.glyphs[glyph_index].texture);
  }

  // setting up demo categoies
  ui_demo.categories[0] = (UI_DemoCategory){
    .name = Str8C("Layout Direction X"),
    .BuildUI = UI_DemoCategoryLayoutDirectionX,
  };
  ui_demo.categories[1] = (UI_DemoCategory){
    .name = Str8C("Layout Direcition Y"),
    .BuildUI = UI_DemoCategoryLayoutDirectionY,
  };
  ui_demo.categories[2] = (UI_DemoCategory){
    .name = Str8C("Widgets"),
    .BuildUI = UI_DemoCategoryWidgets,
  };
  ui_demo.categories_length = 3;
  ui_demo.current_category_index = 1;

  ui_demo.ui_context = UI_CreateContext();

  OS_ShowWindow(ui_demo.window);

  U64 start_ts = OS_GetTimeTicks();
  while (!finished) {
    OS_EventList events = OS_DispatchEvents(ui_demo.frame_arena, ui_demo.window);

    if (OS_KeyPressed(OS_KEY_ESC)) {
      finished = 1;
    }

    Demo_BuildUI(ui_demo.window);

    Demo_Render(command_buffer);

    U64 end_ts = OS_GetTimeTicks();
    ui_demo.dt = (F32)(end_ts - start_ts)*0.001f;
    start_ts = end_ts;

    ResetArena(ui_demo.frame_arena);
  }

  return 0;
}
