#include "ui_widgets.h"

// -- Buttons
func B32
UI_Button(Str8 label, UI_TextStyleInfo text, UI_WidgetLayoutInfo layout, UI_WidgetStyleInfo style) {
  B32 result = 0;
  UI_WidgetBlock(
    label,
    {
      .flags = UI_WidgetFlag_MouseInteraction|UI_WidgetFlag_DrawBackground|UI_WidgetFlag_DrawText,
      .layout = layout,
      .style = style,
      .text = text,
    }
  ) {
    result = UI_IsHot() && OS_MousePressed(OS_MouseButton_Left);
  }
  return result;
}

func void
UI_RadioButton(Str8 label, I32* ptr, I32 value, UI_TextStyleInfo text, UI_WidgetLayoutInfo layout, UI_WidgetStyleInfo style) {
  UI_WidgetBlock(
    label,
    {
      .flags = UI_WidgetFlag_MouseInteraction|UI_WidgetFlag_DrawBackground,
      .layout = layout,
      .style = style,
      .text = text
    }
  ) {
    if (UI_IsHot() && OS_MousePressed(OS_MouseButton_Left)) {
      *ptr = value;
    }

    if (*ptr == value) {
      Str8 radio_button_active_label = ConcatStr8(ui_current_context->arena, label, Str8C("_Active"));
      UI_WidgetBlock(
        radio_button_active_label,
        {
          .flags = UI_WidgetFlag_DrawBackground,
          .layout = {
            .width = UI_PercentSize(1.0f),
            .height = UI_PercentSize(1.0f),
          },
          .style = {
            .background_color = RGBAFromHex(0xf5f5f5ff),
          }
        }
      ) {
      }
    }
  }
}

// -- Numbers
func void
UI_DragI32(Str8 label, I32* value, I32 step, I32 min, I32 max, UI_TextStyleInfo text, UI_WidgetLayoutInfo layout, UI_WidgetStyleInfo style) {
  UI_WidgetBlock(
    label,
    {
      .flags = UI_WidgetFlag_MouseInteraction|UI_WidgetFlag_DrawBackground|UI_WidgetFlag_DrawText,
      .layout = layout,
      .style = style,
      .text = text,
    }
  ) {
    if (UI_IsHot() && OS_MousePressed(OS_MouseButton_Left)) {
      UI_SetActive();
    }

    if (UI_IsActive()) {
      Vec2F32 mouse_position_delta = UI_GetMousePositionDelta();
      *value += (I32)RoundF32(mouse_position_delta.x*(F32)step);
      *value = Clamp(*value + (I32)RoundF32(mouse_position_delta.x*(F32)step), min, max);
      if (OS_MouseReleased(OS_MouseButton_Left)) {
        UI_UnsetActive();
      }
    }
  }
}

func void
UI_DragF32(Str8 label, F32* value, F32 step, F32 min, F32 max, UI_TextStyleInfo text, UI_WidgetLayoutInfo layout, UI_WidgetStyleInfo style) {
  UI_WidgetBlock(
    label,
    {
      .flags = UI_WidgetFlag_MouseInteraction|UI_WidgetFlag_DrawBackground|UI_WidgetFlag_DrawText,
      .layout = layout,
      .style = style,
      .text = text,
    }
  ) {
    if (UI_IsHot() && OS_MousePressed(OS_MouseButton_Left)) {
      UI_SetActive();
    }

    if (UI_IsActive()) {
      Vec2F32 mouse_position_delta = UI_GetMousePositionDelta();
      *value = Clamp(*value + mouse_position_delta.x*step, min, max);
      if (OS_MouseReleased(OS_MouseButton_Left)) {
        UI_UnsetActive();
      }
    }
  }
}

