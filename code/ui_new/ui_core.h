#pragma once
#include "base/base_include.h"

// -------------------------------------------------------------------
// -- Widget ---------------------------------------------------------
typedef struct UI_Key UI_Key;
struct UI_Key {
  U64 value;
};

func UI_Key UI_ZeroKey();
func UI_Key UI_KeyFromStr8(Str8 str, UI_Key seed);
func B32 UI_KeyEqual(UI_Key a, UI_Key b);

typedef U8 UI_SizeKind;
enum {
  UI_SizeKind_Nil,
  UI_SizeKind_Pixel,
  UI_SizeKind_Percent,
  UI_SizeKind_Fit,
  UI_SizeKind_Fill,
  UI_SizeKind_Count
} UI_SizeKindEnum;

typedef struct UI_Size UI_Size;
struct UI_Size {
  UI_SizeKind kind;
  F32         value;
};

#define UI_PixelSize(v)   ((UI_Size){.kind = UI_SizeKind_Pixel,   .value = v})
#define UI_PercentSize(v) ((UI_Size){.kind = UI_SizeKind_Percent, .value = v})
#define UI_FitSize()      ((UI_Size){.kind = UI_SizeKind_Fit})
#define UI_FillSize()     ((UI_Size){.kind = UI_SizeKind_Fill})

typedef enum {
  UI_Axis_Nil = -1,
  UI_Axis_X,
  UI_Axis_Y,
  UI_Axis_Count
} UI_Axis;

typedef U32 UI_WidgetFlag;
enum {
  UI_WidgetFlag_None = (1<<0),

  // Interaction
  UI_WidgetFlag_MouseInteraction = (1<<1),

  // Drawing
  UI_WidgetFlag_DrawBackground = (1<<2),
  UI_WidgetFlag_DrawText       = (1<<3),
} UI_WidgetFlagEnum;

typedef struct UI_WidgetLayoutInfo UI_WidgetLayoutInfo;
struct UI_WidgetLayoutInfo {
  union {
    UI_Size sizes[2];

    struct {
      UI_Size width;
      UI_Size height;
    };
  };
  UI_Axis direction;
  Vec4F32 paddings;
  F32     child_gap;
};

#define UI_PaddingAll(v) MakeVec4F32(v, v, v, v)

typedef struct UI_WidgetStyleInfo UI_WidgetStyleInfo;
struct UI_WidgetStyleInfo {
  Vec4F32 radius;
  Vec4F32 background_color;
  F32     border_width;
  Vec4F32 border_color;
};

typedef U8 UI_TextAlignment;
enum {
  UI_TextAlignment_Left,
  UI_TextAlignment_Right,
  UI_TextAlignment_Center,
  UI_TextAlignment_Count
} UI_TextAlignmentEnum;

typedef struct UI_TextStyleInfo UI_TextStyleInfo;
struct UI_TextStyleInfo {
  AST_Font*        font;
  UI_TextAlignment alignment;
  Vec4F32          color;

  Str8 str; // --AlNov: @TODO Move it to separate struct (Maybe text should be another kind of widget)
};

typedef struct UI_WidgetInfo UI_WidgetInfo;
struct UI_WidgetInfo {
  UI_WidgetFlag flags;

  UI_WidgetLayoutInfo layout;
  UI_WidgetStyleInfo  style;
  UI_TextStyleInfo    text;
};

typedef struct UI_Widget UI_Widget;
struct UI_Widget {
  UI_Widget* first;
  UI_Widget* last;
  UI_Widget* next;
  UI_Widget* prev;
  UI_Widget* parent;

  UI_Key     key;
  UI_Widget* hash_next;
  UI_Widget* hash_prev;

  U64 last_build_index;

  UI_Widget* stack_next;
  UI_Widget* free_next;
  UI_Widget* root_next;
  UI_Widget* root_prev;

  UI_WidgetInfo info;

  RectF32 bounding_box;
  Vec2F32 empty_size;
  I32     growable_children_count[UI_Axis_Count];
};

func UI_Widget* UI_WidgetFromKey(UI_Key key);

func void UI_OpenWidget(Str8 label);
func void UI_ConfigureWidget(UI_WidgetInfo info);
func void UI_CloseWidget();

#define UI_DefineWidgetInfoStructWrapper() typedef struct {UI_WidgetInfo package;} UI_WidgetInfoWrapper;
UI_DefineWidgetInfoStructWrapper()
#define UI_WidgetInfoWrapper(...) ((UI_WidgetInfoWrapper){__VA_ARGS__}).package
#define UI_WidgetBlock(label, ...) DeferBlock((UI_OpenWidget(label), UI_ConfigureWidget(UI_WidgetInfoWrapper(__VA_ARGS__))), UI_CloseWidget())

// -------------------------------------------------------------------
// -- Draw Command ---------------------------------------------------
typedef U8 UI_DrawCommandKind;
enum {
  UI_DrawCommandKind_Nil,
  UI_DrawCommandKind_Rectangle,
  UI_DrawCommandKind_Text,
  UI_DrawCommandKind_Count,
} UI_DrawCommandKindEnum;

typedef struct UI_DrawCommand UI_DrawCommand;
struct UI_DrawCommand {
  UI_DrawCommand* next;

  UI_DrawCommandKind kind;
  union {
    struct {
      RectF32 bounding_box;
      Vec4F32 radius;
      F32     border_width;
      Vec4F32 background_color;
      Vec4F32 border_color;
    } rectangle;
    struct {
      AST_Font* font;
      Str8      str;
      Vec2F32   position;
      F32       size;
      Vec4F32   color;
    } text;
  };
};

// -------------------------------------------------------------------
// -- State ----------------------------------------------------------
typedef struct UI_HashSlot UI_HashSlot;
struct UI_HashSlot {
  UI_Widget* first;
  UI_Widget* last;
};

typedef struct UI_Root UI_Root;
struct UI_Root {
  UI_Widget* first;
  UI_Widget* last;
};

typedef struct UI_Context UI_Context;
struct UI_Context {
  Arena* arena;

  Arena* frame_arena;

  // Caching
  U64          hash_table_length;
  UI_HashSlot* hash_table;
  U64          build_index;
  UI_Widget*   free_widgets;

  UI_Root    root;
  UI_Widget* canvas;
  UI_Widget* opened_widget;

  F32     dt;
  Vec2F32 mouse_position;
  Vec2F32 mouse_scroll;

  UI_Key hot_key;
  UI_Key next_hot_key;
  UI_Key active_key;

  UI_DrawCommand* first_draw_command;
  UI_DrawCommand* last_draw_command;
};

func UI_Context* UI_CreateContext();
func void UI_DestroyContext(UI_Context* context);
func void UI_SelectContext(UI_Context* context);

func void UI_BeginFrame(F32 dt, Vec2F32 mouse_position, Vec2F32 mouse_scroll);
func UI_DrawCommand* UI_EndFrame();

// -- Passes
func void UI_CalculateIndependentSizes(UI_Widget* root, UI_Axis axis);
func void UI_CalculateParentDependentSizes(UI_Widget* root, UI_Axis axis);
func void UI_CalculateChildDependentSizes(UI_Widget* root, UI_Axis axis);
func void UI_CalculatePositions(UI_Widget* root, UI_Axis axis);
func void UI_FinalPass(UI_Widget* root);

// -- Context/Widget Information
func Vec2F32 UI_GetMousePosition();
func RectF32 UI_GetBoundingBox();
  
// -- Interaction
func B32  UI_IsHot();
func void UI_SetActive();
func void UI_UnsetActive();
func B32  UI_IsActive();
