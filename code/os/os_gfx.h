#pragma once

#include "base/base_include.h"

typedef struct OS_Window OS_Window;
struct OS_Window {
  Vec2U32 size;
  Vec2F32 cursor_position;
  Vec2F32 virtual_cursor_position;
};

// -------------------------------------------------------------------
// -- Keyboard -------------------------------------------------------
typedef U16 OS_KeyCode;
typedef enum OS_KeyCodeEnum {
  OS_KEY_NONE,
  OS_KEY_ESC,
  OS_KEY_F1,
  OS_KEY_F2,
  OS_KEY_F3,
  OS_KEY_F4,
  OS_KEY_F5,
  OS_KEY_F6,
  OS_KEY_F7,
  OS_KEY_F8,
  OS_KEY_F9,
  OS_KEY_F10,
  OS_KEY_F11,
  OS_KEY_F12,
  OS_KEY_BACKTICK,
  OS_KEY_0,
  OS_KEY_1,
  OS_KEY_2,
  OS_KEY_3,
  OS_KEY_4,
  OS_KEY_5,
  OS_KEY_6,
  OS_KEY_7,
  OS_KEY_8,
  OS_KEY_9,
  OS_KEY_MINUS,
  OS_KEY_EQUAL,
  OS_KEY_BACKSPACE,
  OS_KEY_TAB,
  OS_KEY_Q,
  OS_KEY_W,
  OS_KEY_E,
  OS_KEY_R,
  OS_KEY_T,
  OS_KEY_Y,
  OS_KEY_U,
  OS_KEY_I,
  OS_KEY_O,
  OS_KEY_P,
  OS_KEY_LEFT_BRACKET,
  OS_KEY_RIGHT_BRACKET,
  OS_KEY_BACK_SLASH,
  OS_KEY_CAPS_LOCK,
  OS_KEY_A,
  OS_KEY_S,
  OS_KEY_D,
  OS_KEY_F,
  OS_KEY_G,
  OS_KEY_H,
  OS_KEY_J,
  OS_KEY_K,
  OS_KEY_L,
  OS_KEY_SEMICOLON,
  OS_KEY_QUOTE,
  OS_KEY_RETURN,
  OS_KEY_SHIFT,
  OS_KEY_Z,
  OS_KEY_X,
  OS_KEY_C,
  OS_KEY_V,
  OS_KEY_B,
  OS_KEY_N,
  OS_KEY_M,
  OS_KEY_COMMA,
  OS_KEY_PERIOD,
  OS_KEY_SLASH,
  OS_KEY_CTRL,
  OS_KEY_ALT,
  OS_KEY_SPACE,
  OS_KEY_ARROW_UP,
  OS_KEY_ARROW_DOWN,
  OS_KEY_ARROW_LEFT,
  OS_KEY_ARROW_RIGHT,
  OS_KEY_COUNT
} OS_KeyCodeEnum;

typedef struct OS_KeyState OS_KeyState;
struct OS_KeyState {
  B32 pressed;
};

typedef struct OS_Keyboard OS_Keyboard;
struct OS_Keyboard {
  OS_KeyState previous[OS_KEY_COUNT];
  OS_KeyState current[OS_KEY_COUNT];
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

func OS_Window* OS_CreateWindow(Str8 title, Vec2U32 size);
func void OS_DestroyWindow(OS_Window* window);
func void OS_ShowWindow(OS_Window* window);

func void OS_LockCursor(OS_Window* window);
func void OS_UnlockCursor(OS_Window* window);
func void OS_ShowCursor(B32 to_show);

func OS_EventList OS_DispatchEvents(OS_Window* window);

func F32 OS_GetMonitorHZ(void);
func U64 OS_GetTimeTicks(void);

func void OS_Sleep(U64 ms);
