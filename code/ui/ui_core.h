#pragma once

#include "base/base_include.h"
#include "render/r_include.h"

typedef I32 UI_ID;
#define UI_ID_Nil -1
UI_ID _ui_id_nil = UI_ID_Nil;
DefineArray(UI_ID, UI_IDArray, _ui_id_nil)

// -------------------------------------------------------------------
// -- UI Font --------------------------------------------------------
// -- AlNov. 12 December 2025: @TODO
// UI Layer should not depend on Render Layer. tIt should just build Layout and DrawCommands.
// (Remove usage for R_Texture from there)
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
  UI_SizeType_FitChildren,
  UI_SizeType_Percent,
  UI_SizeType_Pixel,
} UI_SizeTypeEnum;

typedef struct UI_Size UI_Size;
struct UI_Size
{
  UI_SizeType type;
  F32 value;
};

#define UI_FitChildrenSize()     ((UI_Size){.type = UI_SizeType_FitChildren, .value = 0.0f})
#define UI_PercentSize(percent)  ((UI_Size){.type = UI_SizeType_Percent,     .value = percent})
#define UI_PixelSize(coordinate) ((UI_Size){.type = UI_SizeType_Pixel,       .value = coordinate})

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

#define UI_EqualPadding(value) {value, value, value, value}

typedef U16 UI_WidgetFlags;
enum UI_WidgetFlagEnum
{
  // Interaction Flags
  UI_WidgetFlag_Hover = 1 << 0,
  UI_WidgetFlag_Clickable = 1 << 1,

  // Draw Flags
  UI_WidgetFlag_DrawBackground = 1 << 2,
  UI_WidgetFlag_DrawLabel = 1 << 3,
} UI_WidgetFlagEnum;

typedef struct UI_LayoutDescription UI_LayoutDescription;
struct UI_LayoutDescription
{
  UI_Size width;
  UI_Size height;
  UI_Padding padding;
  F32 child_gap;
  UI_LayoutDirection direction;
  B32 clip;
  Vec2I32 scroll_offset;
};

typedef struct UI_RectangleDescription UI_RectangleDescription;
struct UI_RectangleDescription
{
  Vec4F32 color;
  Vec4F32 border_color;
  UI_BorderRadius radius;
};

typedef struct UI_TextDescription UI_TextDescription;
struct UI_TextDescription
{
  Str8 str;
  FontBitmap font;
  Vec4F32 color;
  U32 font_size;
};

typedef U16 UI_WidgetType;
enum UI_WidgetTypeEnum
{
  UI_WidgetType_Rectangle,
  UI_WidgetType_Text
} UI_WidgetTypeEnum;

typedef struct UI_WidgetDescription UI_WidgetDescription;
struct UI_WidgetDescription
{
  Str8 name;

  UI_WidgetType type;
  UI_WidgetFlags flags;

  UI_LayoutDescription layout;
  union
  {
    UI_RectangleDescription rectangle;
    UI_TextDescription text;
  };
};

typedef struct UI_ScrollOffset UI_ScrollOffset;
struct UI_ScrollOffset
{
  UI_ID element_id;
  Vec2I32 offset;
};
UI_ScrollOffset _scroll_offset_nil = {0};
DefineArray(UI_ScrollOffset, UI_ScrollOffsetArray, _scroll_offset_nil)

typedef struct UI_Widget UI_Widget;
struct UI_Widget
{
  UI_ID id;
  UI_WidgetDescription description;

  // --AlNov 23 December 2025: @TEST
  UI_ID   clip_element_id;

  struct
  {
    UI_ID* ids;
    I32 length;
  } children_array_slice;

  RectF32 rect;
  Vec2 child_position_offset;
};

UI_Widget UI_WidgetDefaultValue = {0};
DefineArray(UI_Widget, UI_WidgetArray, UI_WidgetDefaultValue)

typedef U8 UI_DrawCommandType;
enum UI_DrawCommandTypeEnum
{
  UI_DrawCommandType_None,
  UI_DrawCommandType_Rectangle,
  UI_DrawCommandType_Text,
  UI_DrawCommandType_ScissorBegin,
  UI_DrawCommandType_ScissorEnd,
  UI_DrawCommandTpye_Count,
} UI_DrawCommandTypeEnum;

