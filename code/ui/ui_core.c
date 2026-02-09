#include "ui_core.h"

// -------------------------------------------------------------------
// -- UI Font --------------------------------------------------------
func Vec2F32
GetTextSize(FontBitmap font, Str8 text, U32 font_size)
{
  // --AlNov: @TODO Font spacing is hardcoded and not correct (Bitmap Grid size is 30px, but glyphs is smaller)
  Vec2F32 result = MakeVec2F32(font_size*0.5f, font_size);

  for (I32 i = 0; i < text.length; i += 1)
  {
    if (text.data[i] == '\n')
    {
      result.y += font_size*0.7f;
      continue;
    }
    result.x += font_size*0.5f;
  }

  return result;
}

func void
UI_Init(Arena* arena, U32 max_elements_count)
{
  ui_context.elements = UI_WidgetArrayAllocate(arena, max_elements_count);
  ui_context.final_elements = UI_IDArrayAllocate(arena, max_elements_count);
  ui_context.open_elements_stack = UI_IDArrayAllocate(arena, max_elements_count);
  ui_context.clip_elements_stack = UI_IDArrayAllocate(arena, max_elements_count);
  ui_context.branches = UI_IDArrayAllocate(arena, max_elements_count);
  ui_context.children = UI_IDArrayAllocate(arena, max_elements_count);
  ui_context.children_formation_buffer = UI_IDArrayAllocate(arena, max_elements_count);
  ui_context.draw_commands = UI_DrawCommandArrayAllocate(arena, max_elements_count);

  ui_context.traversal_stack = UI_IDArrayAllocate(arena, max_elements_count);
  ui_context.visited_lookup = B32ArrayAllocate(arena, max_elements_count);
  ui_context.visited_lookup.length = ui_context.visited_lookup.capacity;

  ui_context.scroll_offsets = UI_ScrollOffsetArrayAllocate(arena, 64);
}

// -------------------------------------------------------------------
// -- UI Context Mutation --------------------------------------------
func void
UI_CalculateSizes(B32 is_width)
{
  for (I32 branch_index = ui_context.branches.length - 1; branch_index >= 0; branch_index -= 1)
  {
    UI_Widget* branch = UI_WidgetArrayGetPointer(&ui_context.elements, UI_IDArrayGet(&ui_context.branches, branch_index));

    for (I32 child_offset = 0; child_offset < branch->children_array_slice.length; child_offset += 1)
    {
      UI_ID child_id = branch->children_array_slice.ids[child_offset];
      UI_Widget* child_element = UI_WidgetArrayGetPointer(&ui_context.elements, child_id);

      UI_Size child_size = (is_width) ? child_element->description.layout.width : child_element->description.layout.height;
      F32* child_size_value = (is_width) ? &child_element->rect.size.x : &child_element->rect.size.y;

      if (child_size.type == UI_SizeType_Percent)
      {
        F32 parent_size_value = (is_width) ? branch->rect.size.x : branch->rect.size.y;
        F32 parent_padding_value_0 = (is_width) ? branch->description.layout.padding.left : branch->description.layout.padding.top;
        F32 parent_padding_value_1 = (is_width) ? branch->description.layout.padding.left : branch->description.layout.padding.bottom;
        *child_size_value = (parent_size_value*child_size.value) - parent_padding_value_0 - parent_padding_value_1;
      }
    }
  }
}

func void
UI_CalculatePositions()
{
  for (I32 branch_index = 0; branch_index < ui_context.branches.length; branch_index += 1)
  // for (I32 branch_index = ui_context.branches.length - 1; branch_index >= 0; branch_index -= 1)
  {
    UI_Widget* branch = UI_WidgetArrayGetPointer(&ui_context.elements, UI_IDArrayGet(&ui_context.branches, branch_index));
    for (I32 child_offset = 0; child_offset < branch->children_array_slice.length; child_offset += 1)
    {
      UI_Widget* child_element = UI_WidgetArrayGetPointer(&ui_context.elements, branch->children_array_slice.ids[child_offset]);

      child_element->rect.position = branch->rect.position;

      child_element->rect.x += branch->description.layout.padding.left;
      child_element->rect.y += branch->description.layout.padding.top;

      switch (branch->description.layout.direction)
      {
        case UI_LayoutDirection_TopToBottom:
        {
          child_element->rect.y = branch->rect.y + branch->child_position_offset.y;
          branch->child_position_offset.y += child_element->rect.h + branch->description.layout.child_gap;
        } break;

        case UI_LayoutDirection_LeftToRight:
        {
          child_element->rect.x = branch->rect.position.x + branch->child_position_offset.x;
          branch->child_position_offset.x += child_element->rect.w + branch->description.layout.child_gap;
        } break;
      }
    }
  }
}

