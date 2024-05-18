#include "os_gfx_win32.h"

#include <stdio.h>

#pragma comment(lib, "user32.lib")

#define OS_WIN32_WindowClassName L"AppWindowClass"

global Arena* os_event_arena;
global OS_EventList* os_event_list;
global WINDOWPLACEMENT previous_window_params = { sizeof(previous_window_params) };

func void OS_WIN32_InitGfx()
{
}

func OS_Window OS_CreateWindow(const char* title, Vec2u size)
{
  OS_Window window = {};
  window.instance = GetModuleHandle(nullptr);

  WNDCLASSW window_class     = {};
  window_class.style         = CS_HREDRAW | CS_VREDRAW;
  window_class.lpfnWndProc   = OS_WIN32_WindowProcedure;
  window_class.hInstance     = window.instance;
  window_class.lpszClassName = OS_WIN32_WindowClassName;
  window_class.hCursor       = LoadCursor(0, IDC_ARROW);

  if (RegisterClassW(&window_class) == 0)
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

  window.handle        = handle;
  window.width         = size.width;
  window.height        = size.height;
  window.is_fullscreen = false;
  window.status        = OS_WINDOW_STATUS_CREATED;

  return window;
}

func void OS_ShowWindow(OS_Window* window)
{
  ShowWindow(window->handle, SW_SHOW);
  UpdateWindow(window->handle);

  window->status = OS_WINDOW_STATUS_OPEN;
}

func void OS_WIN32_ToggleFullscreen(HWND window_handle)
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

func OS_EventList OS_GetEventList(Arena* arena)
{
  OS_EventList event_list = {};
  os_event_arena          = arena;
  os_event_list           = &event_list;

  MSG message;
  while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
  {
    TranslateMessage(&message);
    DispatchMessageW(&message);
  }

  os_event_list = 0;

  return event_list;
}

func f32 OS_CurrentTimeSeconds()
{
  // --AlNov: @NOTE Frequency should be computed only ones, as it doens't change after system start.
  local_persist LARGE_INTEGER frequency = {};
  if (!frequency.QuadPart)
  {
    QueryPerformanceFrequency(&frequency);
  }

  LARGE_INTEGER counter;
  QueryPerformanceCounter(&counter);

  return (1000.f * (f32)counter.QuadPart) / (f32)frequency.QuadPart;
}

func Vec2f OS_MousePosition(OS_Window window)
{
  POINT mouse_point;
  GetCursorPos(&mouse_point);
  ScreenToClient(window.handle, &mouse_point);

  return MakeVec2f((f32)mouse_point.x, (f32)mouse_point.y);
}

func bool OS_IsWindowClosed()
{
  return false;
}

func void OS_PushEvent(OS_EventList* event_list, OS_Event* event)
{
  if (event_list->count == 0)
  {
    event_list->first = event;
    event_list->last  = event;
    event_list->count = 1;

    event->previous = 0;
    event->next     = 0;
  }
  else
  {
    event->previous        =  event_list->last;
    event->next            =  0;
    event_list->last->next =  event;
    event_list->last       =  event;
    event_list->count      += 1;
  }
}

func f32 OS_GetMonitorHZ()
{
  DEVMODEW dev_mode = {};
  EnumDisplaySettingsW(0, ENUM_CURRENT_SETTINGS, &dev_mode);

  return (f32)dev_mode.dmDisplayFrequency;
}

func LRESULT OS_WIN32_WindowProcedure(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param)
{
  LRESULT   result  = 0;
  OS_Event* event   = 0;
  bool      release = false;

  switch (message)
  {
    case WM_SIZE:
      {
        // --AlNov: WM_SIZE called on window creation - before there arena
        // mapped to os_event_arena
        if (!os_event_arena) break;

        event                     = (OS_Event*)PushArena(os_event_arena, sizeof(OS_Event));
        event->type               = OS_EVENT_TYPE_RESIZE;
        event->window_size.width  = LOWORD(l_param);
        event->window_size.height = HIWORD(l_param);
      } break;

    case WM_CLOSE:
      {
        DestroyWindow(hwnd);
      } break;

    case WM_DESTROY:
      {
        event       = (OS_Event*)PushArena(os_event_arena, sizeof(OS_Event));
        event->type = OS_EVENT_TYPE_EXIT;

        PostQuitMessage(0);
      } break;

    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
      {
        DefWindowProcW(hwnd, message, w_param, l_param);
      } // go through;
    case WM_SYSKEYUP:
    case WM_KEYUP:
      {
        bool was_down = !!(l_param & (1 << 30));
        bool is_down  = !(l_param & (1 << 31));

        if (w_param == VK_ESCAPE)
        {
          DestroyWindow(hwnd);
        }

        if (w_param == VK_UP)
        {
          event           = (OS_Event*)PushArena(os_event_arena, sizeof(OS_Event));
          event->type     = OS_EVENT_TYPE_KEYBOARD;
          event->key      = OS_KEY_ARROW_UP;
          event->was_down = was_down;
          event->is_down  = is_down;
        }

        if (w_param == VK_DOWN)
        {
          event           = (OS_Event*)PushArena(os_event_arena, sizeof(OS_Event));
          event->type     = OS_EVENT_TYPE_KEYBOARD;
          event->key      = OS_KEY_ARROW_DOWN;
          event->was_down = was_down;
          event->is_down  = is_down;
        }

        if (w_param == VK_LEFT)
        {
          event           = (OS_Event*)PushArena(os_event_arena, sizeof(OS_Event));
          event->type     = OS_EVENT_TYPE_KEYBOARD;
          event->key      = OS_KEY_ARROW_LEFT;
          event->was_down = was_down;
          event->is_down  = is_down;
        }

        if (w_param == VK_RIGHT)
        {
          event           = (OS_Event*)PushArena(os_event_arena, sizeof(OS_Event));
          event->type     = OS_EVENT_TYPE_KEYBOARD;
          event->key      = OS_KEY_ARROW_RIGHT;
          event->was_down = was_down;
          event->is_down  = is_down;
        }

        if (w_param == VK_RETURN && (l_param & (1 << 29)))
        {
          if (was_down && !is_down)
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
        event = (OS_Event*)PushArena(os_event_arena, sizeof(OS_Event));
        if (release)
        {
          event->type = OS_EVENT_TYPE_MOUSE_RELEASE;
        } 
        else
        {
          event->type = OS_EVENT_TYPE_MOUSE_PRESS;
        }
        event->mouse_position.x = LOWORD(l_param);
        event->mouse_position.y = HIWORD(l_param);

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
        result = DefWindowProcW(hwnd, message, w_param, l_param);
      } break;
  }

  if (event != 0)
  {
    OS_PushEvent(os_event_list, event);
  }

  return result;
}
