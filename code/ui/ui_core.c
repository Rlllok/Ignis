#include "ui_core.h"

// -------------------------------------------------------------------
// -- UI Font --------------------------------------------------------
func Vec2
GetTextSize(FontBitmap font, Str8 text, U32 font_size)
{
  // --AlNov: @TODO Font spacing is hardcoded and not correct (Bitmap Grid size is 30px, but glyphs is smaller)
  Vec2 result = MakeVec2(font_size*0.5f, font_size);

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
  ui_context.elements = UI_ElementArrayAllocate(arena, max_elements_count);
  ui_context.open_elements_stack = UI_IDArrayAllocate(arena, max_elements_count);
  ui_context.branches = UI_IDArrayAllocate(arena, max_elements_count);
  ui_context.children = UI_IDArrayAllocate(arena, max_elements_count);
  ui_context.children_formation_buffer = UI_IDArrayAllocate(arena, max_elements_count);
  ui_context.draw_commands = UI_DrawCommandArrayAllocate(arena, max_elements_count);
}

// -------------------------------------------------------------------
// -- UI Context Mutation --------------------------------------------
func void
UI_CalculateSizes(B32 is_width)
{
  for (I32 branch_index = ui_context.branches.length - 1; branch_index >= 0; branch_index -= 1)
  {
    UI_Element branch = UI_ElementArrayGet(&ui_context.elements, UI_IDArrayGet(&ui_context.branches, branch_index));

    for (I32 child_offset = 0; child_offset < branch.children_array_slice.length; child_offset += 1)
    {
      UI_ID child_id = branch.children_array_slice.ids[child_offset];
      UI_Element* child_element = UI_ElementArrayGetPointer(&ui_context.elements, child_id);

      UI_Size child_size = (is_width) ? child_element->description.layout.width : child_element->description.layout.height;
      F32* child_size_value = (is_width) ? &child_element->rect.size.x : &child_element->rect.size.y;

      if (child_size.type == UI_SizeType_Percent)
      {
        F32 parent_size_value = (is_width) ? branch.rect.size.x : branch.rect.size.y;
        F32 parent_padding_value_0 = (is_width) ? branch.description.layout.padding.left : branch.description.layout.padding.top;
        F32 parent_padding_value_1 = (is_width) ? branch.description.layout.padding.left : branch.description.layout.padding.bottom;
        *child_size_value = (parent_size_value*child_size.value) - parent_padding_value_0 - parent_padding_value_1;
      }
    }
  }
}

func void
UI_CalculatePositions()
{
  // for (I32 branch_index = 0; branch_index < ui_context.branches.length; branch_index += 1)
  for (I32 branch_index = ui_context.branches.length - 1; branch_index >= 0; branch_index -= 1)
  {
    UI_Element* branch = UI_ElementArrayGetPointer(&ui_context.elements, UI_IDArrayGet(&ui_context.branches, branch_index));
    LOG_DEBUG("BranchName: %s\n", branch->description.name);

    for (I32 child_offset = 0; child_offset < branch->children_array_slice.length; child_offset += 1)
    {
      UI_Element* child_element = UI_ElementArrayGetPointer(&ui_context.elements, branch->children_array_slice.ids[child_offset]);

      child_element->rect.position = branch->rect.position;

      child_element->rect.x += branch->description.layout.padding.left;
      child_element->rect.y += branch->description.layout.padding.top;

      switch (branch->description.layout.direction)
      {
        case UI_LayoutDirection_TopToBottom:
        {
          child_element->rect.y = branch->rect.y + branch->child_position_offset.y;
          branch->child_position_offset.y += child_element->rect.h;
        } break;

        case UI_LayoutDirection_LeftToRight:
        {
          child_element->rect.x = branch->rect.position.x + branch->child_position_offset.x;
          branch->child_position_offset.x += child_element->rect.w;
        } break;
      }
    }
  }
}