func void
UI_BeginFrame(Vec2F32 mouse_position, Vec2F32 mouse_scroll)
{
  ui_context.mouse_position = mouse_position;
  ui_context.mouse_scroll = mouse_scroll;
  ui_context.hot_id = 0;

  for (I32 i = 0; i < ui_context.children.length; i += 1)
  {
    UI_ID element_id = UI_IDArrayGet(&ui_context.children, i);
    UI_Widget* element = UI_WidgetArrayGetPointer(&ui_context.elements, element_id);
    
    B32 item_is_hot = element->description.type == UI_WidgetType_Rectangle
      && InsideRectF32(element->rect, ui_context.mouse_position)
      && InsideRectF32(UI_WidgetArrayGetPointer(&ui_context.elements, element->clip_element_id)->rect, ui_context.mouse_position);

    if (item_is_hot)
    {
      ui_context.hot_id = element_id;
      break;
    }
  }

  for (I32 i = 0; i < ui_context.scroll_offsets.length; i += 1)
  {
    UI_ScrollOffset* scroll_offset = UI_ScrollOffsetArrayGetPointer(&ui_context.scroll_offsets, i);
    UI_Widget* element = UI_WidgetArrayGetPointer(&ui_context.elements, scroll_offset->element_id);
    
    scroll_offset->offset = AddVec2I32(scroll_offset->offset, Vec2IFromVec2F32(mouse_scroll));
    scroll_offset->offset.y = Max(Min(0, element->rect.h - element->child_position_offset.y), Min(0, scroll_offset->offset.y));
  }

  UI_WidgetArrayReset(&ui_context.elements);
  UI_IDArrayReset(&ui_context.final_elements);
  UI_IDArrayReset(&ui_context.open_elements_stack);
  UI_IDArrayReset(&ui_context.clip_elements_stack);
  UI_IDArrayReset(&ui_context.branches);
  UI_IDArrayReset(&ui_context.children);
  UI_IDArrayReset(&ui_context.children_formation_buffer);
  UI_DrawCommandArrayReset(&ui_context.draw_commands);

  UI_IDArrayReset(&ui_context.traversal_stack);
  // --AlNov 24 December 2025: @TODO @NOTE
  // This is table that contains visited flag per every element_id.
  // It means that length should be equal capacity. And we should reset values inside.
  // The problem - I don't like usage code. I spent 20 minutes debuging why elements always visited.
  // I forgot that ArrayResetDefault(..) sets length to 0.
  B32ArrayResetDefault(&ui_context.visited_lookup);
  ui_context.visited_lookup.length = ui_context.visited_lookup.capacity;
}

