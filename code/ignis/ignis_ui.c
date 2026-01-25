#include "ignis_ui.h"

func void
Ignis_UI_Init(Arena* arena, U32 max_widgets_count)
{
  _ignis_ui_state = (Ignis_UI_State){0};
  _ignis_ui_state.arena = arena;

  Str8 texture_path = Str8C("./data/fonts/RobotoMonoBitmap.png");
  I32 tex_width = 0;
  I32 tex_height = 0;
  I32 tex_channels = 0;
  U8* tex_pixels = stbi_load(CFromStr8(texture_path), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);

  if (!tex_pixels)
  {
    LOG_ERROR("Cannot load texture %s\n", CFromStr8(texture_path));
  }
  I32 texture_size = tex_width * tex_height * 4;

  _ignis_ui_state.font.bitmap = RHI_CreateTexture(
    &(RHI_TextureCreateInfo){
      .type = RHI_TEXTURE_TYPE_2D,
      .format = RHI_TEXTURE_FORMAT_R8G8B8A8_SRGB,
      .usage_flags = RHI_TEXTURE_USAGE_FLAG_SAMPLED | RHI_TEXTURE_USAGE_FLAG_TRANSFER_DST,
      .width = tex_width,
      .height = tex_height,
      .depth = 1,
      .num_levels = 1,
    }
  );
  _ignis_ui_state.font.bitmap_size = MakeVec2U32(tex_width, tex_height);
  _ignis_ui_state.font.glyph_size = MakeVec2U32(30, 30);
  _ignis_ui_state.font.glyphs_per_row = 19;
  U64 font_texture_offset = RHI_PushBuffer(_ignis_r_state.transfer_buffer, tex_pixels, texture_size);
  RHI_CopyBufferToTexture(0, _ignis_r_state.transfer_buffer, font_texture_offset, texture_size, _ignis_ui_state.font.bitmap);

  UI_Init(arena, max_widgets_count);
  Ignis_UI_ApplyColors();
  
  _ignis_ui_state.bg_rectangle = (UI_RectangleDescription){
    .color  = _ignis_ui_state.colors.bg0,
    .radius = {0.0f, 0.0f, 0.0f, 0.0f},
  };
  _ignis_ui_state.rectangle = (UI_RectangleDescription){
    .color  = _ignis_ui_state.colors.bg1,
    .radius = {0.0f, 0.0f, 0.0f, 0.0f},
  };
  _ignis_ui_state.text = (UI_TextDescription){
    .font      = _ignis_ui_state.font,
    .font_size = 20,
    .color     = _ignis_ui_state.colors.text,
  };
  _ignis_ui_state.title_text = (UI_TextDescription){
    .font      = _ignis_ui_state.font,
    .font_size = 28,
    .color     = _ignis_ui_state.colors.accent,
  };
}

func void
Ignis_UI_ApplyColors()
{
  _ignis_ui_state.colors.text    = RGBAFromHex(0xf9e2afff);
  _ignis_ui_state.colors.accent  = RGBAFromHex(0xfcbf3bff);
  _ignis_ui_state.colors.bg0     = RGBAFromHex(0x32383bff);
  _ignis_ui_state.colors.bg1     = RGBAFromHex(0x40484cff);
  _ignis_ui_state.colors.bg2     = RGBAFromHex(0x585b70ff);
  _ignis_ui_state.colors.bg_tint = RGBAFromHex(0x474c4fff);
  _ignis_ui_state.colors.button  = RGBAFromHex(0x39394eff);
  _ignis_ui_state.colors.hover   = RGBAFromHex(0x44445cff);
}

func void
Ignis_UI_BeginFrame(Vec2I32 context_size, Vec2F32 pointer_position, F32 dt)
{
  UI_BeginFrame(pointer_position, OS_MouseScroll());
  UI_OpenWidget();
  UI_ConfigureWidget((UI_WidgetDescription){
    .name = Str8C("Ignis_UI_Context"),
    .layout = {
      .width = UI_PixelSize(context_size.x),
      .height = UI_PixelSize(context_size.y),
      .direction = UI_LayoutDirection_LeftToRight,
    },
  });
}

func void
Ignis_UI_EndFrame()
{
  UI_CloseWidget();
  UI_EndFrame();
}

func void
Ignis_UI_Editor(Ignis_Scene* scene)
{
  Ignis_UI_SideBar(scene);
}