func void
UI_BeginFrame(Vec2 mouse_position)
{
  ui_context.mouse_position = mouse_position;
  ui_context.hot_id = 0;

  for (I32 i = 0; i < ui_context.children.length; i += 1)
  {
    UI_ID element_id = UI_IDArrayGet(&ui_context.children, i);
    UI_Element* element = UI_ElementArrayGetPointer(&ui_context.elements, element_id);
    if (element->description.type == UI_ElementType_Rectangle && InsideRectF32(element->rect, ui_context.mouse_position))
    {
      ui_context.hot_id = element_id;
      break;
    }
  }

  UI_ElementArrayReset(&ui_context.elements);
  UI_IDArrayReset(&ui_context.open_elements_stack);
  UI_IDArrayReset(&ui_context.branches);
  UI_IDArrayReset(&ui_context.children);
  UI_IDArrayReset(&ui_context.children_formation_buffer);
  UI_DrawCommandArrayReset(&ui_context.draw_commands);
}

func void UI_EndFrame()
{
  UI_CalculateSizes(0);
  UI_CalculateSizes(1);

  // --AlNov 17 December 2025:
  // Calculate positions.
  for (I32 branch_index = ui_context.branches.length - 1; branch_index >= 0; branch_index -= 1)
  {
    UI_Element branch = UI_ElementArrayGet(&ui_context.elements, UI_IDArrayGet(&ui_context.branches, branch_index));

    LOG_DEBUG("\tElementName: %s. ElementID: %d. Children Count: %d\n", CFromStr8(branch.description.name), branch.id, branch.children_array_slice.length);
    for (I32 i = 0; i < branch.children_array_slice.length; i += 1)
    {
      UI_ID child_id = branch.children_array_slice.ids[i];
      UI_Element child = UI_ElementArrayGet(&ui_context.elements, child_id);
      LOG_DEBUG("\t\tChildName: %s, ChildID:  %d\n", CFromStr8(child.description.name), child_id);
    }
  }

  UI_CalculatePositions();

  for (I32 element_index = 0; element_index < ui_context.elements.length; element_index += 1)
  {
    UI_Element element = UI_ElementArrayGet(&ui_context.elements, element_index);

    if (element.description.flags & UI_ElementFlag_Hover)
    {
      if (InsideRectF32(element.rect, ui_context.mouse_position))
      {
        ui_context.hot_id = element.id;
        element.description.rectangle.color = AddVec4(element.description.rectangle.color, MakeVec4(0.2f, 0.2f, 0.2f, 0.0f));
      }
    }
    if (element.description.flags & UI_ElementFlag_Clickable)
    {
    }
    if (element.description.flags & UI_ElementFlag_DrawBackground)
    {
      UI_DrawCommandArrayAdd(
        &ui_context.draw_commands,
        (UI_DrawCommand){
          .type = UI_DrawCommandType_Rectangle,
          .rectangle = {
            .bound = element.rect,
            .color = element.description.rectangle.color,
            .border_color = element.description.rectangle.border_color,
            .radius = element.description.rectangle.radius.values,
          }
        }
      );
    }
    if (element.description.flags & UI_ElementFlag_DrawLabel)
    {
      UI_DrawCommandArrayAdd(
        &ui_context.draw_commands,
        (UI_DrawCommand){
          .type = UI_DrawCommandType_Text,
          .text = {
            .content = element.description.text.str,
            .font = element.description.text.font,
            .font_size = element.description.text.font_size,
            .color = element.description.text.color,
            .position = element.rect.position,
          }
        }
      );
    }
  }
}

// -------------------------------------------------------------------
// -- UI Default Elements --------------------------------------------
func UI_Element*
UI_GetOpenedElement()
{
  UI_ID opened_element_id = UI_IDArrayGet(&ui_context.open_elements_stack, ui_context.open_elements_stack.length - 1);
  return UI_ElementArrayGetPointer(&ui_context.elements, opened_element_id);
}

func void
UI_OpenElement()
{
  UI_Element element = {.id = ui_context.elements.length};

  UI_ElementArrayAdd(&ui_context.elements, element);
  UI_IDArrayAdd(&ui_context.open_elements_stack, element.id);
}

func void
UI_ConfigureElement(UI_ElementDescription description)
{
  UI_Element* element = UI_GetOpenedElement();
  element->description = description;

  if (element->description.layout.width.type == UI_SizeType_Pixel)
  {
    element->rect.size.x = element->description.layout.width.value;
  }
  if (element->description.layout.height.type == UI_SizeType_Pixel)
  {
    element->rect.size.y = element->description.layout.height.value;
  }
}

