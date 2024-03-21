#include "../os_gfx.h"
#include "os_gfx_win32.h"

#include <stdio.h>

#pragma comment(lib, "user32.lib")

#define OS_WIN32_WindowClassName L"AppWindowClass"

global Arena* OS_WIN32_EventArena;
global OS_EventList* OS_WIN32_EventList;
global WINDOWPLACEMENT previous_window_params = { sizeof(previous_window_params) };

void OS_WIN32_InitGfx()
{
}

OS_Window OS_CreateWindow(const char* title, Vec2u size)
{
    OS_Window window = {};
    window.instance = GetModuleHandle(nullptr);

    WNDCLASSW windowClass = {};
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = OS_WIN32_WindowProcedure;
    windowClass.hInstance = window.instance;
    windowClass.lpszClassName = OS_WIN32_WindowClassName;
    windowClass.hCursor = LoadCursor(0, IDC_ARROW);
    
    if (RegisterClassW(&windowClass) == 0)
    {
        printf("Cannot create window class./n");
        window = {};
        return window;
    }
    
    HWND handle = {};

    handle = CreateWindowW(OS_WIN32_WindowClassName, L"TestApp", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, size.width, size.height, 0, 0, window.instance, 0
    );

    if (handle == 0)
    {
        printf("Cannot create window. Error: %lu\n", GetLastError());
        window = {};
        return window;
    }

    window.handle = handle;
    window.width = size.width;
    window.height = size.height;
    window.is_fullscreen = false;
    window.status = OS_WINDOW_STATUS_CREATED;

    return window;
}

void OS_ShowWindow(OS_Window* window)
{
    ShowWindow(window->handle, SW_SHOW);
    UpdateWindow(window->handle);

    window->status = OS_WINDOW_STATUS_OPEN;
}

void OS_WIN32_ToggleFullscreen(HWND window_handle)
{
    DWORD style = GetWindowLong(window_handle, GWL_STYLE);
    if (style & WS_OVERLAPPEDWINDOW)
    {
        MONITORINFO monitor_info = { sizeof(monitor_info) };
        if (GetWindowPlacement(window_handle, &previous_window_params)
            && GetMonitorInfo(MonitorFromWindow(window_handle, MONITOR_DEFAULTTOPRIMARY), &monitor_info))
        {
          SetWindowLong(window_handle, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
          SetWindowPos(window_handle, HWND_TOP,
                       monitor_info.rcMonitor.left, monitor_info.rcMonitor.top,
                       monitor_info.rcMonitor.right - monitor_info.rcMonitor.left,
                       monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top,
                       SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else
    {
        SetWindowLong(window_handle, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(window_handle, &previous_window_params);
        SetWindowPos(window_handle, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

OS_EventList OS_GetEventList(Arena* arena)
{
    OS_EventList eventList = {};
    OS_WIN32_EventArena = arena;
    OS_WIN32_EventList = &eventList;

    MSG message;
    while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    // OS_WIN32_EventArena = 0;
    OS_WIN32_EventList = 0;

    return eventList;
}

f32 OS_CurrentTimeSeconds()
{
    // --AlNov: @NOTE Frequency should be computed only ones, as it doens't change after system start.
    localPersist LARGE_INTEGER frequency = {};
    if (!frequency.QuadPart)
    {
        QueryPerformanceFrequency(&frequency);
    }

    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);

    return (1000.f * (f32)counter.QuadPart) / (f32)frequency.QuadPart;
}

Vec2f OS_MousePosition(OS_Window window)
{
    POINT mousePoint;
    GetCursorPos(&mousePoint);
    ScreenToClient(window.handle, &mousePoint);

    return MakeVec2f((f32)mousePoint.x, (f32)mousePoint.y);
}

bool OS_IsWindowClosed()
{
    return false;
}

void OS_PushEvent(OS_EventList* eventList, OS_Event* event)
{
    if (eventList->eventCount == 0)
    {
        eventList->firstEvent = event;
        eventList->lastEvent = event;
        eventList->eventCount = 1;

        event->previous = 0;
        event->next = 0;
    }
    else
    {
        event->previous = eventList->lastEvent;
        event->next = 0;
        eventList->lastEvent->next = event;
        eventList->lastEvent = event;
        ++eventList->eventCount;
    }
}

f32 OS_GetMonitorHZ()
{
    DEVMODEW devMode = {};
    EnumDisplaySettingsW(0, ENUM_CURRENT_SETTINGS, &devMode);

    return (f32)devMode.dmDisplayFrequency;
}

LRESULT OS_WIN32_WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    OS_Event* event = 0;
    bool release = false;

    switch (message)
    {
        case WM_SIZE:
        {
            // --AlNov: WM_SIZE called on window creation - before there arena
            // mapped to OS_WIN32_EventArena
            if (!OS_WIN32_EventArena) break;

            event = (OS_Event*)PushArena(OS_WIN32_EventArena, sizeof(OS_Event));
            event->type = OS_EVENT_TYPE_RESIZE;
            event->windowSize.width = LOWORD(lParam);
            event->windowSize.height = HIWORD(lParam);
        } break;

        case WM_CLOSE:
        {
            DestroyWindow(hwnd);
        } break;

        case WM_DESTROY:
        {
            event = (OS_Event*)PushArena(OS_WIN32_EventArena, sizeof(OS_Event));
            event->type = OS_EVENT_TYPE_EXIT;

            PostQuitMessage(0);
        } break;

        // --AlNov: Keyboard handle. Now only for ESC to close program.
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
        {
            DefWindowProcW(hwnd, message, wParam, lParam);
        } // go through;
        case WM_SYSKEYUP:
        case WM_KEYUP:
        {
            bool wasDown = !!(lParam & (1 << 30));
            bool isDown = !(lParam & (1 << 31));

            if (wParam == VK_ESCAPE)
            {
                DestroyWindow(hwnd);
            }

            if (wParam == VK_LEFT)
            {
                event = (OS_Event*)PushArena(OS_WIN32_EventArena, sizeof(OS_Event));
                event->type = OS_EVENT_TYPE_KEYBOARD;
                event->key = OS_KEY_ARROW_LEFT;
                event->wasDown = wasDown;
                event->isDown = isDown;
            }

            if (wParam == VK_RIGHT)
            {
                event = (OS_Event*)PushArena(OS_WIN32_EventArena, sizeof(OS_Event));
                event->type = OS_EVENT_TYPE_KEYBOARD;
                event->key = OS_KEY_ARROW_RIGHT;
                event->wasDown = wasDown;
                event->isDown = isDown;
            }

            if (wParam == VK_RETURN && (lParam & (1 << 29)))
            {
                if (wasDown && !isDown)
                {
                    OS_WIN32_ToggleFullscreen(hwnd);
                }
            }
        } break;

        // --AlNov: Mouse Input ------------------------------
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
        {
            release = 1;
        } // go through;
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        {
            event = (OS_Event*)PushArena(OS_WIN32_EventArena, sizeof(OS_Event));
            if (release)
            {
                event->type = OS_EVENT_TYPE_MOUSE_RELEASE;
            } 
            else
            {
                event->type = OS_EVENT_TYPE_MOUSE_PRESS;
            }
            event->mouseX = LOWORD(lParam);
            event->mouseY = HIWORD(lParam);

            // if (release)
            // {
            //     ReleaseCapture();
            // }
            // else
            // {
            //     SetCapture(hwnd);
            // }
        } break;

        default:
        {
            result = DefWindowProcW(hwnd, message, wParam, lParam);
        } break;
    }

    if (event != 0)
    {
        OS_PushEvent(OS_WIN32_EventList, event);
    }

    return result;
}
