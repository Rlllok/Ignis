#include "win_mouse.h"

#include <iostream>

Mouse::Buttons Mouse::buttons = Mouse::Buttons();
Mouse::Position Mouse::position = Mouse::Position();

void Mouse::leftInput(bool left, float x, float y)
{
    position.x = x;
    position.y = y;

    if (buttons.left == true && left == false)
    {
        std::cout << "Left Mouse: " << left << std::endl;
        std::cout << "Position: " << x << ", " << y << std::endl;
    }

    buttons.left = left;
}

void Mouse::mouseMove(float x, float y)
{
    position.x = x;
    position.y = y;
}
