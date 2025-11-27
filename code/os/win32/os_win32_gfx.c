#include "os_win32_gfx.h"
#include "os/os_gfx.h"

#define OS_WIN32_WindowClassName L"AppWindowClass"

static WINDOWPLACEMENT previous_window_params = { sizeof(previous_window_params) };
static LARGE_INTEGER win32_frequency;

func void
OS_Init(U64 arena_size)
{
  _os_state.arena = AllocateArena(arena_size);
  timeBeginPeriod(1);
  QueryPerformanceFrequency(&win32_frequency);
}

func void
OS_CreateWindow(Str8 title, Vec2U32 size, OS_Window* out)
{
  out->handle = (OS_WindowHandle*)PushArena(_os_state.arena, sizeof(OS_WindowHandle*));
  
  out->handle->instance = GetModuleHandle(0);

  WNDCLASSW window_class     = {0};
  window_class.style         = CS_HREDRAW | CS_VREDRAW;
  window_class.lpfnWndProc   = OS_WIN32_WindowProcedure;
  window_class.hInstance     = out->handle->instance;
  window_class.lpszClassName = OS_WIN32_WindowClassName;
  window_class.hCursor       = LoadCursor(0, IDC_ARROW);

  Assert(RegisterClassW(&window_class));

  HWND handle = {0};

  wchar_t wchar_title[256];
  MultiByteToWideChar(CP_ACP, 0, CFromStr8(title), -1, wchar_title, 256);
  handle = CreateWindowW(
    OS_WIN32_WindowClassName, wchar_title, WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT, size.w, size.h, 0, 0, out->handle->instance, 0
  );

  AssertMessage(handle, "Cannot create out->");

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

func OS_EventList
OS_GetEventList(Arena* arena, OS_Window* window)
{
  _os_state.event_list = OS_EventListCreate(arena);
	_os_state.keyboard_event_list = OS_EventListCreate(arena);
  _os_state.mouse_event_list = OS_EventListCreate(arena);

  MSG message;
  while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
  {
    TranslateMessage(&message);
    DispatchMessageW(&message);
  }

	for (I32 i = 0; i < OS_KEY_COUNT; i += 1)
	{
		OS_KeyState* key = _os_state.keyboard.keys + i;
		key->pressed = 0;
		key->released = 0;
		key->time_down += key->is_down * (0.0f); // AlNov: @TODO Add delta time
	}

  for (OS_EventListNode *event_node = _os_state.keyboard_event_list.first; event_node; event_node = event_node->next)
	{

		OS_Event* event = &event_node->data;
    if(event->key == OS_KEY_W)
    {
      LOG_DEBUG("Keyboard event W\n");
    }
		_os_state.keyboard.keys[event->key].pressed = event->pressed;
		_os_state.keyboard.keys[event->key].released = !event->pressed;
		if (event->pressed)
		{
			_os_state.keyboard.keys[event->key].is_down = 1;
		}
		if (event->released)
		{
			_os_state.keyboard.keys[event->key].is_down = 0;
		}
	}

  for (I32 i = 0; i < OS_MouseButton_Count; i += 1)
  {
    OS_MouseButtonState* button = _os_state.mouse.buttons + i;
    button->pressed = 0;
    button->released = 0;
  }

  for (OS_EventListNode* event_node = _os_state.mouse_event_list.first; event_node; event_node = event_node->next)
  {
    OS_Event* event = &event_node->data;
    _os_state.mouse.buttons[event->mouse_button].pressed = event->pressed;
    _os_state.mouse.buttons[event->mouse_button].released = event->released;
    if (event->pressed)
    {
      _os_state.mouse.buttons[event->mouse_button].is_down = 1;
    }
    if (event->released)
    {
      _os_state.mouse.buttons[event->mouse_button].is_down = 0;
    }
  }

  return _os_state.event_list;
}

func U64 OS_GetTimeTicks(void)
{
  LARGE_INTEGER counter;
  QueryPerformanceCounter(&counter);
  F64 seconds = (F64)(counter.QuadPart)/(F64)(win32_frequency.QuadPart);

  return (U64)(seconds * 1000);
}

func void
OS_Wait(F32 miliseconds)
{
  if (miliseconds <= 0) return;

  F32 begin_time_ms = OS_GetTimeTicks();
  F32 end_time_ms = begin_time_ms + miliseconds;

  Sleep(miliseconds);

  while (OS_GetTimeTicks() < end_time_ms) {}
}

func Vec2F32
OS_MousePosition(OS_Window window)
{
  POINT mouse_point;
  GetCursorPos(&mouse_point);
  ScreenToClient(window.handle->handle, &mouse_point);

  return MakeVec2F32((F32)mouse_point.x, (F32)mouse_point.y);
}

func B32
OS_IsWindowClosed(void)
{
  return 0;
}

func F32
OS_GetMonitorHZ(void)
{
  DEVMODEW dev_mode = {0};
  EnumDisplaySettingsW(0, ENUM_CURRENT_SETTINGS, &dev_mode);

  return (F32)dev_mode.dmDisplayFrequency;
}

func LRESULT
OS_WIN32_WindowProcedure(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param)
{
  OS_Event event = {0};
  LRESULT result  = 0;

  switch (message)
  {
    case WM_SIZE:
    {
      if (_os_state.event_list.arena == 0) break;
      
      event.type = OS_EVENT_TYPE_RESIZE;
      event.window_size.w = LOWORD(l_param);
      event.window_size.h = HIWORD(l_param);
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
        event.type = OS_EVENT_TYPE_KEYBOARD;
        event.pressed = !(l_param & (1 << 31));
        event.released = !event.pressed;
        LOG_DEBUG("PRES: %i REL: %i\n", event.pressed, event.released);

        switch (w_param)
        {
          default: {}; break;
          
          case VK_ESCAPE: {event.key = OS_KEY_ESC;};break;
          case VK_F1: {event.key = OS_KEY_F1;};break;
          case VK_F2: {event.key = OS_KEY_F2;};break;
          case VK_F3: {event.key = OS_KEY_F3;};break;
          case VK_F4: {event.key = OS_KEY_F4;};break;
          case VK_F5: {event.key = OS_KEY_F5;};break;
          case VK_F6: {event.key = OS_KEY_F6;};break;
          case VK_F7: {event.key = OS_KEY_F7;};break;
          case VK_F8: {event.key = OS_KEY_F8;};break;
          case VK_F9: {event.key = OS_KEY_F9;};break;
          case VK_F10: {event.key = OS_KEY_F10;};break;
          case VK_F11: {event.key = OS_KEY_F11;};break;
          case VK_F12: {event.key = OS_KEY_F12;};break;
          case VK_OEM_3: {event.key = OS_KEY_BACKTICK;};break;
          case '0': {event.key = OS_KEY_0;};break;
          case '1': {event.key = OS_KEY_1;};break;
          case '2': {event.key = OS_KEY_2;};break;
          case '3': {event.key = OS_KEY_3;};break;
          case '4': {event.key = OS_KEY_4;};break;
          case '5': {event.key = OS_KEY_5;};break;
          case '6': {event.key = OS_KEY_6;};break;
          case '7': {event.key = OS_KEY_7;};break;
          case '8': {event.key = OS_KEY_8;};break;
          case '9': {event.key = OS_KEY_9;};break;
          case VK_OEM_MINUS: {event.key = OS_KEY_MINUS;};break;
          case VK_OEM_PLUS: {event.key = OS_KEY_EQUAL;};break;
          case VK_BACK: {event.key = OS_KEY_BACKSPACE;};break;
          case VK_TAB: {event.key = OS_KEY_TAB;};break;
          case 'Q': {event.key = OS_KEY_Q;};break;
          case 'W': {event.key = OS_KEY_W; LOG_DEBUG("WIN32 W\n");};break;
          case 'E': {event.key = OS_KEY_E;};break;
          case 'R': {event.key = OS_KEY_R;};break;
          case 'T': {event.key = OS_KEY_T;};break;
          case 'Y': {event.key = OS_KEY_Y;};break;
          case 'U': {event.key = OS_KEY_U;};break;
          case 'I': {event.key = OS_KEY_I;};break;
          case 'O': {event.key = OS_KEY_O;};break;
          case 'P': {event.key = OS_KEY_P;};break;
          case VK_OEM_4: {event.key = OS_KEY_LEFT_BRACKET;};break;
          case VK_OEM_6: {event.key = OS_KEY_RIGHT_BRACKET;};break;
          case VK_OEM_5: {event.key = OS_KEY_BACK_SLASH;};break;
          case VK_CAPITAL: {event.key = OS_KEY_CAPS_LOCK;};break;
          case 'A': {event.key = OS_KEY_A;};break;
          case 'S': {event.key = OS_KEY_S;};break;
          case 'D': {event.key = OS_KEY_D;};break;
          case 'F': {event.key = OS_KEY_F;};break;
          case 'G': {event.key = OS_KEY_G;};break;
          case 'H': {event.key = OS_KEY_H;};break;
          case 'J': {event.key = OS_KEY_J;};break;
          case 'K': {event.key = OS_KEY_K;};break;
          case 'L': {event.key = OS_KEY_L;};break;
          case VK_OEM_1: {event.key = OS_KEY_SEMICOLON;};break;
          case VK_OEM_7: {event.key = OS_KEY_QUOTE;};break;
          case VK_RETURN: {event.key = OS_KEY_RETURN;};break;
          case VK_LSHIFT: {event.key = OS_KEY_SHIFT;};break;
          case VK_RSHIFT: {event.key = OS_KEY_SHIFT;};break;
          case 'Z': {event.key = OS_KEY_Z;};break;
          case 'X': {event.key = OS_KEY_X;};break;
          case 'C': {event.key = OS_KEY_C;};break;
          case 'V': {event.key = OS_KEY_V;};break;
          case 'B': {event.key = OS_KEY_B;};break;
          case 'N': {event.key = OS_KEY_N;};break;
          case 'M': {event.key = OS_KEY_M;};break;
          case VK_OEM_COMMA: {event.key = OS_KEY_COMMA;};break;
          case VK_OEM_PERIOD: {event.key = OS_KEY_PERIOD;};break;
          case VK_OEM_2: {event.key = OS_KEY_SLASH;};break;
          case VK_LCONTROL: {event.key = OS_KEY_CTRL;};break;
          case VK_RCONTROL: {event.key = OS_KEY_CTRL;};break;
          case VK_LMENU: {event.key = OS_KEY_ALT;};break;
          case VK_RMENU: {event.key = OS_KEY_ALT;};break;
          case VK_SPACE: {event.key = OS_KEY_SPACE;};break;
          case VK_UP: {event.key = OS_KEY_ARROW_UP;};break;
          case VK_DOWN: {event.key = OS_KEY_ARROW_DOWN;};break;
          case VK_LEFT: {event.key = OS_KEY_ARROW_LEFT;};break;
          case VK_RIGHT: {event.key = OS_KEY_ARROW_RIGHT;};break;
        };
      }

    // --AlNov: Mouse Input ------------------------------
    case WM_MOUSEMOVE:
    {
      
    } break;
    case WM_LBUTTONUP:
    {
      event.type = OS_EVENT_TYPE_MOUSE_PRESS;
      event.released = 1;
      event.mouse_button = OS_MouseButton_Left;
    } break;
    case WM_RBUTTONUP:
    {
      event.type = OS_EVENT_TYPE_MOUSE_PRESS;
      event.released = 1;
      event.mouse_button = OS_MouseButton_Right;
    } break;
    case WM_LBUTTONDOWN:
    {
      event.type = OS_EVENT_TYPE_MOUSE_PRESS;
      event.pressed = 1;
      event.mouse_button = OS_MouseButton_Left;
    } break;
    case WM_RBUTTONDOWN:
    {
      event.type = OS_EVENT_TYPE_MOUSE_PRESS;
      event.pressed = 1;
      event.mouse_button = OS_MouseButton_Right;
    } break;
    default:
    {
      result = DefWindowProcW(hwnd, message, w_param, l_param);
    } break;
  }

  if (event.type != 0)
  {
    if (event.type == OS_EVENT_TYPE_KEYBOARD)
    {
      OS_EventListPush(&_os_state.keyboard_event_list, event);
    }
    else if (event.type == OS_EVENT_TYPE_MOUSE_PRESS)
    {
      OS_EventListPush(&_os_state.mouse_event_list, event);
    }
    else
    {
      OS_EventListPush(&_os_state.event_list, event);
    }
  }

  return result;
}