func void UI_EndFrame()
{
  UI_CalculateSizes(0);
  UI_CalculateSizes(1);

  #if 0
  for (I32 branch_index = ui_context.branches.length - 1; branch_index >= 0; branch_index -= 1)
  {
    UI_Widget branch = UI_WidgetArrayGet(&ui_context.elements, UI_IDArrayGet(&ui_context.branches, branch_index));

    LOG_DEBUG("\tWidgetName: %s. WidgetID: %d. Children Count: %d\n", CFromStr8(branch.description.name), branch.id, branch.children_array_slice.length);
    for (I32 i = 0; i < branch.children_array_slice.length; i += 1)
    {
      UI_ID child_id = branch.children_array_slice.ids[i];
      UI_Widget child = UI_WidgetArrayGet(&ui_context.elements, child_id);
      LOG_DEBUG("\t\tChildName: %s, ChildID:  %d\n", CFromStr8(child.description.name), child_id);
    }
  }
  #endif

  for (I32 branch_index = ui_context.branches.length - 1; branch_index >= 0; branch_index -= 1)
  {
    UI_Widget* branch = UI_WidgetArrayGetPointer(&ui_context.elements, UI_IDArrayGet(&ui_context.branches, branch_index));

    for (I32 child_offset = 0; child_offset < branch->children_array_slice.length; child_offset += 1)
    {
      UI_Widget* child_element = UI_WidgetArrayGetPointer(&ui_context.elements, branch->children_array_slice.ids[child_offset]);

      child_element->rect.position = branch->rect.position;

      child_element->rect.x += branch->description.layout.padding.left;
      child_element->rect.y += branch->description.layout.padding.top;

      for (I32 i = 0; i < ui_context.scroll_offsets.length; i += 1)
      {
        UI_ScrollOffset* scroll_offset = UI_ScrollOffsetArrayGetPointer(&ui_context.scroll_offsets, i);
        if (scroll_offset->element_id == branch->id)
        {
          I32 offset = scroll_offset->offset.y;
          child_element->rect.y += offset;
        }
      }

      switch (branch->description.layout.direction)
      {
        case UI_LayoutDirection_TopToBottom:
        {
          child_element->rect.y += branch->child_position_offset.y;
          branch->child_position_offset.y += child_element->rect.h + branch->description.layout.child_gap;
        } break;

        case UI_LayoutDirection_LeftToRight:
        {
          child_element->rect.x += branch->child_position_offset.x;
          branch->child_position_offset.x += child_element->rect.w + branch->description.layout.child_gap;
        } break;
      }
    }
  }

  if (ui_context.children.length != 0)
  {
    UI_IDArrayAdd(&ui_context.traversal_stack, UI_IDArrayGet(&ui_context.children, ui_context.children.length - 1));
  
    while (ui_context.traversal_stack.length)
    {
      UI_ID current_id = UI_IDArrayGet(&ui_context.traversal_stack, ui_context.traversal_stack.length - 1);
      UI_Widget* current_element = UI_WidgetArrayGetPointer(&ui_context.elements, current_id);

      if (!ui_context.visited_lookup.elements[current_id])
      {
        ui_context.visited_lookup.elements[current_id] = 1;

        if (current_element->description.flags & UI_WidgetFlag_DrawBackground)
        {
          UI_DrawCommandArrayAdd(
            &ui_context.draw_commands,
            (UI_DrawCommand){
              .type = UI_DrawCommandType_Rectangle,
              .rectangle = {
                .bound = current_element->rect,
                .color = current_element->description.rectangle.color,
                .border_color = (current_element->description.rectangle.border_color.a == 0) ? current_element->description.rectangle.color : current_element->description.rectangle.border_color,
                .radius = current_element->description.rectangle.radius.values,
              }
            }
          );
        }
        if (current_element->description.flags & UI_WidgetFlag_DrawLabel)
        {
          UI_DrawCommandArrayAdd(
            &ui_context.draw_commands,
            (UI_DrawCommand){
              .type = UI_DrawCommandType_Text,
              .text = {
                .content = current_element->description.text.str,
                .font = current_element->description.text.font,
                .font_size = current_element->description.text.font_size,
                .color = current_element->description.text.color,
                .position = current_element->rect.position,
              }
            }
          );
        }
        if (current_element->description.layout.clip)
        {
          UI_DrawCommandArrayAdd(&ui_context.draw_commands, (UI_DrawCommand){
            .type = UI_DrawCommandType_ScissorBegin,
            .scissor = current_element->rect,
          });
        }

        for (I32 i = 0; i < current_element->children_array_slice.length; i += 1)
        {
          UI_IDArrayAdd(&ui_context.traversal_stack, current_element->children_array_slice.ids[i]);
        }
      }
      else
      {
        if (current_element->description.layout.clip)
        {
          UI_DrawCommandArrayAdd(&ui_context.draw_commands, (UI_DrawCommand){
            .type = UI_DrawCommandType_ScissorEnd,
          });
        }
        UI_IDArrayPop(&ui_context.traversal_stack);
      }
    }
  }
}

// -------------------------------------------------------------------
// -- UI Default Widgets --------------------------------------------
func UI_Widget*
UI_GetOpenedWidget()
{
  UI_ID opened_element_id = UI_IDArrayGet(&ui_context.open_elements_stack, ui_context.open_elements_stack.length - 1);
  return UI_WidgetArrayGetPointer(&ui_context.elements, opened_element_id);
}

func Vec2I32
UI_GetScrollOffset()
{
  Vec2I32 result = {0};
  
  UI_Widget* element = UI_GetOpenedWidget();
  for (I32 i = 0; i < ui_context.scroll_offsets.length; i += 1)
  {
    UI_ScrollOffset* scroll_offset = UI_ScrollOffsetArrayGetPointer(&ui_context.scroll_offsets, i);
    if (scroll_offset->element_id == element->id)
    {
      result = scroll_offset->offset;
      break;
    }
  }

  return result;
}

func void
UI_OpenWidget()
{
  UI_Widget element = {.id = ui_context.elements.length};

  UI_WidgetArrayAdd(&ui_context.elements, element);
  UI_IDArrayAdd(&ui_context.open_elements_stack, element.id);
}

