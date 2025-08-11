#pragma once

#include "base/base_include.h"

typedef struct OS_WindowHandle OS_WindowHandle;

typedef U16 OS_WindowStatus;
typedef enum OS_WindowStatusEnum
{
  OS_WINDOW_STATUS_NONE,
  OS_WINDOW_STATUS_CREATED,
  OS_WINDOW_STATUS_OPEN,
  OS_WINDOW_STATUS_CLOSED,

  OS_WINDOW_STATUS_COUNT
} OS_WindowStatusEnum;

typedef struct OS_Window OS_Window;
struct OS_Window
{
    OS_WindowHandle* handle;
    Vec2U32 size;
    Vec2F32 cursor_position;
    Vec2F32 virtual_cursor_position;

    OS_WindowStatus status;
};

typedef U16 OS_EventType;
typedef enum OS_EventTypeEnum
{
    OS_EVENT_TYPE_NONE,

    OS_EVENT_TYPE_EXIT,
    OS_EVENT_TYPE_RESIZE,
    OS_EVENT_TYPE_MOUSE_MOVE,
    OS_EVENT_TYPE_MOUSE_RELEASE,
    OS_EVENT_TYPE_MOUSE_PRESS,
    OS_EVENT_TYPE_KEYBOARD,

    OS_EVENT_TYPE_COUNT
} OS_EventTypeEnum;

typedef U16 OS_KeyCode;
typedef enum OS_KeyCodeEnum
{
    OS_KEY_ARROW_UP,
    OS_KEY_ARROW_DOWN,
    OS_KEY_ARROW_LEFT,
    OS_KEY_ARROW_RIGHT,

    OS_KEY_COUNT
} OS_KeyCodeEnum;

typedef struct OS_Event OS_Event;
struct OS_Event
{
    OS_Event*     next;
    OS_Event*     previous;
    OS_EventType  type;

    Vec2U32 window_size;

    Vec2F32 mouse_position;
    
    OS_KeyCode key;
    
    B32  was_down;
    B32  is_down;

};
DefineList(OS_Event)

typedef struct OS_State OS_State;
struct OS_State
{
    Arena* arena;
    ListOS_Event event_list;
} _os_state;

func void OS_Init(U64 arena_size);

func void OS_CreateWindow(Str8 title, Vec2U32 size, OS_Window* out);
func void OS_DestroyWindow(OS_Window* window);
func void OS_ShowWindow(OS_Window* window);

func void OS_LockCursor(OS_Window* window);
func void OS_UnlockCursor(OS_Window* window);

func ListOS_Event OS_GetEventList(Arena* arena, OS_Window* window);

func F32 OS_GetMonitorHZ(void);

// --AlNov: @NOTE Convertion time getted from this function can be not as presice.
// The reason that we delete small number to large inside (tick and frequency).
func F32   OS_CurrentTimeSeconds(void);
func Vec2F32 OS_MousePosition(OS_Window window);
