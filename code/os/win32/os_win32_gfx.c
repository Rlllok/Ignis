#include "os_win32_gfx.h"
#include "os/os_gfx.h"

#define OS_WIN32_WindowClassName L"AppWindowClass"

static WINDOWPLACEMENT previous_window_params = { sizeof(previous_window_params) };

func void
OS_Init(U64 arena_size)
{
  _os_state.arena = AllocateArena(arena_size);
}

func void
OS_CreateWindow(const char* title, Vec2u size, OS_Window* out)
{
  out->handle = (OS_WindowHandle*)PushArena(_os_state.arena, sizeof(OS_WindowHandle*));
  
  out->handle->instance = GetModuleHandle(nullptr);

  WNDCLASSW window_class     = {};
  window_class.style         = CS_HREDRAW | CS_VREDRAW;
  window_class.lpfnWndProc   = OS_WIN32_WindowProcedure;
  window_class.hInstance     = out->handle->instance;
  window_class.lpszClassName = OS_WIN32_WindowClassName;
  window_class.hCursor       = LoadCursor(0, IDC_ARROW);

  Assert(RegisterClassW(&window_class) == 0);

  HWND handle = {};

  wchar_t wchar_title[256];
  MultiByteToWideChar(CP_ACP, 0, title, -1, wchar_title, 256);
  handle = CreateWindowW(
    OS_WIN32_WindowClassName, wchar_title, WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT, size.width, size.height, 0, 0, out->handle->instance, 0
  );

  AssertMessage(handle == 0, "Cannot create out->");

  out->handle->handle = handle;
  out->size = size;
  out->status = OS_WINDOW_STATUS_CREATED;
}

func void
OS_ShowWindow(OS_Window* window)
{
  ShowWindow(window->handle->handle, SW_SHOW);
  UpdateWindow(window->handle->handle);

  window->status = OS_WINDOW_STATUS_OPEN;
}

func void
OS_WIN32_ToggleFullscreen(HWND window_handle)
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

func ListOS_Event
OS_GetEventList(Arena* arena, OS_Window* window)
{
  _os_state.event_list = CreateListOS_Event(arena);
  
  MSG message;
  while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
  {
    TranslateMessage(&message);
    DispatchMessageW(&message);
  }

  return _os_state.event_list;
}

func F32
OS_CurrentTimeSeconds()
{
  // --AlNov: @NOTE Frequency should be computed only ones, as it doens't change after system start.
  local_persist LARGE_INTEGER frequency = {};
  if (!frequency.QuadPart)
  {
    QueryPerformanceFrequency(&frequency);
  }

  LARGE_INTEGER counter;
  QueryPerformanceCounter(&counter);

  return ((F64)counter.QuadPart) / ((F64)frequency.QuadPart);
}

func void
OS_Wait(F32 wait_seconds)
{
  if (wait_seconds <= 0) return;

  F32 begin_time = OS_CurrentTimeSeconds();
  F32 end_time = begin_time + wait_seconds;

  Sleep(wait_seconds * 500);

  while (OS_CurrentTimeSeconds() < end_time) {}
}

func Vec2f
OS_MousePosition(OS_Window window)
{
  POINT mouse_point;
  GetCursorPos(&mouse_point);
  ScreenToClient(window.handle->handle, &mouse_point);

  return MakeVec2f((F32)mouse_point.x, (F32)mouse_point.y);
}

func bool
OS_IsWindowClosed()
{
  return false;
}

func F32
OS_GetMonitorHZ()
{
  DEVMODEW dev_mode = {};
  EnumDisplaySettingsW(0, ENUM_CURRENT_SETTINGS, &dev_mode);

  return (F32)dev_mode.dmDisplayFrequency;
}

func LRESULT
OS_WIN32_WindowProcedure(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param)
{
  OS_Event event = {};
  LRESULT result  = 0;
  B32 release = false;

  switch (message)
  {
    case WM_SIZE:
    {
      if (_os_state.event_list.arena == 0) break;
      
      event.type = OS_EVENT_TYPE_RESIZE;
      event.window_size.width = LOWORD(l_param);
      event.window_size.height = HIWORD(l_param);
    } break;

    case WM_CLOSE:
    {
      DestroyWindow(hwnd);
    } break;

    case WM_DESTROY:
      {
        event.type = OS_EVENT_TYPE_EXIT;
        
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
          event.type = OS_EVENT_TYPE_KEYBOARD;
          event.key = OS_KEY_ARROW_UP;
          event.was_down = was_down;
          event.is_down = is_down;
        }

        if (w_param == VK_DOWN)
        {
          event.type = OS_EVENT_TYPE_KEYBOARD;
          event.key = OS_KEY_ARROW_DOWN;
          event.was_down = was_down;
          event.is_down = is_down;
        }

        if (w_param == VK_LEFT)
        {
          event.type = OS_EVENT_TYPE_KEYBOARD;
          event.key = OS_KEY_ARROW_LEFT;
          event.was_down = was_down;
          event.is_down = is_down;
        }

        if (w_param == VK_RIGHT)
        {
          event.type = OS_EVENT_TYPE_KEYBOARD;
          event.key = OS_KEY_ARROW_RIGHT;
          event.was_down = was_down;
          event.is_down = is_down;
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
        if (release)
        {
          event.type = OS_EVENT_TYPE_MOUSE_RELEASE;
        } 
        else
        {
          event.type = OS_EVENT_TYPE_MOUSE_PRESS;
        }
        event.mouse_position.x = LOWORD(l_param);
        event.mouse_position.y = HIWORD(l_param);

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

  if (event.type != 0)
  {
    PushListOS_Event(&_os_state.event_list, event);
  }

  return result;
}
