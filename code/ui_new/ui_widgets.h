#pragma once

typedef U8 UI_CustomWidgetKind;
enum {
  UI_CustomWidgetKind_None,
  UI_CustomWidgetKind_Hue,
  UI_CustomWidgetKind_ValueSaturation,
  UI_CistomWidgetKind_Count
} UI_CustomWidgetKindEnum;

typedef struct UI_CustomWidgetInfo UI_CustomWidgetInfo;
struct UI_CustomWidgetInfo {
  UI_CustomWidgetKind kind;
  union {
    struct {
      F32 value;
    } hue;
    struct {
      Vec3F32 hsv;
    } value_saturation;
  };
};

// -- Buttons
func B32 UI_Button(Str8 label, UI_TextStyleInfo text, UI_WidgetLayoutInfo layout, UI_WidgetStyleInfo style);
func void UI_RadioButton(Str8 label, I32* ptr, I32 value, UI_TextStyleInfo text, UI_WidgetLayoutInfo layout, UI_WidgetStyleInfo style);

// -- Numbers
func void UI_DragI32(Str8 label, I32* value, I32 step, I32 min, I32 max, UI_TextStyleInfo text, UI_WidgetLayoutInfo layout, UI_WidgetStyleInfo style);
func void UI_DragF32(Str8 label, F32* value, F32 step, F32 min, F32 max, UI_TextStyleInfo text, UI_WidgetLayoutInfo layout, UI_WidgetStyleInfo style);
func void UI_SliderI32(Str8 label, I32* value, I32 min, I32 max, UI_TextStyleInfo text, UI_WidgetLayoutInfo layout, UI_WidgetStyleInfo style);
func void UI_SliderF32(Str8 label, F32* value, F32 min, F32 max, UI_TextStyleInfo text, UI_WidgetLayoutInfo layout, UI_WidgetStyleInfo style);

// -- Colors
func void UI_DragRGB(Str8 label, Vec3F32* value, UI_TextStyleInfo text, UI_WidgetLayoutInfo layout, UI_WidgetStyleInfo style);
func void UI_ColorPicker(Str8 label, Vec4F32* value, UI_TextStyleInfo text, UI_WidgetLayoutInfo layout, UI_WidgetStyleInfo style);
