#pragma once

#include "base/base_include.h"

// -------------------------------------------------------------------
// -- Widget ---------------------------------------------------------
typedef U8 UI_SizeKind;
enum {
  UI_SizeKind_Nil,
  UI_SizeKind_Pixel,
  UI_SizeKind_Count
} UI_SizeKindEnum;

typedef struct UI_Size UI_Size;
struct UI_Size {
  UI_SizeKind kind;
  F32         value;
};

#define UI_PixelSize(v) ((UI_Size){.kind = UI_SizeKind_Pixel, .value = v})

typedef U32 UI_WidgetFlag;
enum {
  UI_WidgetFlag_None = (1<<0),

  UI_WidgetFlag_DrawBackground = (1<<1),
} UI_WidgetFlagEnum;

typedef struct UI_WidgetStyleInfo UI_WidgetStyleInfo;
struct UI_WidgetStyleInfo {
  Vec4F32 background_color;
};

typedef struct UI_WidgetInfo UI_WidgetInfo;
struct UI_WidgetInfo {
  UI_WidgetFlag flags;
  UI_Size       width;
  UI_Size       height;

  UI_WidgetStyleInfo style_info;
};

typedef struct UI_Widget UI_Widget;
struct UI_Widget {
  UI_Widget* first;
  UI_Widget* last;
  UI_Widget* next;
  UI_Widget* prev;
  UI_Widget* parent;

  UI_Widget* stack_next;

  UI_WidgetInfo info;
};

func void UI_OpenWidget();
func void UI_ConfigureWidget(UI_WidgetInfo info);
func void UI_CloseWidget();

#define UI_DefineWidgetInfoStructWrapper() typedef struct {UI_WidgetInfo package;} UI_WidgetInfoWrapper;
UI_DefineWidgetInfoStructWrapper()
#define UI_WidgetInfoWrapper(...) ((UI_WidgetInfoWrapper){__VA_ARGS__}).package
#define UI_WidgetBlock(...) DeferBlock((UI_OpenWidget(), UI_ConfigureWidget(UI_WidgetInfoWrapper(__VA_ARGS__))), UI_CloseWidget())

// -------------------------------------------------------------------
// -- State ----------------------------------------------------------
typedef struct UI_Root UI_Root;
struct UI_Root {
  UI_Widget* first;
  UI_Widget* last;
};

typedef struct UI_Context UI_Context;
struct UI_Context {
  Arena* arena;

  UI_Root    root;
  UI_Widget* opened_widget;

  F32     dt;
  Vec2F32 mouse_position;
  Vec2F32 mouse_scroll;
};

func UI_Context* UI_CreateContext();
func void UI_DestroyContext(UI_Context* context);
func void UI_SelectContext(UI_Context* context);

func void UI_BeginFrame(F32 dt, Vec2F32 mouse_position, Vec2F32 mouse_scroll);
func void UI_EndFrame();
