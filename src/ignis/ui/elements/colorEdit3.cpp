#include "colorEdit3.h"

#include <imgui.h>

ColorEdit3::ColorEdit3(const char* label, glm::vec3& color)
    : label(label)
    , color(color)
{
    
}

void ColorEdit3::execute()
{
    ImGui::ColorEdit3(label, (float*)&color);
}
