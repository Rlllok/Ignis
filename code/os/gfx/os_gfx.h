#pragma once

#include "../../base/base_include.h"

struct OS_Window;

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

    Vec2u mouse_position;
    bool  was_down;
    bool  is_down;

    OS_KeyCode key;
};

struct OS_EventList
{
    OS_Event* first;
    OS_Event* last;

    U32 count;
};


func OS_Window OS_CreateWindow(const char* title, Vec2u size);
func void      OS_ShowWindow(OS_Window* window);

func OS_EventList OS_GetEventList(Arena* arena);
func void         OS_PushEvent(OS_EventList* event_list, OS_Event* event);

func F32 OS_GetMonitorHZ();

// --AlNov: @NOTE Convertion time getted from this function can be not as presice.
// The reason that we delete small number to large inside (tick and frequency).
func F32   OS_CurrentTimeSeconds();
func Vec2f OS_MousePosition(OS_Window window);

// --AlNov: @TODO It is there because Vulkan neadds windows.h information
// But it could be removed if above problem is fixed.
#include "win32/os_gfx_win32.h"
