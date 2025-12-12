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
  ui_context.current_parent = 0;
  ui_context.mouse_position = mouse_position;
  ui_context.size_x = (UI_Size){0};
  ui_context.size_y = (UI_Size){0};
  ui_context.fixed_position = MakeVec2(0.0f, 0.0f);
  ui_context.font_size = 0.0f;
  ui_context.font = (FontBitmap){0};
  ui_context.text_color = MakeVec4(0.0f, 0.0f, 0.0f, 0.0f);
  ui_context.background_color = MakeVec4(0.0f, 0.0f, 0.0f, 0.0f);
  ui_context.hot_id = 0;
  UI_DrawCommandArrayReset(&ui_context.draw_commands);
}

func void UI_EndFrame(UI_Context* context) {}

// -------------------------------------------------------------------
// -- UI Default Elements --------------------------------------------
func UI_Element*
UI_BuildElement(UI_ElementArray* array, UI_ElementDescription description)
{
  UI_Element element = {0};
  element.id = array->length + 1;
  element.label = description.label;
  element.flags = description.flags;
  element.parent = ui_context.current_parent;
  element.layout = description.layout;
  element.padding = description.padding;
  element.child_gap = description.child_gap;
  element.font = ui_context.font;
  element.font_size = ui_context.font_size;
  element.border_radius = description.border_radius;
  element.text_color = ui_context.text_color;

  element.child_offset = MakeVec2(element.padding.left, element.padding.top);

  switch (ui_context.size_x.type)
  {
    default: Assert(1); break;

    case UI_SizeType_Fixed:
    {
      element.rect.size.x = ui_context.size_x.value;
    } break;
    case UI_SizeType_WrapLabel:
    {
      element.rect.size.x = GetTextSize(ui_context.font, element.label, element.font_size).x;
    } break;
    case UI_SizeType_WrapChildren:
    {
      // --AlNov: @TODO
      Assert(1);
    } break;
    case UI_SizeType_ParentPercent:
    {
      element.rect.size.x = element.parent->rect.size.x*ui_context.size_x.value;
    } break;
  }
  switch (ui_context.size_y.type)
  {
    default: Assert(1); break;

    case UI_SizeType_Fixed:
    {
      element.rect.size.y = ui_context.size_y.value;
    } break;
    case UI_SizeType_WrapLabel:
    {
      element.rect.size.y = GetTextSize(ui_context.font, element.label, element.font_size).y;
    } break;
    case UI_SizeType_WrapChildren:
    {
      // --AlNov: @TODO
      Assert(1);
    } break;
    case UI_SizeType_ParentPercent:
    {
      element.rect.size.y = element.parent->rect.size.y*ui_context.size_y.value;
    } break;
  }

  if (element.parent)
  {
    switch (element.parent->layout)
    {
      case UI_LayoutDirection_TopToBottom:
      {
        element.rect.position = AddVec2(element.parent->rect.position, element.parent->child_offset);
        element.parent->child_offset.y += element.rect.size.y + element.parent->child_gap;
      } break;
      case UI_LayoutDirection_LeftToRight:
      {
        element.rect.position = AddVec2(element.parent->rect.position, element.parent->child_offset);
        element.parent->child_offset.x += element.rect.size.x + element.parent->child_gap;
      } break;
    }
  }
  else
  {
    element.rect.position = ui_context.fixed_position;
  }

  element.text_color = ui_context.text_color;
  element.background_color = ui_context.background_color;

  if (element.flags & UI_ElementFlag_Hover)
  {
    if (InsideRectF32(element.rect, ui_context.mouse_position))
    {
      ui_context.hot_id = element.id;
      element.background_color = AddVec4(element.background_color, MakeVec4(0.2f, 0.2f, 0.2f, 0.0f));
    }
  }
  if (element.flags & UI_ElementFlag_Clickable)
  {
  }
  if (element.flags & UI_ElementFlag_DrawBackground)
  {
    UI_DrawCommandArrayAdd(
      &ui_context.draw_commands,
      (UI_DrawCommand){
        .type = UI_DrawCommandType_Rectangle,
        .rectangle = {
          .color = element.background_color,
          .bound = element.rect,
          .radius = element.border_radius.values,
        }
      }
    );
  }
  if (element.flags & UI_ElementFlag_DrawLabel)
  {
    UI_DrawCommandArrayAdd(
      &ui_context.draw_commands,
      (UI_DrawCommand){
        .type = UI_DrawCommandType_Text,
        .text = {
          .content = element.label,
          .font = element.font,
          .font_size = element.font_size,
          .color = element.text_color,
          .position = element.rect.position,
        }
      }
    );
  }

  return UI_ElementArrayGetPointer(array, UI_ElementArrayAdd(array, element));
}

func UI_Element*
UI_Layout(UI_ElementArray* array, Str8 label)
{
  UI_Element* layout = UI_BuildElement(
    array,
    (UI_ElementDescription){
      .label = label,
      .flags = UI_ElementFlag_DrawBackground,
    }
  );
  return layout;
}

func void
UI_Text(UI_ElementArray* array, Str8 label)
{
  UI_BuildElement(
    array,
    (UI_ElementDescription){
      .label = label,
      .flags = UI_ElementFlag_DrawLabel,
    }
  );
}

func void
UI_NumberInput(UI_ElementArray* array, Str8 label, F32* value)
{
  F32 result = *value;

  UI_Element* input = UI_BuildElement(
    array,
    (UI_ElementDescription){
      .label = label,
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
UI_Button(UI_ElementArray* array, Str8 label)
{
  B32 result = 0;

  UI_Element* button = UI_BuildElement(
    array,
    (UI_ElementDescription){
      .label = label,
      .flags = UI_ElementFlag_Hover|
        UI_ElementFlag_DrawLabel|
        UI_ElementFlag_DrawBackground,
      .border_radius = {
        .top_right = 10.0f,
        .bottom_right = 10.0f,
      },
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
      .label = label,
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
