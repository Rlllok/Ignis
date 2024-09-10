#pragma once

#include <windows.h>

#include "../../../base/base_include.h"
#include "../os_gfx.h"

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
  HWND      handle;
  HINSTANCE instance;

  u32  width;
  u32  height;
  bool is_fullscreen;

  OS_WindowStatus status;
};

func void OS_WIN32_InitGfx();

// --AlNov: @TODO This function is alread defined in os_gfx.h.
// Is it better to not redefine. Better to experement
func OS_Window OS_CreateWindow(const char* title, Vec2u size);
func void      OS_ShowWindow(const OS_Window* window);
// func void      OS_Win32_ToggleFullscreen(const OS_Window* window);

func OS_EventList OS_GetEventList(Arena* arena);
func void         OS_PushEvent(OS_EventList* event_list, OS_Event* event);

func f32    OS_CurrentTimeSeconds();
func void   OS_Wait(f32 wait_seconds);
func Vec2f  OS_MousePosition(OS_Window window);

func LRESULT OS_WIN32_WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