func void
UI_SliderI32(Str8 label, I32* value, I32 min, I32 max, UI_TextStyleInfo text, UI_WidgetLayoutInfo layout, UI_WidgetStyleInfo style) {
  UI_WidgetBlock(
    label,
    {
      .flags = UI_WidgetFlag_MouseInteraction|UI_WidgetFlag_DrawBackground|UI_WidgetFlag_DrawText,
      .layout = layout,
      .style = style,
      .text = text,
    }
  ) {
    if (UI_IsHot() && OS_MousePressed(OS_MouseButton_Left)) {
      UI_SetActive();
    }

    if (UI_IsActive()) {
      Vec2F32 mouse_position = UI_GetMousePosition();
      RectF32 bounding_box = UI_GetBoundingBox();
      F32 local_mouse_x = Clamp(mouse_position.x - bounding_box.x, 0, bounding_box.w);
      F32 t = local_mouse_x/bounding_box.w;
      *value = (I32)RoundF32((1.0f - t)*(F32)min + t*(F32)max);
      if (OS_MouseReleased(OS_MouseButton_Left)) {
        UI_UnsetActive();
      }
    }

    ScratchArena scratch = BeginScratchArena(ui_current_context->frame_arena); {
      F32 percent = ((F32)*value - (F32)min)/((F32)max - (F32)min);
      Str8 internal_slider_label = ConcatStr8(scratch.arena, label, Str8C("_Internal"));
      UI_WidgetBlock(
        internal_slider_label,
        {
          .flags = UI_WidgetFlag_DrawBackground,
          .layout = {
            .width = UI_PercentSize(percent),
            .height = UI_PercentSize(1.0f),
          },
          .style = {
            .background_color = RGBAFromHex(0xf0f0f044),
          }
        }
      ) {
      }
    }
    EndScratchArena(scratch);
  }
}

func void
UI_SliderF32(Str8 label, F32* value, F32 min, F32 max, UI_TextStyleInfo text, UI_WidgetLayoutInfo layout, UI_WidgetStyleInfo style) {
  UI_WidgetBlock(
    label,
    {
      .flags = UI_WidgetFlag_MouseInteraction|UI_WidgetFlag_DrawBackground|UI_WidgetFlag_DrawText,
      .layout = layout,
      .style = style,
      .text = text,
    }
  ) {
    if (UI_IsHot() && OS_MousePressed(OS_MouseButton_Left)) {
      UI_SetActive();
    }

    if (UI_IsActive()) {
      Vec2F32 mouse_position = UI_GetMousePosition();
      RectF32 bounding_box = UI_GetBoundingBox();
      F32 local_mouse_x = Clamp(mouse_position.x - bounding_box.x, 0, bounding_box.w);
      F32 t = local_mouse_x/bounding_box.w;
      *value = (1.0f - t)*min + t*max;
      if (OS_MouseReleased(OS_MouseButton_Left)) {
        UI_UnsetActive();
      }
    }

    ScratchArena scratch = BeginScratchArena(ui_current_context->frame_arena); {
      F32 percent = (*value - min)/(max - min);
      Str8 internal_slider_label = ConcatStr8(scratch.arena, label, Str8C("_Internal"));
      UI_WidgetBlock(
        internal_slider_label,
        {
          .flags = UI_WidgetFlag_DrawBackground,
          .layout = {
            .width = UI_PercentSize(percent),
            .height = UI_PercentSize(1.0f),
          },
          .style = {
            .background_color = RGBAFromHex(0xf0f0f044),
          }
        }
      ) {
      }
    }
    EndScratchArena(scratch);
  }
}

// -- Colors
func void
UI_DragRGB(Str8 label, Vec3F32* value, UI_TextStyleInfo text, UI_WidgetLayoutInfo layout, UI_WidgetStyleInfo style) {
  UI_WidgetBlock(
    label,
    {
      .layout = layout,
      .style = style,
      .text = text,
    }
  ) {
      UI_WidgetLayoutInfo color_drag_layout = {
        .width = UI_FillSize(),
        .height = UI_PercentSize(1.0f),
      };
      UI_TextStyleInfo color_drag_text = text;
      color_drag_text.str = FormatStr8(ui_current_context->frame_arena, "%f", value->r);
      UI_DragF32(ConcatStr8(ui_current_context->frame_arena, label, Str8C("_RedValue")), &value->r, 0.001f, 0.0f, 1.0f, color_drag_text, color_drag_layout, style);
      color_drag_text.str = FormatStr8(ui_current_context->frame_arena, "%f", value->g);
      UI_DragF32(ConcatStr8(ui_current_context->frame_arena, label, Str8C("_GreenValue")), &value->g, 0.001f, 0.0f, 1.0f, color_drag_text, color_drag_layout, style);
      color_drag_text.str = FormatStr8(ui_current_context->frame_arena, "%f", value->b);
      UI_DragF32(ConcatStr8(ui_current_context->frame_arena, label, Str8C("_BlueValue")), &value->b, 0.001f, 0.0f, 1.0f, color_drag_text, color_drag_layout, style);
      UI_WidgetBlock(
        Str8C("nmqwenk"),
        {
          .flags = UI_WidgetFlag_DrawBackground,
          .layout = {
            .width = UI_PixelSize(40.0f),
            .height = UI_PercentSize(1.0f),
          },
          .style = {
            .background_color = MakeVec4F32(value->r, value->g, value->b, 1.0f),
          }
        }
      ) {
      }
  }
}

