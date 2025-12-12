#pragma once

typedef U32 UI_ID;

// -------------------------------------------------------------------
// -- UI Font --------------------------------------------------------
typedef struct FontBitmap FontBitmap;
struct FontBitmap
{
  R_Texture bitmap;
  Vec2U32 bitmap_size; // --AlNov: @TODO Should be in texture
  Vec2U32 glyph_size;
  U32 glyphs_per_row;
};

typedef struct TextVertex TextVertex;
struct TextVertex
{
  Vec2 position;
  Vec2 uv;
};

func Vec2 GetTextSize(FontBitmap font, Str8 text, U32 font_size);

typedef U8 UI_PositionType;
enum UI_PositionTypeEnum
{
  UI_Position
} UI_PositionTypeEnum;

typedef U8 UI_LayoutDirection;
enum UI_LayoutDirectionEnum
{
  UI_LayoutDirection_TopToBottom,
  UI_LayoutDirection_LeftToRight,
} UI_LayoutDirectionEnum;

typedef U8 UI_SizeType;
enum UI_SizeTypeEnum
{
  UI_SizeType_None,
  UI_SizeType_Fixed,
  UI_SizeType_WrapLabel,
  UI_SizeType_WrapChildren,
  UI_SizeType_ParentPercent,
  UI_SizeType_Count,
} UI_SizeTypeEnum;

typedef struct UI_Size UI_Size;
struct UI_Size
{
  UI_SizeType type;
  F32 value;
};

#define UI_FixedSize(coordinate) ((UI_Size){.type = UI_SizeType_Fixed, .value = coordinate})
#define UI_WrapLabelSize() ((UI_Size){.type = UI_SizeType_WrapLabel})
#define UI_ParentPercentSize(percent) ((UI_Size){.type = UI_SizeType_ParentPercent, .value = percent})

typedef struct UI_BorderRadius UI_BorderRadius;
struct UI_BorderRadius
{
  union
  {
    Vec4F32 values;
    struct 
    {
      F32 top_left;
      F32 top_right;
      F32 bottom_left;
      F32 bottom_right;
    };
  };
};

typedef struct UI_Padding UI_Padding;
struct UI_Padding
{
  union
  {
    Vec4F32 v;
    struct
    {
      F32 top;
      F32 right;
      F32 bottom;
      F32 left;
    };
  };
};

typedef U16 UI_ElementFlags;
enum UI_ElementFlagEnum
{
  // Interaction Flags
  UI_ElementFlag_Hover = 1 << 0,
  UI_ElementFlag_Clickable = 1 << 1,

  // Draw Flags
  UI_ElementFlag_DrawBackground = 1 << 2,
  UI_ElementFlag_DrawLabel = 1 << 3,
} UI_ElementFlagEnum;

typedef struct UI_Element UI_Element;
struct UI_Element
{
  UI_ID id;

  UI_Element* next;
  UI_Element* previous;
  UI_Element* parent;

  Str8 label;
  UI_ElementFlags flags;
  FontBitmap font;
  Vec4 text_color;
  U32 font_size;
  RectF32 rect;
  UI_LayoutDirection layout;
  UI_Padding padding;
  F32 child_gap;
  Vec2 child_offset;
  Vec4 background_color;
  UI_BorderRadius border_radius;
};

UI_Element UI_ElementDefaultValue = {0};
DefineArray(UI_Element, UI_ElementArray, UI_ElementDefaultValue)

typedef U8 UI_DrawCommandType;
enum UI_DrawCommandTypeEnum
{
  UI_DrawCommandType_Rectangle,
  UI_DrawCommandType_Text,
} UI_DrawCommandTypeEnum;

typedef struct UI_DrawCommand UI_DrawCommand;
struct UI_DrawCommand
{
  UI_DrawCommandType type;
  union
  {
    struct
    {
      Vec4 color;
      RectF32 bound;
      Vec4 radius;
    } rectangle;

    struct
    {
      Str8 content;
      FontBitmap font;
      F32 font_size;
      Vec4 color;
      Vec2 position;
    } text;
  };
};
UI_DrawCommand UI_DrawCommandDefaultValue = {0};
DefineArray(UI_DrawCommand, UI_DrawCommandArray, UI_DrawCommandDefaultValue)

typedef struct UI_Context UI_Context;
struct UI_Context
{
  UI_Element* current_parent;

  UI_Size size_x;
  UI_Size size_y;
  Vec2 fixed_position;
  U32 font_size;
  FontBitmap font;
  Vec4 text_color;
  Vec4 background_color;

  // Interaction
  UI_ID hot_id;
  UI_ID active_id;
  Vec2 mouse_position;

  // Draw
  UI_DrawCommandArray draw_commands;

  UI_ElementArray elements;
} ui_context; // -- AlNov. 12 December 2025: @TODO Multiole contexts?

// -------------------------------------------------------------------
// -- UI Context Mutation --------------------------------------------
func void UI_BeginFrame(Vec2 mouse_position);
func void UI_EndFrame(UI_Context* context);

func void UI_SetParent(UI_Element* parent)           {ui_context.current_parent = parent;}
func void UI_SetSizeX(UI_Size size)                  {ui_context.size_x = size;}
func void UI_SetSizeY(UI_Size size)                  {ui_context.size_y = size;}
func void UI_SetFont(FontBitmap font, U32 font_size) {ui_context.font = font; ui_context.font_size = font_size;}
func void UI_SetTextColor(Vec4 color)                {ui_context.text_color = color;}
func void UI_SetBackgroundColor(Vec4 color)          {ui_context.background_color = color;}
func void UI_SetFixedPosition(Vec2 position)         {ui_context.fixed_position = position;}

// -------------------------------------------------------------------
// -- UI Elements ----------------------------------------------------
typedef struct UI_ElementDescription UI_ElementDescription;
struct UI_ElementDescription
{
  Str8 label;
  UI_ElementFlags flags;
  UI_LayoutDirection layout;
  UI_Padding padding;
  F32 child_gap;
  UI_BorderRadius border_radius;
};

func UI_Element* UI_BuildElement(UI_ElementArray* array, UI_ElementDescription description);

func UI_Element* UI_Layout(UI_ElementArray* array, Str8 label);
func void        UI_Text(UI_ElementArray* array, Str8 label);
func void        UI_NumberInput(UI_ElementArray* array, Str8 label, F32* value);
func B32         UI_Button(UI_ElementArray* array, Str8 label);
func F32         UI_SliderF32(UI_ElementArray* array, Str8 label, F32 min, F32 max, F32* value);

