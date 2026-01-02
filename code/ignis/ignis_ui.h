#pragma once

typedef struct Ignis_UI_Colors Ignis_UI_Colors;
struct Ignis_UI_Colors
{
  Vec4F32 text;
  Vec4F32 accent;
  Vec4F32 bg;
  Vec4F32 bg_tint;
  Vec4F32 button;
  Vec4F32 hover;
};

typedef struct Ignis_UI_State Ignis_UI_State;
struct Ignis_UI_State
{
  Ignis_UI_Colors colors;
  FontBitmap      font;

  // Component Descriptions
  UI_RectangleDescription rectangle;
  UI_TextDescription      text;
  UI_TextDescription      title_text;
} _ignis_ui_state;

func void Ignis_UI_Init(Arena* arena, U32 max_widgets_count);
func void Ignis_UI_ApplyColors();

// --AlNov 26 December 2025: @TODO pointer_position should be I32
func void                Ignis_UI_Configure      (Ignis_Scene* scene, Vec2I32 context_size, Vec2F32 pointer_position, F32 dt);
func UI_DrawCommandArray Ignis_UI_GetDrawCommands();

// -------------------------------------------------------------------
// -- Ignis UI Components --------------------------------------------
func void Ignis_UI_Text(Str8 str);
func void Ignis_UI_Title(Str8 str);

func B32 Ignis_UI_Button(Str8 label, UI_Size width, UI_Size height);

func void Ignis_UI_SideBar(Ignis_Scene* scene);
func void Ignis_UI_SceneDetails(Ignis_Scene* scene);
func void Ignis_UI_EntityDetails(Ignis_Entity* entity);
