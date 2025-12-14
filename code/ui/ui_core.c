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


// -------------------------------------------------------------------
// -- UI Context Mutation --------------------------------------------
func void
UI_BeginFrame(Vec2 mouse_position)
{
  ui_context.mouse_position = mouse_position;
  ui_context.hot_id = 0;
  UI_DrawCommandArrayReset(&ui_context.draw_commands);
}

func void UI_EndFrame() {}

// -------------------------------------------------------------------
// -- UI Default Elements --------------------------------------------
func U32
UI_CalculateSize(UI_Size size, B32 is_heigth)
{
  U32 result = 0;
  switch (size.type)
  {
    default: Assert(1); break;
    
    case UI_SizeType_Fixed:
    {
      result = size.value;
    } break;
    case UI_SizeType_WrapChildren:
    {
      // --AlNov: @TODO
      Assert(1);
    } break;
    case UI_SizeType_ParentPercent:
    {
      UI_Element* parent = UI_GetParent();
      U32 parent_size = (is_heigth) ? parent->rect.size.y : parent->rect.size.x;
      result = parent_size*size.value;
    } break;
    case UI_SizeType_ParentFill:
    {
      UI_Element* parent = UI_GetParent();
      U32 parent_size = (is_heigth) ? parent->rect.size.y : parent->rect.size.x;
      U32 offset = (is_heigth) ? parent->child_offset.y : parent->child_offset.x;
      result = parent_size - offset;
    };
  }

  return result;
}

func UI_Element*
UI_BuildElement(UI_ElementArray* array, UI_ElementDescription description)
{
  UI_Element element = {0};
  element.id = array->length + 1;
  element.description = description;

  UI_Element* parent = UI_GetParent();

  element.child_offset = MakeVec2(description.layout.padding.left, description.layout.padding.top);

  element.rect.size.x = UI_CalculateSize(element.description.layout.width, 0);
  element.rect.size.y = UI_CalculateSize(element.description.layout.height, 1);

  if (parent != &UI_ElementDefaultValue)
  {
    switch (parent->description.layout.direction)
    {
      case UI_LayoutDirection_TopToBottom:
      {
        element.rect.position = AddVec2(parent->rect.position, parent->child_offset);
        parent->child_offset.y += element.rect.size.y + parent->description.layout.child_gap;
      } break;
      case UI_LayoutDirection_LeftToRight:
      {
        element.rect.position = AddVec2(parent->rect.position, parent->child_offset);
        parent->child_offset.x += element.rect.size.x + parent->description.layout.child_gap;
      } break;
    }
  }
  else
  {
    // --AlNov 14 December 2025: @TODO Floating Elements doesn't work
    element.rect.position = MakeVec2F32(0, 0);
  }

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
          .color = element.description.rectangle.color,
          .bound = element.rect,
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

  return UI_ElementArrayGetPointer(array, UI_ElementArrayAdd(array, element));
}

func UI_Element*
UI_GetParent()
{
  return UI_ElementArrayGetPointer(&ui_context.elements, ui_context.elements.length - 1);
}

func void UI_PushElement(UI_ElementDescription description)
{
  UI_Element* element = UI_BuildElement(&ui_context.elements, description);
}

func void UI_PopElement()
{
  UI_ElementArrayPop(&ui_context.elements);
}

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
UI_Text(Str8 text, UI_TextDescription text_description)
{
  // --AlNov 14 December 2025: @TODO
  // Roundtrip to set what text to draw. I am sure that there is a better way.
  UI_TextDescription with_str = text_description;
  with_str.str = text;

  UI_PushElement(
    (UI_ElementDescription){
      .flags = UI_ElementFlag_DrawLabel,
      .text = with_str,
    }
  );
  UI_PopElement();
}

func void
UI_NumberInput(UI_ElementArray* array, Str8 label, F32* value)
{
  F32 result = *value;

  UI_Element* input = UI_BuildElement(
    array,
    (UI_ElementDescription){
      .flags = UI_ElementFlag_Hover|
        UI_ElementFlag_DrawLabel|
        UI_ElementFlag_DrawBackground,
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
  UI_Element* current_element = UI_ElementArrayGetPointer(&ui_context.elements, ui_context.elements.length - 1);
  return current_element->rect;
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
