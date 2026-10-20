#pragma once

#include "base/base_include.h"

// -------------------------------------------------------------------
// -- Window ---------------------------------------------------------
typedef struct OS_Window OS_Window;
struct OS_Window {
  Vec2U32 size;
  Vec2F32 cursor_position;
  Vec2F32 virtual_cursor_position;
};

func OS_Window* OS_CreateWindow(Str8 title, Vec2U32 size);
func void OS_DestroyWindow(OS_Window* window);
func void OS_ShowWindow(OS_Window* window);

// -------------------------------------------------------------------
// -- Keyboard -------------------------------------------------------
typedef U16 OS_KeyCode;
typedef enum OS_KeyCodeEnum {
  OS_Key_None,
  OS_Key_Esc,
  OS_Key_F1,
  OS_Key_F2,
  OS_Key_F3,
  OS_Key_F4,
  OS_Key_F5,
  OS_Key_F6,
  OS_Key_F7,
  OS_Key_F8,
  OS_Key_F9,
  OS_Key_F10,
  OS_Key_F11,
  OS_Key_F12,
  OS_Key_Backtick,
  OS_Key_0,
  OS_Key_1,
  OS_Key_2,
  OS_Key_3,
  OS_Key_4,
  OS_Key_5,
  OS_Key_6,
  OS_Key_7,
  OS_Key_8,
  OS_Key_9,
  OS_Key_Minus,
  OS_Key_Equal,
  OS_Key_Backspace,
  OS_Key_Tab,
  OS_Key_Q,
  OS_Key_W,
  OS_Key_E,
  OS_Key_R,
  OS_Key_T,
  OS_Key_Y,
  OS_Key_U,
  OS_Key_I,
  OS_Key_O,
  OS_Key_P,
  OS_Key_LeftBracket,
  OS_Key_RightBracket,
  OS_Key_BackSlash,
  OS_Key_CapsLock,
  OS_Key_A,
  OS_Key_S,
  OS_Key_D,
  OS_Key_F,
  OS_Key_G,
  OS_Key_H,
  OS_Key_J,
  OS_Key_K,
  OS_Key_L,
  OS_Key_Semicolon,
  OS_Key_Quote,
  OS_Key_Return,
  OS_Key_Shift,
  OS_Key_Z,
  OS_Key_X,
  OS_Key_C,
  OS_Key_V,
  OS_Key_B,
  OS_Key_N,
  OS_Key_M,
  OS_Key_Comma,
  OS_Key_Period,
  OS_Key_Slash,
  OS_Key_Ctrl,
  OS_Key_Alt,
  OS_Key_Space,
  OS_Key_ArrowUp,
  OS_Key_ArrowDown,
  OS_Key_ArrowLeft,
  OS_KEY_ARROW_RIGHT,
  OS_Key_ArrowRight,
  OS_Key_Count,
} OS_KeyCodeEnum;

typedef struct OS_KeyState OS_KeyState;
struct OS_KeyState {
  B32 pressed;
};

typedef struct OS_Keyboard OS_Keyboard;
struct OS_Keyboard {
  OS_KeyState previous[OS_Key_Count];
  OS_KeyState current[OS_Key_Count];
};

func B32 OS_KeyPressed(OS_KeyCode key);
func B32 OS_KeyReleased(OS_KeyCode key);
func B32 OS_KeyDown(OS_KeyCode key);
func B32 OS_KeyUp(OS_KeyCode key);

// -------------------------------------------------------------------
// -- Mouse ----------------------------------------------------------
typedef U8 OS_MouseButtonCode;
enum OS_MouseButtonEnum {
  OS_MouseButton_None,
  OS_MouseButton_Left,
  OS_MouseButton_Right,
  OS_MouseButton_Count,
} OS_MouseButtonEnum;

typedef struct OS_MouseButtonState OS_MouseButtonState;
struct OS_MouseButtonState {
  B32 pressed;
};

typedef struct OS_Mouse OS_Mouse;
struct OS_Mouse
{
  Vec2F32 position;
  Vec2F32 scroll;
  OS_MouseButtonState previous[OS_MouseButton_Count];
  OS_MouseButtonState current[OS_MouseButton_Count];
};

func Vec2F32 OS_MousePosition(OS_Window* window);
func void    OS_SetMousePosition(OS_Window* window, Vec2F32 position);
func Vec2F32 OS_MouseScroll();
func B32 OS_MousePressed(OS_MouseButtonCode button);
func B32 OS_MouseReleased(OS_MouseButtonCode button);
func B32 OS_MouseDown(OS_MouseButtonCode button);
func B32 OS_MouseUp(OS_MouseButtonCode button);

// -------------------------------------------------------------------
// Events
typedef U16 OS_EventKind;
typedef enum OS_EventKindEnum
{
    OS_EventKind_None,

    OS_EventKind_Exit,
    OS_EventKind_Resize,
    OS_EventKind_Focus,
    OS_EventKind_Unfocus,

    OS_EventKind_Count
} OS_EventTypeEnum;

typedef struct OS_Event OS_Event;
struct OS_Event {
  OS_EventKind kind;
  union {
    Vec2U32 window_size;
    Vec2F32 mouse_position;
    Vec2F32 mouse_scroll;
  };

  OS_Event* next;
};

typedef struct OS_EventList OS_EventList;
struct OS_EventList {
  OS_Event* first;
  OS_Event* last;
};

typedef struct OS_State OS_State;
struct OS_State {
  Arena*       arena;
  Arena*       event_arena;
  OS_EventList event_list;
  OS_Keyboard  keyboard;
  OS_Mouse     mouse;
} _os_state;

func void OS_Init(U64 arena_size);

func void OS_LockCursor(OS_Window* window);
func void OS_UnlockCursor(OS_Window* window);
func void OS_ShowCursor(B32 to_show);

func OS_EventList OS_DispatchEvents(OS_Window* window);

func F32 OS_GetMonitorHZ(void);
func U64 OS_GetTimeTicks(void);

func void OS_Sleep(U64 ms);
