#include "ui_core.h"

global_variable UI_Context* ui_current_context = 0;

// -------------------------------------------------------------------
// -- Widget ---------------------------------------------------------

func UI_Key
UI_ZeroKey() {
  UI_Key result = ZeroStruct();
  return result;
}

func UI_Key
UI_KeyFromStr8(Str8 str) {
  // --AlNov: @TODO stupid
  UI_Key result = UI_ZeroKey();
  result.value = (str.length*3 + str.data[0]*11 + str.data[str.length - 1])/37;
  return result;
}

func UI_Widget*
UI_WidgetFromStr8(Str8 str) {
  UI_Widget* result = 0;
  
  U64 slot_index = UI_KeyFromStr8(str).value%ui_current_context->hash_table_length;
  for (UI_Widget* widget = ui_current_context->hash_table[slot_index].first; widget != 0; widget = widget->hash_next) {
    if (Str8Equal(str, widget->info.label)) {
      result = widget;
      break;
    }
  }

  return result;
}

func void
UI_OpenWidget(UI_WidgetInfo info) {
  // --AlNov: @TODO Widget should be added to main arena of the context and managed with free list
  UI_Widget* widget = UI_WidgetFromStr8(info.label);
  if (widget == 0) {
    // create widget
    if (ui_current_context->free_widgets) {
      widget = ui_current_context->free_widgets;
      MemoryZeroStruct(widget);
      StackPop_Next(ui_current_context->free_widgets, free_next);
    } else {
      widget = PushArena(ui_current_context->arena, sizeof(UI_Widget));
      MemoryZeroStruct(widget);
      widget->info = info;
    }
    // add to hash table
    // --AlNov: @TODO Computing key again (UI_WidgetFromStr8() called before)
    UI_Key key = UI_KeyFromStr8(info.label);
    widget->key = key;
    U64 slot_index = key.value%ui_current_context->hash_table_length;
    UI_HashSlot* slot = ui_current_context->hash_table + slot_index;
    DllPushBack_NextPrev(slot->first, slot->last, widget, hash_next, hash_prev);

    if (ui_current_context->opened_widget == 0) {
      DllPushBack_NextPrev(ui_current_context->root.first, ui_current_context->root.last, widget, root_next, root_prev);
    }
    else {
      widget->parent = ui_current_context->opened_widget;
      DllPushBack(ui_current_context->opened_widget->first, ui_current_context->opened_widget->last, widget);
    }
  }
  widget->last_build_index = ui_current_context->build_index;
  StackPush_Next(ui_current_context->opened_widget, widget, stack_next);
}

func void
UI_CloseWidget() {
  StackPop_Next(ui_current_context->opened_widget, stack_next);
}

func void
UI_BeginFrame(F32 dt, Vec2F32 mouse_position, Vec2F32 mouse_scroll) {
  Assert(ui_current_context != 0);

  ResetArena(ui_current_context->frame_arena);

  ui_current_context->build_index += 1;

  // ui_current_context->root.first = 0;
  // ui_current_context->root.last = 0;

  ui_current_context->dt = dt;
  ui_current_context->mouse_position = mouse_position;
  ui_current_context->mouse_scroll = mouse_scroll;

  // Interaction reset
  // ui_current_context->hot_widget = 0;

  // Drawing reset
  ui_current_context->first_draw_command = 0;
  ui_current_context->last_draw_command = 0;
}

func UI_DrawCommand*
UI_EndFrame() {
  // remove untoched widgets
  for (U64 slot_index = 0; slot_index < ui_current_context->hash_table_length; slot_index += 1) {
    UI_HashSlot* slot = ui_current_context->hash_table + slot_index;
    for (UI_Widget* widget = slot->first; widget != 0; widget = widget->next) {
      if (widget->last_build_index < ui_current_context->build_index) {
        MemoryZeroStruct(widget);
        if (widget->parent == 0) {
          // --AlNov: @TODO It is better to change UI_Root concept to UI_Widget
          DllRemove_NextPrev(ui_current_context->root.first, ui_current_context->root.last, widget, root_next, root_prev);
        } else {
          DllRemove(widget->parent->first, widget->parent->last, widget);
        }
        // remove from hash table
        DllRemove_NextPrev(slot->first, slot->last, widget, hash_next, hash_prev);
        // add to free list
        StackPush_Next(ui_current_context->free_widgets, widget, free_next);
      }
    }
  }

  UI_CalculateIndependentSizes(ui_current_context->root.first, UI_Axis_X);
  UI_CalculateIndependentSizes(ui_current_context->root.first, UI_Axis_Y);
  UI_CalculateParentDependentSizes(ui_current_context->root.first, UI_Axis_X);
  UI_CalculateParentDependentSizes(ui_current_context->root.first, UI_Axis_Y);
  UI_CalculateChildDependentSizes(ui_current_context->root.first, UI_Axis_X);
  UI_CalculateChildDependentSizes(ui_current_context->root.first, UI_Axis_Y);
  UI_CalculatePositions(ui_current_context->root.first, UI_Axis_X);
  UI_CalculatePositions(ui_current_context->root.first, UI_Axis_Y);

  UI_FinalPass(ui_current_context->root.first);

  return ui_current_context->first_draw_command;
}

// -------------------------------------------------------------------
// -- State ----------------------------------------------------------

