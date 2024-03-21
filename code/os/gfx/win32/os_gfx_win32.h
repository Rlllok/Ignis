#pragma once

#include <windows.h>

#include "base/base_include.h"

enum OS_WindowStatus
{
    OS_WINDOW_STATUS_NONE,
    OS_WINDOW_STATUS_CREATED,
    OS_WINDOW_STATUS_OPEN,
    OS_WINDOW_STATUS_CLOSED,

    OS_WINDOW_STATUS_COUNT
};

struct OS_Window
{
    HWND handle;
    HINSTANCE instance;
    
    u32 width;
    u32 height;
    bool is_fullscreen;

    OS_WindowStatus status;
// --AlNov: @TODO Check name convention
};

void OS_WIN32_InitGfx();

// --AlNov: @TODO This function is alread defined in os_gfx.h.
// Is it better to not redefine. Better to experement
OS_Window OS_CreateWindow(const char* title, Vec2u size);
void OS_ShowWindow(const OS_Window* window);
void OS_Win32_ToggleFullscreen(const OS_Window* window);

OS_EventList OS_GetEventList(Arena* arena);
void OS_PushEvent(OS_EventList* eventList, OS_Event* event);

f32 OS_CurrentTimeSeconds();
Vec2f OS_MousePosition(OS_Window window);

LRESULT OS_WIN32_WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