func void UI_CloseElement()
{
  UI_Element* current_element = UI_GetOpenedElement();

  current_element->children_array_slice.ids = ui_context.children.elements + ui_context.children.length;
  for (I32 i = 0; i < current_element->children_array_slice.length; i += 1)
  {
    I32 buffer_offset = ui_context.children_formation_buffer.length - current_element->children_array_slice.length + i;
    UI_ID child_id = UI_IDArrayGet(&ui_context.children_formation_buffer, buffer_offset);
    UI_IDArrayAdd(&ui_context.children, child_id);

    if (current_element->description.layout.height.type == UI_SizeType_FitChildren)
    {
      UI_Element* child_element = UI_ElementArrayGetPointer(&ui_context.elements, child_id);
      current_element->rect.size.y += child_element->rect.size.y;
    }
    if (current_element->description.layout.width.type == UI_SizeType_FitChildren)
    {
      UI_Element* child_element = UI_ElementArrayGetPointer(&ui_context.elements, child_id);
      current_element->rect.size.x += child_element->rect.size.x;
    }
  }
  ui_context.children_formation_buffer.length -= current_element->children_array_slice.length;

  if (current_element->children_array_slice.length)
  {
    UI_IDArrayAdd(&ui_context.branches, current_element->id);
  }

  UI_IDArrayPop(&ui_context.open_elements_stack);

  if (ui_context.open_elements_stack.length > 0)
  {
    UI_Element* parent_element = UI_GetOpenedElement();
    UI_IDArrayAdd(&ui_context.children_formation_buffer, current_element->id);
    parent_element->children_array_slice.length += 1;
  }
}

func B32
UI_Hovered()
{
  UI_Element* element = UI_GetOpenedElement();
  return ui_context.hot_id == element->id;
}

func B32
UI_IsClicked()
{
  B32 result = 0;
  UI_Element* current_element = UI_ElementArrayGetPointer(&ui_context.elements, ui_context.elements.length - 1);
  
  if (ui_context.active_id == current_element->id)
  {
    if ((ui_context.hot_id == current_element->id) && OS_IsMouseReleased(OS_MouseButton_Left))
    {
      ui_context.active_id = 0;
      result = 1;
    }
  }
  else if (ui_context.hot_id == current_element->id)
  {
    if (OS_IsMousePressed(OS_MouseButton_Left)) ui_context.active_id = current_element->id;
  }

  return result;
}

func RectF32
UI_GetElementRectF32()
{
  return UI_GetOpenedElement()->rect;
}

func void
UI_Text(Str8 text, UI_TextDescription text_description)
{
  // --AlNov 14 December 2025: @TODO
  // Roundtrip to set what text to draw. I am sure that there is a better way.
  UI_TextDescription with_str = text_description;
  with_str.str = text;

  Vec2F32 text_dimension = GetTextSize(text_description.font, text, text_description.font_size);

  UI_ElementBlock({
    .type = UI_ElementType_Text,
    .flags = UI_ElementFlag_DrawLabel,
    .text = with_str,
    .layout = {
      .width = UI_PixelSize(text_dimension.x),
      .height = UI_PixelSize(text_dimension.y),
    }
  });
}

#if 0
func UI_Element*
UI_Layout(UI_ElementArray* array, Str8 label)
{
  UI_Element* layout = UI_BuildElement(
    array,
    (UI_ElementDescription){
      .flags = UI_ElementFlag_DrawBackground,
    }
  );
  return layout;
}

func void
UI_NumberInput(UI_ElementArray* array, Str8 label, F32* value)
{
  F32 result = *value;

  UI_Element* input = UI_BuildElement(
    array,
    (UI_ElementDescription){
      .flags = UI_ElementFlag_Hover | UI_ElementFlag_DrawLabel | UI_ElementFlag_DrawBackground,
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
UI_Button(UI_ElementArray* array, Str8 label)
{
  B32 result = 0;

  UI_Element* button = UI_BuildElement(
    array,
    (UI_ElementDescription){
      .flags = UI_ElementFlag_Hover | UI_ElementFlag_DrawLabel | UI_ElementFlag_DrawBackground,
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
UI_SliderF32(UI_ElementArray* array, Str8 label, F32 min, F32 max, F32* value)
{
  UI_Element* slider = UI_BuildElement(
    array,
    (UI_ElementDescription){
      .flags = UI_ElementFlag_Hover|
        UI_ElementFlag_DrawLabel|
        UI_ElementFlag_DrawBackground,
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
