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
