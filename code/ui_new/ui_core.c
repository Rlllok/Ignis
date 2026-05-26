#include "ui_core.h"

// -------------------------------------------------------------------
// -- State ----------------------------------------------------------

// --AlNov: @TODO Doesn't work with threads
global_variable UI_Context* ui_current_context = 0;

func UI_Context*
UI_CreateContext() {
  Arena* arena = AllocateArena(Gigabytes(16), Kilobytes(4));
  UI_Context* context = (UI_Context*)PushArena(arena, sizeof(UI_Context));
  context->arena = arena;

  return context;
}

func void
UI_DestroyContext(UI_Context* context) {
  FreeArena(context->arena);
}

func void
UI_SelectContext(UI_Context* context) {
  ui_current_context = context;
}

// -------------------------------------------------------------------
// -- Widget ---------------------------------------------------------
func void
UI_OpenWidget() {
  UI_Widget* widget = PushArena(ui_current_context->arena, sizeof(UI_Widget));
  DllPushBack(ui_current_context->root.first, ui_current_context->root.last, widget);
  StackPush_Next(ui_current_context->opened_widget, widget, stack_next);
}

func void
UI_ConfigureWidget(UI_WidgetInfo info) {
  UI_Widget* widget = ui_current_context->opened_widget;

  widget->info = info;
}

func void
UI_CloseWidget() {
  StackPop_Next(ui_current_context->opened_widget, stack_next);
}

func void
UI_BeginFrame(F32 dt, Vec2F32 mouse_position, Vec2F32 mouse_scroll) {
  Assert(ui_current_context != 0);

  ui_current_context->dt = dt;
  ui_current_context->mouse_position = mouse_position;
  ui_current_context->mouse_scroll = mouse_scroll;
}

func void
UI_EndFrame() {
}
