#include "ui_core.h"

global_variable UI_Context* ui_current_context = 0;

// -------------------------------------------------------------------
// -- Widget ---------------------------------------------------------
func void
UI_OpenWidget() {
  // --AlNov: @TODO Widget should be added to main arena of the context and managed with free list
  UI_Widget* widget = PushArena(ui_current_context->frame_arena, sizeof(UI_Widget));
  MemoryZeroStruct(widget);
  if (ui_current_context->opened_widget == 0) {
    DllPushBack_NextPrev(ui_current_context->root.first, ui_current_context->root.last, widget, root_next, root_prev);
  }
  else {
    widget->parent = ui_current_context->opened_widget;
    DllPushBack(ui_current_context->opened_widget->first, ui_current_context->opened_widget->last, widget);
  }
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

  ResetArena(ui_current_context->frame_arena);

  ui_current_context->root.first = 0;
  ui_current_context->root.last = 0;

  ui_current_context->dt = dt;
  ui_current_context->mouse_position = mouse_position;
  ui_current_context->mouse_scroll = mouse_scroll;

  ui_current_context->first_draw_command = 0;
  ui_current_context->last_draw_command = 0;
}

func UI_DrawCommand*
UI_EndFrame() {
  UI_CalculateIndependentSizes(ui_current_context->root.first, UI_Axis_X);
  UI_CalculateIndependentSizes(ui_current_context->root.first, UI_Axis_Y);
  UI_CalculateParentDependentSizes(ui_current_context->root.first, UI_Axis_X);
  UI_CalculateParentDependentSizes(ui_current_context->root.first, UI_Axis_Y);
  UI_CalculatePositions(ui_current_context->root.first, UI_Axis_X);
  UI_CalculatePositions(ui_current_context->root.first, UI_Axis_Y);

  UI_BuildDrawCommands(ui_current_context->root.first);

  return ui_current_context->first_draw_command;
}

// -------------------------------------------------------------------
// -- Draw Command ---------------------------------------------------

// -------------------------------------------------------------------
// -- State ----------------------------------------------------------
// --AlNov: @TODO Doesn't work with threads
func UI_Context*
UI_CreateContext() {
  Arena* arena = AllocateArena(Gigabytes(16), Kilobytes(4));
  UI_Context* context = (UI_Context*)PushArena(arena, sizeof(UI_Context));
  context->arena = arena;
  context->frame_arena = AllocateArena(Gigabytes(8), Kilobytes(4));

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

// -- Passes
func void
UI_CalculateIndependentSizes(UI_Widget* root, UI_Axis axis) {
  switch (root->info.layout.sizes[axis].kind) {
    default: break;
    case UI_SizeKind_Pixel: {
      root->bounding_box.size.values[axis] = root->info.layout.sizes[axis].value;
    } break;
  }
  
  for (UI_Widget* child = root->first; child != 0; child = child->next) {
    UI_CalculateIndependentSizes(child, axis);
  }
}

func void
UI_CalculateParentDependentSizes(UI_Widget* root, UI_Axis axis) {
  for (UI_Widget* child = root->first; child != 0; child = child->next) {
    switch (child->info.layout.sizes[axis].kind) {
      default: break;
      case UI_SizeKind_Percent: {
        child->bounding_box.size.values[axis] = root->bounding_box.size.values[axis] * child->info.layout.sizes[axis].value;
      } break;
    }
  }
}

func void 
UI_CalculatePositions(UI_Widget* root, UI_Axis axis) {
  if (root->info.layout.direction == axis) {
    F32 offset = root->bounding_box.position.values[axis];
    for (UI_Widget* child = root->first; child != 0; child = child->next) {
      child->bounding_box.position.values[axis] = offset;
      offset += (child->bounding_box.size.values[axis] + root->info.layout.child_gap);
    }
  }

  for (UI_Widget* child = root->first; child != 0; child = child->next) {
    UI_CalculatePositions(child, axis);
  }
}

func void
UI_BuildDrawCommands(UI_Widget* root) {
  if (root->info.flags & UI_WidgetFlag_DrawBackground) {
    UI_DrawCommand* draw_command = PushArena(ui_current_context->frame_arena, sizeof(UI_DrawCommand));
    draw_command->kind = UI_DrawCommandKind_Rectangle;
    draw_command->rectangle.bounding_box = root->bounding_box;
    draw_command->rectangle.background_color = root->info.style.background_color;
    SllPushBack(ui_current_context->first_draw_command,  ui_current_context->last_draw_command, draw_command);
  }

  for (UI_Widget* child = root->first; child != 0; child = child->next) {
    UI_BuildDrawCommands(child);
  }

}