func void
UI_ColorPicker(Str8 label, Vec4F32* color, UI_TextStyleInfo text, UI_WidgetLayoutInfo layout, UI_WidgetStyleInfo style) {
  layout.direction = UI_Axis_Y;
  UI_WidgetBlock(
    label,
    {
      .flags = UI_WidgetFlag_DrawBackground,
      .layout = layout,
      .style = style,
    }
  ) {
    Vec3F32 hsv = HSVFromRGB(MakeVec3F32(color->r, color->g, color->b));
    UI_CustomWidgetInfo* value_saturation_info = (UI_CustomWidgetInfo*)PushArena(ui_current_context->frame_arena, sizeof(UI_CustomWidgetInfo));
    value_saturation_info->kind = UI_CustomWidgetKind_ValueSaturation;
    value_saturation_info->value_saturation.hsv = hsv;
    UI_WidgetBlock(
      ConcatStr8(ui_current_context->frame_arena, label, Str8C("_ColorWheel")),
      {
        .flags = UI_WidgetFlag_MouseInteraction|UI_WidgetFlag_DrawCustom,
        .layout = {
          .width = UI_PercentSize(1.0f),
          .height = UI_PercentSize(0.5f),
        },
        .custom = value_saturation_info,
      }
    ) {
      if (UI_IsHot() && OS_MousePressed(OS_MouseButton_Left)) {
        UI_SetActive();
      }

      if (UI_IsActive()) {
        Vec2F32 mouse_position = UI_GetMousePosition();
        RectF32 bounding_box = UI_GetBoundingBox();
        hsv.y = Clamp(mouse_position.x - bounding_box.x, 0.0f, bounding_box.w)/bounding_box.w;
        hsv.z = 1.0f - Clamp(mouse_position.y - bounding_box.y, 0.0f, bounding_box.h)/bounding_box.h;
        if (OS_MouseReleased(OS_MouseButton_Left)) {
          UI_UnsetActive();
        }
      }
    }
    UI_WidgetBlock(
      ConcatStr8(ui_current_context->frame_arena, label, Str8C("_HueSlider")),
      {
        .flags = UI_WidgetFlag_MouseInteraction|UI_WidgetFlag_DrawBackground,
        .layout = {
          .width = UI_PercentSize(1.0f),
          .height = UI_PixelSize(20.0f),
        },
        .style = {
          .background_color = MakeVec4F32(hsv.x, 0.0f, 0.0f, 1.0f),
        }
      }
    ) {
      if (UI_IsHot() && OS_MousePressed(OS_MouseButton_Left)) {
        UI_SetActive();
      }

      if (UI_IsActive()) {
        Vec2F32 mouse_position = UI_GetMousePosition();
        RectF32 bounding_box = UI_GetBoundingBox();
        F32 local_mouse_x = Clamp(mouse_position.x - bounding_box.x, 0, bounding_box.w);
        hsv.x = local_mouse_x/bounding_box.w;
        if (OS_MouseReleased(OS_MouseButton_Left)) {
          UI_UnsetActive();
        }
      }
    }

    UI_WidgetBlock(
      ConcatStr8(ui_current_context->frame_arena, label, Str8C("_ResultColor")),
      {
        .flags = UI_WidgetFlag_DrawBackground,
        .layout = {
          .width = UI_PercentSize(1.0f),
          .height = UI_PixelSize(20.0f),
        },
        .style = {
          .background_color = MakeVec4F32(color->r, color->g, color->b, 1.0f),
        },
      }
    ) {
    }

    Vec3F32 rgb = RGBFromHSV(hsv);
    *color = MakeVec4F32(rgb.r, rgb.g, rgb.b, 1.0f);
  } 
}
