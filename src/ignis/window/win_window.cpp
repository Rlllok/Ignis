#include "win_window.h"

#include <iostream>

#include "ignis/input/win_keyboard.h"
#include "ignis/input/win_mouse.h"

Window::Window(const char* name, const int width, const int height)
    : name(name)
    , width(width)
    , height(height)
{
    const LPCSTR WINDOW_CLASS_NAME = name;

    instance = GetModuleHandle(nullptr);

    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = windowProcedure;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = sizeof(LONG_PTR);
    windowClass.hInstance = instance;
    windowClass.hIcon = LoadIcon(NULL, IDI_WINLOGO);
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.hbrBackground = NULL;
    windowClass.lpszMenuName = NULL;
    windowClass.lpszClassName = WINDOW_CLASS_NAME;

    if (RegisterClass(&windowClass) == 0)
    {
        std::cerr << "Cannot register window class." << std::endl;
        std::cerr << GetLastError() << std::endl;
    }

    windowHandle = CreateWindowEx(
        0,
        WINDOW_CLASS_NAME,
        name,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        width,
        height,
        NULL,
        NULL,
        instance,
        this
    );

    if (windowHandle == NULL)
    {
        std::cerr << "Cannot initialize app." << std::endl;
        std::cerr << GetLastError() << std::endl;
    }

    SetPropW(windowHandle, L"API", this);
}

void Window::show()
{
    ShowWindow(windowHandle, SW_SHOW);
    UpdateWindow(windowHandle);
}

LRESULT Window::windowProcedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        // Keyboard input handle
        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            bool wasKeyDown = (lParam & (1 << 30)) != 0;

            Keyboard::processInput(wParam, false, wasKeyDown, false);

            return 0;
        }

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        {
            bool wasKeyDown = (lParam & (1 << 30)) != 0;
            bool isKeyDown = (lParam & (1 << 31)) == 0;

            Keyboard::processInput(wParam, isKeyDown, false, !wasKeyDown && isKeyDown);

            return 0;
        }
        // End Keyboard input handle

        // Mouse input
        case WM_MOUSEMOVE:
        {
            Mouse::mouseMove(LOWORD(lParam), HIWORD(lParam));
            break;
        }
        case WM_LBUTTONDOWN:
        {
            Mouse::leftInput(true, LOWORD(lParam), HIWORD(lParam));
            break;
        }
        
        case WM_LBUTTONUP:
        {
            Mouse::leftInput(false, LOWORD(lParam), HIWORD(lParam));
            break;
        }
        // End Mouse input handle

        case WM_SIZE:
        {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);

            return 0;
        }

        case WM_CLOSE:
        {
            DestroyWindow(hwnd);
            return 0;
        }

        case WM_DESTROY:
        {
            // pApp->onClose();
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
