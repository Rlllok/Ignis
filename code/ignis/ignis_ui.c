#include "ignis_ui.h"

func void
Ignis_UI_Init(Arena* arena, U32 max_widgets_count)
{
  _ignis_ui_state = (Ignis_UI_State){0};

#if 0
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

  app_state.font.bitmap = R_CreateTexture(
    &(R_TextureCreateInfo){
      .type = R_TEXTURE_TYPE_2D,
      .format = R_TEXTURE_FORMAT_R8G8B8A8_SRGB,
      .usage_flags = R_TEXTURE_USAGE_FLAG_SAMPLED | R_TEXTURE_USAGE_FLAG_TRANSFER_DST,
      .width = tex_width,
      .height = tex_height,
      .depth = 1,
      .num_levels = 1,
    }
  );
  app_state.font.bitmap_size = MakeVec2U32(tex_width, tex_height);
  app_state.font.glyph_size = MakeVec2U32(30, 30);
  app_state.font.glyphs_per_row = 19;
  U64 font_texture_offset = R_PushBuffer(data_buffer, tex_pixels, texture_size);
  R_CopyBufferToTexture(0, data_buffer, font_texture_offset, texture_size, app_state.font.bitmap);
  #endif

  UI_Init(arena, max_widgets_count);
  Ignis_UI_ApplyColors();
  
  _ignis_ui_state.rectangle = (UI_RectangleDescription){
    .color  = _ignis_ui_state.colors.bg,
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
  _ignis_ui_state.colors.bg      = RGBAFromHex(0x32383bff);
  _ignis_ui_state.colors.bg_tint = RGBAFromHex(0x474c4fff);
  _ignis_ui_state.colors.button  = RGBAFromHex(0x39394eff);
  _ignis_ui_state.colors.hover   = RGBAFromHex(0x44445cff);
}

func void
Ignis_UI_Configure(Vec2I32 context_size, Vec2F32 pointer_position, F32 dt)
{
  DeferBlock(UI_BeginFrame(pointer_position, OS_MouseScroll()), UI_EndFrame())
  {
    UI_ElementBlock({
      .name = Str8C("Ignis_UI_Context"),
      .layout = {
        .width = UI_PixelSize(context_size.x),
        .height = UI_PixelSize(context_size.y),
        .direction = UI_LayoutDirection_LeftToRight,
      },
    })
    {
      Ignis_UI_SideBar();
    }
  }
}
func void Ignis_UI_Draw();

// -------------------------------------------------------------------
// -- Ignis UI Components --------------------------------------------
func void
Ignis_UI_Text(Str8 str)
{
  UI_Text(str, _ignis_ui_state.text);
}

func void
Ignis_UI_Button(Str8 label, UI_Size width, UI_Size height)
{
  UI_ElementBlock({
    .flags  = UI_ElementFlag_DrawBackground,
    .name   = Str8C("Ignis_Button"),
    .layout = {
      .width  = width,
      .height = height,
    },
    .rectangle = {
      .color = UI_Hovered() ? _ignis_ui_state.colors.hover : _ignis_ui_state.colors.button,
    },
  })
  {
    Ignis_UI_Text(label);
  }
}

func void
Ignis_UI_Title(Str8 str)
{
  UI_Text(str, _ignis_ui_state.title_text);
}

func void
Ignis_UI_SideBar()
{
  UI_ElementBlock({
    .flags = UI_ElementFlag_DrawBackground,
    .name = Str8C("Ignis_SideBar"),
    .layout = {
      .width  = UI_PercentSize(0.3f),
      .height = UI_PercentSize(1.0f),
    },
    .rectangle = _ignis_ui_state.rectangle,
  })
  {
    UI_ElementBlock({
      .name = Str8C("Ignis_SideBar_Top"),
      .layout = {
        .width  = UI_PercentSize(1.0f),
        .height = UI_PercentSize(0.5f),
      }
    })
    {
      Ignis_UI_SceneDetails();
    }

    UI_ElementBlock({
      .name = Str8C("Ignis_SideBar_Bottom"),
      .layout = {
        .width  = UI_PercentSize(1.0f),
        .height = UI_PercentSize(0.5f),
      }
    })
    {
      Ignis_UI_EntityDetails();
    }
  }
}

func void
Ignis_UI_SceneDetails()
{
  UI_ElementBlock({
    .name = Str8C("Ignis_SceneDetails"),
    .layout = {
      .width  = UI_PercentSize(1.0f),
      .height = UI_PercentSize(1.0f),
    }
  })
  {
    Ignis_UI_Title(Str8C("Scene"));
  }
}

func void Ignis_UI_EntityDetails()
{
  UI_ElementBlock({
    .name = Str8C("Ignis_EntityDetails"),
    .layout = {
      .width  = UI_PercentSize(1.0f),
      .height = UI_PercentSize(1.0f),
    }
  })
  {
    Ignis_UI_Title(Str8C("Entity"));
    Ignis_UI_Button(Str8C("Button Test"), UI_PercentSize(1.0f), UI_PixelSize(40.0f));
  }
}
