#pragma once

#include <windows.h>

#include <stdint.h>

class Keyboard
{
    // friend LRESULT IBaseApp::windowProcedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
    static bool isKeyDown(uint32_t keyCode);
    static bool WasKeyReleased(uint32_t keyCode);
    static bool WasKeyPressed(uint32_t keyCode);

private:
    static bool down;
    static bool pressed;
    static bool released;

public:
    static void processInput(uint32_t vkCode, bool isDown, bool isReleased, bool isPressed);
};  