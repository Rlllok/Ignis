#pragma once

#include <glm/glm.hpp>

#include "uiElement.h"

#include <iostream>

class ColorEdit3 : public IUIElement
{
public:
    ColorEdit3(const char* label, glm::vec3& color);

public:
    void execute() override;

private:
    const char* label;
    glm::vec3& color;
};