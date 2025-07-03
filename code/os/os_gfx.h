#pragma once

#include "base/base_include.h"

struct OS_WindowHandle;

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
    OS_WindowHandle* handle;
    Vec2u size;
    Vec2f cursor_position;
    Vec2f virtual_cursor_position;

    OS_WindowStatus status;
};

enum OS_EventType
{
    OS_EVENT_TYPE_NONE,

    OS_EVENT_TYPE_EXIT,
    OS_EVENT_TYPE_RESIZE,
    OS_EVENT_TYPE_MOUSE_MOVE,
    OS_EVENT_TYPE_MOUSE_RELEASE,
    OS_EVENT_TYPE_MOUSE_PRESS,
    OS_EVENT_TYPE_KEYBOARD,

    OS_EVENT_TYPE_COUNT
};

enum OS_KeyCode
{
    OS_KEY_ARROW_UP,
    OS_KEY_ARROW_DOWN,
    OS_KEY_ARROW_LEFT,
    OS_KEY_ARROW_RIGHT,

    OS_KEY_COUNT
};

struct OS_Event
{
    OS_Event*     next;
    OS_Event*     previous;
    OS_EventType  type;

    Vec2u window_size;

    Vec2f mouse_position;
    
    OS_KeyCode key;
    
    bool  was_down;
    bool  is_down;

};
DefineList(OS_Event)

struct _OS_State
{
    Arena* arena;
    ListOS_Event event_list;
} _os_state;

func void OS_Init(U64 arena_size);

func void OS_CreateWindow(const char* title, Vec2u size, OS_Window* out);
func void OS_DestroyWindow(OS_Window* window);
func void OS_ShowWindow(OS_Window* window);

func void OS_LockCursor(OS_Window* window);
func void OS_UnlockCursor(OS_Window* window);

func ListOS_Event OS_GetEventList(Arena* arena, OS_Window* window);

func F32 OS_GetMonitorHZ();

// --AlNov: @NOTE Convertion time getted from this function can be not as presice.
// The reason that we delete small number to large inside (tick and frequency).
func F32   OS_CurrentTimeSeconds();
func Vec2f OS_MousePosition(OS_Window window);
