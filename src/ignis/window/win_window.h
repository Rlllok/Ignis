#pragma once

#include <windows.h>

class Window
{
public:
    Window() = default;
    Window(const char* name, const int width, const int height); 

    ~Window() = default;

public:
    void show();

public:
    const char* name;
    int         width;
    int         height;

    HINSTANCE   instance;
    WNDCLASS    windowClass;
    HWND        windowHandle;

private:
    static LRESULT windowProcedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};