func void
UI_ConfigureWidget(UI_WidgetDescription description)
{
  UI_Widget* element = UI_GetOpenedWidget();
  element->description = description;

  if (ui_context.clip_elements_stack.length > 0)
  {
    UI_ID clip_element_id = UI_IDArrayGet(&ui_context.clip_elements_stack, ui_context.clip_elements_stack.length - 1);
    element->clip_element_id = clip_element_id;
  }

  if (element->description.layout.width.type == UI_SizeType_Pixel)
  {
    element->rect.size.x = element->description.layout.width.value;
  }
  if (element->description.layout.height.type == UI_SizeType_Pixel)
  {
    element->rect.size.y = element->description.layout.height.value;
  }

  if (element->description.layout.clip)
  {
    UI_ScrollOffset* scroll_offset = 0;
    for (I32 i = 0; i < ui_context.scroll_offsets.length; i += 1)
    {
      UI_ScrollOffset* found_offset = UI_ScrollOffsetArrayGetPointer(&ui_context.scroll_offsets, i);
      if (found_offset->element_id == element->id)
      {
        scroll_offset = found_offset;
        break;
      }
    }

    if (!scroll_offset)
    {
      UI_ScrollOffsetArrayAdd(&ui_context.scroll_offsets, (UI_ScrollOffset){.element_id = element->id});
    }

    UI_IDArrayAdd(&ui_context.clip_elements_stack, element->id);
  }
}

func void UI_CloseWidget()
{
  UI_Widget* current_element = UI_GetOpenedWidget();

  current_element->children_array_slice.ids = ui_context.children.elements + ui_context.children.length;
  for (I32 i = 0; i < current_element->children_array_slice.length; i += 1)
  {
    I32 buffer_offset = ui_context.children_formation_buffer.length - current_element->children_array_slice.length + i;
    UI_ID child_id = UI_IDArrayGet(&ui_context.children_formation_buffer, buffer_offset);
    UI_IDArrayAdd(&ui_context.children, child_id);

    if (current_element->description.layout.height.type == UI_SizeType_FitChildren)
    {
      UI_Widget* child_element = UI_WidgetArrayGetPointer(&ui_context.elements, child_id);
      current_element->rect.size.y += child_element->rect.size.y;
    }
    if (current_element->description.layout.width.type == UI_SizeType_FitChildren)
    {
      UI_Widget* child_element = UI_WidgetArrayGetPointer(&ui_context.elements, child_id);
      current_element->rect.size.x += child_element->rect.size.x;
    }
  }
  ui_context.children_formation_buffer.length -= current_element->children_array_slice.length;

  if (current_element->children_array_slice.length)
  {
    UI_IDArrayAdd(&ui_context.branches, current_element->id);
  }

  if (ui_context.open_elements_stack.length > 0)
  {
    UI_IDArrayPop(&ui_context.open_elements_stack);
    UI_Widget* parent_element = UI_GetOpenedWidget();
    UI_IDArrayAdd(&ui_context.children_formation_buffer, current_element->id);
    parent_element->children_array_slice.length += 1;
  }

  if (current_element->description.layout.clip && ui_context.clip_elements_stack.length > 0)
  {
    UI_IDArrayPop(&ui_context.clip_elements_stack);
  }
}

func B32
UI_Hovered()
{
  UI_Widget* element = UI_GetOpenedWidget();
  return ui_context.hot_id == element->id;
}

func B32
UI_IsClicked()
{
  B32 result = 0;
  UI_Widget* current_element = UI_WidgetArrayGetPointer(&ui_context.elements, ui_context.elements.length - 1);
  
  if (ui_context.active_id == current_element->id)
  {
    if ((ui_context.hot_id == current_element->id) && OS_MouseReleased(OS_MouseButton_Left))
    {
      ui_context.active_id = 0;
      result = 1;
    }
  }
  else if (ui_context.hot_id == current_element->id)
  {
    if (OS_MousePressed(OS_MouseButton_Left)) ui_context.active_id = current_element->id;
  }

  return result;
}

func RectF32
UI_GetWidgetRectF32()
{
  return UI_GetOpenedWidget()->rect;
}

func void
UI_Text(Str8 text, UI_TextDescription text_description)
{
  // --AlNov 14 December 2025: @TODO
  // Roundtrip to set what text to draw. I am sure that there is a better way.
  UI_TextDescription with_str = text_description;
  with_str.str = text;

  Vec2F32 text_dimension = GetTextSize(text_description.font, text, text_description.font_size);

  UI_WidgetBlock({
    .name = text,
    .type = UI_WidgetType_Text,
    .flags = UI_WidgetFlag_DrawLabel,
    .text = with_str,
    .layout = {
      .width = UI_PixelSize(text_dimension.x),
      .height = UI_PixelSize(text_dimension.y),
    }
  });
}

