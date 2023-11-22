#include "win_keyboard.h"

#include <iostream>

bool Keyboard::down = false;
bool Keyboard::pressed = false;
bool Keyboard::released = false;    

bool Keyboard::isKeyDown(uint32_t keyCode)
{
    bool result = down;
    down = false;
    return result;
}

bool Keyboard::WasKeyReleased(uint32_t keyCode)
{
    bool result = released;
    released = false;
    return result;
}

bool Keyboard::WasKeyPressed(uint32_t keyCode)
{
    bool result = pressed;
    pressed = false;
    return result;
}   

void Keyboard::processInput(uint32_t vkCode, bool isDown, bool isReleased, bool isPressed)
{
    down = isDown;
    released = isReleased;
    pressed = isPressed;
}
