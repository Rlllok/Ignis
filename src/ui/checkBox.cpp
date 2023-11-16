#include "checkBox.h"

#include <imgui.h>

CheckBox::CheckBox(const char* lable, bool& value)
    : lable(lable)
    , value(value)
{
}

void CheckBox::execute()
{
    ImGui::Checkbox(lable, &value);
}