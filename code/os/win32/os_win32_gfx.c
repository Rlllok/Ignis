#include "os_win32_gfx.h"
#include "os/os_gfx.h"

#define OS_WIN32_WindowClassName L"AppWindowClass"

static WINDOWPLACEMENT previous_window_params = { sizeof(previous_window_params) };
static LARGE_INTEGER win32_frequency;

func void
OS_Init(U64 arena_size) {
  // --AlNov 7 January 2026: @TODO Do we need arena_size?
  _os_state.arena = AllocateArena(Gigabytes(8), Kilobytes(64));
  _os_state.event_arena = AllocateArena(Megabytes(256), Kilobytes(4));
  timeBeginPeriod(1);
  QueryPerformanceFrequency(&win32_frequency);
}

func OS_Window*
OS_CreateWindow(Str8 title, Vec2U32 size) {
  OS_Win32_Window* result = (OS_Win32_Window*)PushArena(_os_state.arena, sizeof(OS_Win32_Window*));
  
  result->instance = GetModuleHandle(0);

  WNDCLASSW window_class     = {0};
  window_class.style         = CS_HREDRAW | CS_VREDRAW;
  window_class.lpfnWndProc   = OS_WIN32_WindowProcedure;
  window_class.hInstance     = result->instance;
  window_class.lpszClassName = OS_WIN32_WindowClassName;
  window_class.hCursor       = LoadCursor(0, IDC_ARROW);

  Assert(RegisterClassW(&window_class));

  HWND handle = {0};

  wchar_t wchar_title[256];
  MultiByteToWideChar(CP_ACP, 0, CFromStr8(title), -1, wchar_title, 256);
  handle = CreateWindowW(
    OS_WIN32_WindowClassName, wchar_title, WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT, size.w, size.h, 0, 0, result->instance, 0
  );

  AssertMessage(handle, "Cannot create out->");

  result->handle = handle;
  result->header.size = size;

  return (OS_Window*)result;
}

func void
OS_ShowWindow(OS_Window* window) {
  OS_Win32_Window* win32_window = (OS_Win32_Window*)window;
  ShowWindow(win32_window->handle, SW_SHOW);
  UpdateWindow(win32_window->handle);
}

func void
OS_LockCursor(OS_Window* window) {
  OS_Win32_Window* win32_window = (OS_Win32_Window*)window;

  RECT rect;
  GetWindowRect(win32_window->handle, &rect);
  ClipCursor(&rect);
}

func void
OS_UnlockCursor(OS_Window* window) {
}

func void
OS_ShowCursor(B32 to_show) {
  ShowCursor(0);
}