#if 0
func UI_Widget*
UI_Layout(UI_WidgetArray* array, Str8 label)
{
  UI_Widget* layout = UI_BuildWidget(
    array,
    (UI_WidgetDescription){
      .flags = UI_WidgetFlag_DrawBackground,
    }
  );
  return layout;
}

func void
UI_NumberInput(UI_WidgetArray* array, Str8 label, F32* value)
{
  F32 result = *value;

  UI_Widget* input = UI_BuildWidget(
    array,
    (UI_WidgetDescription){
      .flags = UI_WidgetFlag_Hover | UI_WidgetFlag_DrawLabel | UI_WidgetFlag_DrawBackground,
    }
  );

  if (ui_context.active_id == input->id)
  {
    if (OS_IsKeyPressed(OS_KEY_0))
    {
      result = result*10;
    }
    else if (OS_IsKeyPressed(OS_KEY_1))
    {
      result = result*10 + 1;
    }
    else if (OS_IsKeyPressed(OS_KEY_2))
    {
      result = result*10 + 2;
    }
    else if (OS_IsKeyPressed(OS_KEY_3))
    {
      result = result*10 + 3;
    }
    else if (OS_IsKeyPressed(OS_KEY_4))
    {
      result = result*10 + 4;
    }
    else if (OS_IsKeyPressed(OS_KEY_5))
    {
      result = result*10 + 5;
    }
    else if (OS_IsKeyPressed(OS_KEY_6))
    {
      result = result*10 + 6;
    }
    else if (OS_IsKeyPressed(OS_KEY_7))
    {
      result = result*10 + 7;
    }
    else if (OS_IsKeyPressed(OS_KEY_8))
    {
      result = result*10 + 8;
    }
    else if (OS_IsKeyPressed(OS_KEY_9))
    {
      result = result*10 + 9;
    }
    else if (OS_IsKeyPressed(OS_KEY_BACKSPACE))
    {
      result = (F32)((I32)(result/10));
    }
    else if (OS_IsKeyPressed(OS_KEY_RETURN))
    {
      ui_context.active_id = 0;
    }
  }
  else if (ui_context.hot_id == input->id)
  {
    if (OS_IsMousePressed(OS_MouseButton_Left)) ui_context.active_id = input->id;
  }

  *value = result;
}

func B32
UI_Button(UI_WidgetArray* array, Str8 label)
{
  B32 result = 0;

  UI_Widget* button = UI_BuildWidget(
    array,
    (UI_WidgetDescription){
      .flags = UI_WidgetFlag_Hover | UI_WidgetFlag_DrawLabel | UI_WidgetFlag_DrawBackground,
      .rectangle.radius = {.top_right = 10.0f, .bottom_right = 10.0f},
    }
  );

  if (ui_context.active_id == button->id)
  {
    if ((ui_context.hot_id == button->id) && OS_IsMouseReleased(OS_MouseButton_Left))
    {
      ui_context.active_id = 0;
      result = 1;
    }
  }
  else if (ui_context.hot_id == button->id)
  {
    if (OS_IsMousePressed(OS_MouseButton_Left)) ui_context.active_id = button->id;
  }

  return result;
}

func F32
UI_SliderF32(UI_WidgetArray* array, Str8 label, F32 min, F32 max, F32* value)
{
  UI_Widget* slider = UI_BuildWidget(
    array,
    (UI_WidgetDescription){
      .flags = UI_WidgetFlag_Hover|
        UI_WidgetFlag_DrawLabel|
        UI_WidgetFlag_DrawBackground,
    }
  );

  F32 slider_value = (*value - min)/(max-min);

  UI_DrawCommandArrayAdd(
    &ui_context.draw_commands,
    (UI_DrawCommand){
      .type = UI_DrawCommandType_Rectangle,
      .rectangle = {
        .color = MakeVec4(0.4f, 0.4f, 0.4f, 0.5f),
        .bound = {
          .position = slider->rect.position,
          .size.x = slider->rect.size.x*slider_value,
          .size.y = slider->rect.size.y,
        },
      },
    }
  );

  if (ui_context.active_id == slider->id)
  {
    if (OS_IsMouseDown(OS_MouseButton_Left))
    {
      F32 slider_value = (ui_context.mouse_position.x - slider->rect.position.x)/slider->rect.size.x;
      slider_value = Clamp(slider_value, 0.0f, 1.0f);
      *value = min*(1.0f - slider_value) + max*slider_value;
    }
    else
    {
      ui_context.active_id = 0;
    }
  }
  else if (ui_context.hot_id == slider->id)
  {
    if (OS_IsMousePressed(OS_MouseButton_Left))
    {
      ui_context.active_id = slider->id;
    }
  }

  return 0.0f;
}
#endif