// --AlNov: @TODO Doesn't work with threads
func UI_Context*
UI_CreateContext() {
  Arena* arena = AllocateArena(Gigabytes(16), Kilobytes(4));
  UI_Context* context = (UI_Context*)PushArena(arena, sizeof(UI_Context));
  context->arena = arena;
  context->frame_arena = AllocateArena(Gigabytes(8), Kilobytes(4));
  context->hash_table_length = 2048;
  context->hash_table = PushArena(context->arena, sizeof(UI_HashSlot)*context->hash_table_length);

  return context;
}

func void
UI_DestroyContext(UI_Context* context) {
  FreeArena(context->arena);
  FreeArena(context->frame_arena);
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
        F32 padding_0 = root->info.layout.paddings.values[axis*2];
        F32 padding_1 = root->info.layout.paddings.values[axis*2 + 1];
        child->bounding_box.size.values[axis] = root->bounding_box.size.values[axis]*child->info.layout.sizes[axis].value - padding_0 - padding_1;
      } break;
    }
  }

  for (UI_Widget* child = root->first; child != 0; child = child->next) {
    UI_CalculateParentDependentSizes(child, axis);
  }
}

func void
UI_CalculateChildDependentSizes(UI_Widget* root, UI_Axis axis) {
  for (UI_Widget* child = root->first; child != 0; child = child->next) {
    UI_CalculateChildDependentSizes(child, axis);
  }

  switch (root->info.layout.sizes[axis].kind) {
    default: break;
    case UI_SizeKind_Fit: {
      F32 padding_0 = root->info.layout.paddings.values[axis*2];
      F32 padding_1 = root->info.layout.paddings.values[axis*2 + 1];
      F32 child_gap = root->info.layout.child_gap;
      root->bounding_box.size.values[axis] = padding_0 + padding_1 + child_gap;
      for (UI_Widget* child = root->first; child != 0; child = child->next) {
        root->bounding_box.size.values[axis] += child->bounding_box.size.values[axis];
      }
    } break;
  }
}

func void 
UI_CalculatePositions(UI_Widget* root, UI_Axis axis) {
  F32 offset = root->bounding_box.position.values[axis];
  F32 padding = root->info.layout.paddings.values[axis*2];
  for (UI_Widget* child = root->first; child != 0; child = child->next) {
    child->bounding_box.position.values[axis] = offset + padding;
    if (root->info.layout.direction == axis) {
      offset += (child->bounding_box.size.values[axis] + root->info.layout.child_gap);
    }
  }

  for (UI_Widget* child = root->first; child != 0; child = child->next) {
    UI_CalculatePositions(child, axis);
  }
}

func void
UI_FinalPass(UI_Widget* root) {
// Interaction
  if (root->info.flags & UI_WidgetFlag_MouseInteraction) {
    B32 mouse_inside = InsideRectF32(root->bounding_box, ui_current_context->mouse_position);
    if (mouse_inside) {
      ui_current_context->hot_widget = root;
    }
  }

// Build draw commands
  if (root->info.flags & UI_WidgetFlag_DrawBackground) {
    UI_DrawCommand* draw_command = PushArena(ui_current_context->frame_arena, sizeof(UI_DrawCommand));
    draw_command->kind = UI_DrawCommandKind_Rectangle;
    draw_command->rectangle.bounding_box = root->bounding_box;
    draw_command->rectangle.background_color = root->info.style.background_color;
    SllPushBack(ui_current_context->first_draw_command, ui_current_context->last_draw_command, draw_command);
  }

  if (root->info.flags & UI_WidgetFlag_DrawText) {
    Assert(root->info.text.font != 0);

    UI_DrawCommand* draw_command = PushArena(ui_current_context->frame_arena, sizeof(UI_DrawCommand));
    draw_command->kind = UI_DrawCommandKind_Text;
    draw_command->text.font = root->info.text.font;
    draw_command->text.str = root->info.label;
    switch (root->info.text.alignment) {
      default: break;
      case UI_TextAlignment_Left: {
        draw_command->text.position = root->bounding_box.position;
        draw_command->text.position.y += root->bounding_box.size.y;
      } break;
      case UI_TextAlignment_Right: {
        Vec2F32 text_size = AST_TextSize(root->info.label, root->info.text.font);
        F32 x_right = root->bounding_box.position.x + root->bounding_box.size.x;
        draw_command->text.position.x = x_right - text_size.x;
        draw_command->text.position.y = root->bounding_box.position.y + root->bounding_box.size.y;
      } break;
      case UI_TextAlignment_Center: {
        Vec2F32 text_size = AST_TextSize(root->info.label, root->info.text.font);
        Vec2F32 root_center = AddVec2F32(root->bounding_box.position, ScaleVec2F32(root->bounding_box.size, 0.5f));
        draw_command->text.position.x = root_center.x - text_size.x*0.5f;
        draw_command->text.position.y = root->bounding_box.position.y + root->bounding_box.size.y;
      };
    }
    draw_command->text.size = 0; // --AlNov: @TODO Do nothing for now
    draw_command->text.color = root->info.text.color;
    SllPushBack(ui_current_context->first_draw_command, ui_current_context->last_draw_command, draw_command);
  }

  for (UI_Widget* child = root->first; child != 0; child = child->next) {
    UI_FinalPass(child);
  }
}

// -- Interaction
func B32
UI_IsHot() {
  return ui_current_context->opened_widget == ui_current_context->hot_widget;
}
