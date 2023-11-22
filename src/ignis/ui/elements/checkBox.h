#pragma once

#include "uiElement.h"

class CheckBox : public IUIElement
{
public:
    CheckBox(const char* lable, bool& value);

private:
    const char* lable;
    bool&       value;

public:
    void execute() override;
};