func void
Ignis_UI_Performance(F32 dt)
{
  ScratchArena scratch = BeginScratchArena(_ignis_ui_state.arena);
  {
    UI_WidgetBlock({
      .layout = {
        .width = UI_PercentSize(1.0f),
        .height = UI_PixelSize(30),
        .direction = UI_LayoutDirection_TopToBottom,
      },
    })
    {
      Ignis_UI_Text(FormatStr8(scratch.arena, "Frame Time: %fms", dt));
    }

#if 0
    U64 total_ts = vei_state.end_ts - vei_state.start_ts;
    UI_WidgetBlock({
      .layout = {
        .width = UI_PercentSize(1.0f),
        .height = UI_PixelSize(30),
      },
    })
    {
      Ignis_UI_Text(FormatStr8(scratch.arena, "Total: %u\n", total_ts));
    }

    for (I32 i = 1; i < vei_state.points_length; i += 1)
    {
      UI_WidgetBlock({
        .layout = {
          .width = UI_PercentSize(1.0f),
          .height = UI_PixelSize(30),
        },
      })
      {
        Vei_Point* point = vei_state.points + i;

        F64 percent          = 100*((F64)point->exclusive_ts/(F64)total_ts);
        F64 percent_children = 100*((F64)point->inclusive_ts/(F64)total_ts);
        Ignis_UI_Text(FormatStr8(scratch.arena, "%s %u clocks", point->name, point->exclusive_ts));
      }
    }
#endif
  }
  EndScratchArena(scratch);
}

func UI_DrawCommandArray
Ignis_UI_GetDrawCommands()
{
  return ui_context.draw_commands;
}

// -------------------------------------------------------------------
// -- Ignis UI Components --------------------------------------------
func void
Ignis_UI_Text(Str8 str)
{
  UI_Text(str, _ignis_ui_state.text);
}

func B32
Ignis_UI_Button(Str8 label, UI_Size width, UI_Size height)
{
  B32 result = 0;
  
  UI_WidgetBlock({
    .flags  = UI_WidgetFlag_DrawBackground,
    .name   = label,
    .layout = {
      .width  = width,
      .height = height,
    },
    .rectangle = {
      .color = UI_Hovered() ? _ignis_ui_state.colors.hover : _ignis_ui_state.colors.bg1,
    },
  })
  {
    Ignis_UI_Text(label);

    result = UI_Hovered() && OS_MousePressed(OS_MouseButton_Left);
  }

  return result;
}

func void
Ignis_UI_Title(Str8 str)
{
  UI_Text(str, _ignis_ui_state.title_text);
}

func void
Ignis_UI_SideBar(Ignis_Scene* scene)
{
  UI_WidgetBlock({
    .flags = UI_WidgetFlag_DrawBackground,
    .name = Str8C("Ignis_SideBar"),
    .layout = {
      .width  = UI_PercentSize(0.3f),
      .height = UI_PercentSize(1.0f),
      .padding = UI_EqualPadding(4),
      .child_gap = 4,
    },
    .rectangle = _ignis_ui_state.bg_rectangle,
  })
  {
    UI_WidgetBlock({
      .flags = UI_WidgetFlag_DrawBackground,
      .name = Str8C("Ignis_SideBar_Top"),
      .layout = {
        .width  = UI_PercentSize(1.0f),
        .height = UI_PercentSize(0.25f),
        .clip = 1,
      },
      .rectangle = {
        .color  = _ignis_ui_state.colors.bg1,
        .radius = {8.0f, 8.0f, 0.0f, 0.0f},
      },
    })
    {
      Ignis_UI_SceneDetails(scene);
    }

    UI_WidgetBlock({
      .flags = UI_WidgetFlag_DrawBackground,
      .name = Str8C("Ignis_SideBar_Bottom"),
      .layout = {
        .width  = UI_PercentSize(1.0f),
        .height = UI_PercentSize(0.75f),
        .clip = 1,
      },
      .rectangle = {
        .color  = _ignis_ui_state.colors.bg1,
        .radius = {0.0f, 0.0f, 4.0f, 4.0f},
      },
    })
    {
      Ignis_Entity* selected_entity = Ignis_GetSelectedEntity(scene);
      if (Ignis_EntityValid(selected_entity))
      {
        Ignis_UI_EntityDetails(selected_entity);
      }
    }
  }
}

func void
Ignis_UI_SceneDetails(Ignis_Scene* scene)
{
  UI_WidgetBlock({
    .name = Str8C("Ignis_SceneDetails"),
    .layout = {
      .width  = UI_PercentSize(1.0f),
      .height = UI_PercentSize(1.0f),
    }
  })
  {
    Ignis_UI_Title(Str8C("Scene"));
    UI_WidgetBlock({
      .name = Str8C("Ignis_SceneDetails_Header"),
      .layout = {
        .width = UI_PercentSize(1.0f),
        .height = UI_PixelSize(2.0f),
      },
    });
    {
      Ignis_UI_Text(Str8C("Entity Name"));
    }
    for (I32 i = 1; i < scene->entities.length; i += 1)
    {
      Ignis_Entity* entity = Ignis_EntityArrayGetPointer(&scene->entities, i);
      if (Ignis_UI_Button(entity->name, UI_PercentSize(1.0f), UI_PixelSize(25.0f)))
      {
        Ignis_SetSelectedEntity(scene, entity);
      }
    }
  }
}

func void Ignis_UI_EntityDetails(Ignis_Entity* entity)
{
  UI_WidgetBlock({
    .name = Str8C("Ignis_EntityDetails"),
    .layout = {
      .width  = UI_PercentSize(1.0f),
      .height = UI_PercentSize(1.0f),
    }
  })
  {
    Ignis_UI_Title(entity->name);
  }
}