func void
OS_WIN32_ToggleFullscreen(HWND window_handle) {
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

func OS_EventList
OS_DispatchEvents(OS_Window* window) {
  ResetArena(_os_state.event_arena);

  _os_state.event_list.first = 0;
  _os_state.event_list.last = 0;

  memcpy(_os_state.keyboard.previous, _os_state.keyboard.current, sizeof(_os_state.keyboard.previous));

  MSG message;
  while (PeekMessageW(&message, 0, 0, 0, PM_REMOVE)) {
    TranslateMessage(&message);
    DispatchMessageW(&message);
  }

  return _os_state.event_list;
}

func U64 OS_GetTimeTicks(void) {
  LARGE_INTEGER counter;
  QueryPerformanceCounter(&counter);
  F64 seconds = (F64)(counter.QuadPart)/(F64)(win32_frequency.QuadPart);

  return (U64)(seconds * 1000);
}

func void
OS_Sleep(U64 ms) {
  Sleep(ms);
}

func void
OS_Wait(F32 miliseconds) {
  if (miliseconds <= 0) return;

  F32 begin_time_ms = OS_GetTimeTicks();
  F32 end_time_ms = begin_time_ms + miliseconds;

  Sleep(miliseconds);

  while (OS_GetTimeTicks() < end_time_ms) {}
}

func Vec2F32
OS_MousePosition(OS_Window* window) {
  OS_Win32_Window* win32_window = (OS_Win32_Window*)window;

  POINT mouse_point;
  GetCursorPos(&mouse_point);
  ScreenToClient(win32_window->handle, &mouse_point);

  return MakeVec2F32((F32)mouse_point.x, (F32)mouse_point.y);
}

func void
OS_SetMousePosition(OS_Window* window, Vec2F32 position) {
  OS_Win32_Window* win32_window = (OS_Win32_Window*)window;
  
  POINT point = {(I32)position.x, (I32)position.y};
  ClientToScreen(win32_window->handle, &point);
  SetCursorPos(point.x, point.y);
}

func Vec2F32
OS_MouseScroll() {
  return _os_state.mouse.scroll;
}

func B32
OS_IsWindowClosed(void) {
  return 0;
}

func F32
OS_GetMonitorHZ(void) {
  DEVMODEW dev_mode = {0};
  EnumDisplaySettingsW(0, ENUM_CURRENT_SETTINGS, &dev_mode);

  return (F32)dev_mode.dmDisplayFrequency;
}

func OS_KeyCode
OS_KeyCodeFromWin32(U32 win32_key) {
  OS_KeyCode result = OS_KEY_NONE;
  switch (win32_key) {
    default: {}; break;

    case VK_ESCAPE    : {result = OS_KEY_ESC;};break;
    case VK_F1        : {result = OS_KEY_F1;};break;
    case VK_F2        : {result = OS_KEY_F2;};break;
    case VK_F3        : {result = OS_KEY_F3;};break;
    case VK_F4        : {result = OS_KEY_F4;};break;
    case VK_F5        : {result = OS_KEY_F5;};break;
    case VK_F6        : {result = OS_KEY_F6;};break;
    case VK_F7        : {result = OS_KEY_F7;};break;
    case VK_F8        : {result = OS_KEY_F8;};break;
    case VK_F9        : {result = OS_KEY_F9;};break;
    case VK_F10       : {result = OS_KEY_F10;};break;
    case VK_F11       : {result = OS_KEY_F11;};break;
    case VK_F12       : {result = OS_KEY_F12;};break;
    case VK_OEM_3     : {result = OS_KEY_BACKTICK;};break;
    case '0'          : {result = OS_KEY_0;};break;
    case '1'          : {result = OS_KEY_1;};break;
    case '2'          : {result = OS_KEY_2;};break;
    case '3'          : {result = OS_KEY_3;};break;
    case '4'          : {result = OS_KEY_4;};break;
    case '5'          : {result = OS_KEY_5;};break;
    case '6'          : {result = OS_KEY_6;};break;
    case '7'          : {result = OS_KEY_7;};break;
    case '8'          : {result = OS_KEY_8;};break;
    case '9'          : {result = OS_KEY_9;};break;
    case VK_OEM_MINUS : {result = OS_KEY_MINUS;};break;
    case VK_OEM_PLUS  : {result = OS_KEY_EQUAL;};break;
    case VK_BACK      : {result = OS_KEY_BACKSPACE;};break;
    case VK_TAB       : {result = OS_KEY_TAB;};break;
    case 'Q'          : {result = OS_KEY_Q;};break;
    case 'W'          : {result = OS_KEY_W;};break;
    case 'E'          : {result = OS_KEY_E;};break;
    case 'R'          : {result = OS_KEY_R;};break;
    case 'T'          : {result = OS_KEY_T;};break;
    case 'Y'          : {result = OS_KEY_Y;};break;
    case 'U'          : {result = OS_KEY_U;};break;
    case 'I'          : {result = OS_KEY_I;};break;
    case 'O'          : {result = OS_KEY_O;};break;
    case 'P'          : {result = OS_KEY_P;};break;
    case VK_OEM_4     : {result = OS_KEY_LEFT_BRACKET;};break;
    case VK_OEM_6     : {result = OS_KEY_RIGHT_BRACKET;};break;
    case VK_OEM_5     : {result = OS_KEY_BACK_SLASH;};break;
    case VK_CAPITAL   : {result = OS_KEY_CAPS_LOCK;};break;
    case 'A'          : {result = OS_KEY_A;};break;
    case 'S'          : {result = OS_KEY_S;};break;
    case 'D'          : {result = OS_KEY_D;};break;
    case 'F'          : {result = OS_KEY_F;};break;
    case 'G'          : {result = OS_KEY_G;};break;
    case 'H'          : {result = OS_KEY_H;};break;
    case 'J'          : {result = OS_KEY_J;};break;
    case 'K'          : {result = OS_KEY_K;};break;
    case 'L'          : {result = OS_KEY_L;};break;
    case VK_OEM_1     : {result = OS_KEY_SEMICOLON;};break;
    case VK_OEM_7     : {result = OS_KEY_QUOTE;};break;
    case VK_RETURN    : {result = OS_KEY_RETURN;};break;
    case VK_LSHIFT    : {result = OS_KEY_SHIFT;};break;
    case VK_RSHIFT    : {result = OS_KEY_SHIFT;};break;
    case 'Z'          : {result = OS_KEY_Z;};break;
    case 'X'          : {result = OS_KEY_X;};break;
    case 'C'          : {result = OS_KEY_C;};break;
    case 'V'          : {result = OS_KEY_V;};break;
    case 'B'          : {result = OS_KEY_B;};break;
    case 'N'          : {result = OS_KEY_N;};break;
    case 'M'          : {result = OS_KEY_M;};break;
    case VK_OEM_COMMA : {result = OS_KEY_COMMA;};break;
    case VK_OEM_PERIOD: {result = OS_KEY_PERIOD;};break;
    case VK_OEM_2     : {result = OS_KEY_SLASH;};break;
    case VK_LCONTROL  : {result = OS_KEY_CTRL;};break;
    case VK_RCONTROL  : {result = OS_KEY_CTRL;};break;
    case VK_LMENU     : {result = OS_KEY_ALT;};break;
    case VK_RMENU     : {result = OS_KEY_ALT;};break;
    case VK_SPACE     : {result = OS_KEY_SPACE;};break;
    case VK_UP        : {result = OS_KEY_ARROW_UP;};break;
    case VK_DOWN      : {result = OS_KEY_ARROW_DOWN;};break;
    case VK_LEFT      : {result = OS_KEY_ARROW_LEFT;};break;
    case VK_RIGHT     : {result = OS_KEY_ARROW_RIGHT;};break;
  };
  return result;
}

func LRESULT
OS_WIN32_WindowProcedure(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param) {
  OS_Event event = {0};
  LRESULT result  = 0;

  switch (message) {
    case WM_SIZE: {
      event.kind = OS_EventKind_Resize;
      event.window_size.w = LOWORD(l_param);
      event.window_size.h = HIWORD(l_param);
    } break;
    
    case WM_SETFOCUS: {
      event.kind = OS_EventKind_Focus;
    } break;
    
    case WM_KILLFOCUS: {
      event.kind = OS_EventKind_Unfocus;
    } break;

    case WM_CLOSE:
    {
      DestroyWindow(hwnd);
    } break;

    case WM_DESTROY:
      {
        event.kind = OS_EventKind_Exit;
        PostQuitMessage(0);
      } break;

    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYUP: {
      OS_KeyCode key = OS_KeyCodeFromWin32(w_param);
      _os_state.keyboard.current[key].pressed = (message == WM_KEYDOWN || message == WM_SYSKEYDOWN);
    } break;

    // --AlNov: Mouse Input ------------------------------
    case WM_MOUSEMOVE:
    {
      
    } break;
    case WM_MOUSEWHEEL: {
      _os_state.mouse.scroll.y = GET_WHEEL_DELTA_WPARAM(w_param)/WHEEL_DELTA;
    } break;
    case WM_MOUSEHWHEEL: {
      _os_state.mouse.scroll.x = GET_WHEEL_DELTA_WPARAM(w_param)/WHEEL_DELTA;
    } break;
    case WM_LBUTTONUP: {
    } break;
    case WM_RBUTTONUP: {
    } break;
    case WM_LBUTTONDOWN: {
    } break;
    case WM_RBUTTONDOWN: {
    } break;
    default: {
      result = DefWindowProcW(hwnd, message, w_param, l_param);
    } break;
  }

  if (event.kind != 0) {
    OS_Event* new_event = (OS_Event*)PushArena(_os_state.event_arena, sizeof(new_event));
    *new_event = event;
    SllPushBack(_os_state.event_list.first, _os_state.event_list.last, new_event);
  }

  return result;
}