typedef struct UI_DrawCommand UI_DrawCommand;
struct UI_DrawCommand
{
  UI_DrawCommandType type;
  union
  {
    struct
    {
      RectF32 bound;
      Vec4F32 color;
      Vec4F32 border_color;
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

    struct
    {
      RectF32 bound;
    } scissor;
  };
};
UI_DrawCommand UI_DrawCommandDefaultValue = {0};
DefineArray(UI_DrawCommand, UI_DrawCommandArray, UI_DrawCommandDefaultValue)

typedef struct UI_Context UI_Context;
struct UI_Context
{
  // Interaction
  UI_ID hot_id;
  UI_ID active_id;
  Vec2 mouse_position;
  Vec2F32 mouse_scroll;

  UI_WidgetArray elements;

  UI_IDArray final_elements;
  UI_IDArray open_elements_stack;
  UI_IDArray clip_elements_stack;
  UI_IDArray branches;
  UI_IDArray children;
  UI_IDArray children_formation_buffer;

  UI_IDArray traversal_stack;
  B32Array visited_lookup;

  UI_ScrollOffsetArray scroll_offsets;

  UI_DrawCommandArray draw_commands;
} ui_context; // -- AlNov. 12 December 2025: @TODO Multiole contexts?

func void UI_Init(Arena* arena, U32 max_elements_count);

// -------------------------------------------------------------------
// -- UI Context Mutation --------------------------------------------
func void UI_CalculateSizes(B32 is_width);
func void UI_CalculatePositions();

func void UI_BeginFrame(Vec2 mouse_position, Vec2F32 mouse_scroll);
func void UI_EndFrame();

// -------------------------------------------------------------------
// -- UI Widgets ----------------------------------------------------
func UI_Widget* UI_GetOpenedWidget();

func UI_ID   UI_GetID()                {return UI_GetOpenedWidget()->id;}
func RectF32 UI_GetRectangle(UI_ID id) {return UI_WidgetArrayGet(&ui_context.elements, id).rect;}

func Vec2I32 UI_GetScrollOffset();

// --AlNov 20 December 2025:
// Separate Open and Configure to be able to use UI_Hovered, UI_GetRectangle, etc in UI_WidgetBlock.
func void UI_OpenWidget();
func void UI_ConfigureWidget(UI_WidgetDescription description);
func void UI_CloseWidget();

// --AlNov 16 December 2025:
// Macroses below solve next problem - warning: C99 forbids casting nonscalar type 'UI_WidgetDescription'
// With wrapper it is possilbe to use api with inline structure definition (1) and predefined structure (2).
// (1) - UI_OpenWidget(.layout.width = UI_FixedSize(100)) {}
// (2) - UI_WidgetDescription default_element = {.layout.width = UI_FixedSize(100)};
//       UI_OpenWidget(default_element) {}
#define UI_DefineWidgetDescriptionStructWrapper() typedef struct {UI_WidgetDescription package;} UI_WidgetDescriptionWrapper;
UI_DefineWidgetDescriptionStructWrapper()
#define UI_WidgetDescriptionWrapper(...) ((UI_WidgetDescriptionWrapper){__VA_ARGS__}).package
#define UI_WidgetBlock(...) DeferBlock((UI_OpenWidget(), UI_ConfigureWidget(UI_WidgetDescriptionWrapper(__VA_ARGS__))), UI_CloseWidget())

// --AlNov 20 December 2025:
// Interaction with UI is delayed by 1 frame.
// While we adding Widgets we don't know the final configuration of layout.
// The layout is in the final state after UI_EndFrame() is called.
func B32 UI_Hovered();
func B32 UI_Clicked();
func RectF32 UI_GetWidgetRectF32();

func UI_Widget* UI_Layout(UI_WidgetArray* array, Str8 label);
func void        UI_Text(Str8 label, UI_TextDescription text);
func void        UI_NumberInput(UI_WidgetArray* array, Str8 label, F32* value);
func B32         UI_Button(UI_WidgetArray* array, Str8 label);
func F32         UI_SliderF32(UI_WidgetArray* array, Str8 label, F32 min, F32 max, F32* value);
