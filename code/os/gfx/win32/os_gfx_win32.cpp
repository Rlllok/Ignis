#include "../os_gfx.h"
#include "os_gfx_win32.h"

#include <stdio.h>

#pragma comment(lib, "user32.lib")

#define OS_WIN32_WindowClassName L"AppWindowClass"

global Arena* OS_WIN32_EventArena;
global OS_EventList* OS_WIN32_EventList;

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

    handle = CreateWindowW(OS_WIN32_WindowClassName, (LPCWSTR)title, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, size.width, size.height, 0, 0, window.instance, 0
    );

    if (handle == 0)
    {
        printf("Cannot create window. Error: %lu", GetLastError());
        window = {};
        return window;
    }

    window.handle = handle;
    window.width = size.width;
    window.height = size.height;
    window.status = OS_WINDOW_STATUS_CREATED;

    return window;
}

void OS_ShowWindow(OS_Window* window)
{
    ShowWindow(window->handle, SW_SHOW);
    UpdateWindow(window->handle);

    window->status = OS_WINDOW_STATUS_OPEN;
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
        DispatchMessage(&message);
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

LRESULT OS_WIN32_WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    OS_Event* event = 0;

    switch (message)
    {
        case WM_SIZE:
        {
            // --AlNov: @TODO handle window resize
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
           // --AlNov: Not interestiong for now.
        } break;

        case WM_SYSKEYUP:
        case WM_KEYUP:
        {
            if (wParam == VK_ESCAPE)
            {
                DestroyWindow(hwnd);
            }
        } break;

        // --AlNov: Mouse Input ------------------------------
        case WM_MOUSEMOVE:
        {
            event = (OS_Event*)PushArena(OS_WIN32_EventArena, sizeof(OS_Event)); 
            if (event)
            {
            event->type = OS_EVENT_TYPE_MOUSE_INPUT;
            event->mouseX = GET_X_LPARAM(lParam);
            event->mouseY = GET_Y_LPARAM(lParam);
            }
            else
            {
                printf("Wrong mouse event\n");
            }
        } break;

        default:
        {
            result = DefWindowProc(hwnd, message, wParam, lParam);
        } break;
    }

    if (event != 0)
    {
        OS_PushEvent(OS_WIN32_EventList, event);
    }

    return result;
}