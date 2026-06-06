#pragma once

// -- Buttons
func B32 UI_Button(Str8 label, UI_TextStyleInfo text, UI_WidgetLayoutInfo layout, UI_WidgetStyleInfo style);
func void UI_RadioButton(Str8 label, I32* ptr, I32 value, UI_TextStyleInfo text, UI_WidgetLayoutInfo layout, UI_WidgetStyleInfo style);

// -- Numbers
func void UI_SliderI32(Str8 label, I32* value, I32 min, I32 max, UI_TextStyleInfo text, UI_WidgetLayoutInfo layout, UI_WidgetStyleInfo style);
func void UI_SliderF32(Str8 label, F32* value, F32 min, F32 max, UI_TextStyleInfo text, UI_WidgetLayoutInfo layout, UI_WidgetStyleInfo style);
