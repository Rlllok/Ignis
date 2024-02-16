#pragma once

#include "base/base_include.h"

struct OS_Window;

enum OS_EventType
{
    OS_EVENT_TYPE_NONE,

    OS_EVENT_TYPE_EXIT,
    OS_EVENT_TYPE_RESIZE,
    OS_EVENT_TYPE_MOUSE_MOVE,
    OS_EVENT_TYPE_MOUSE_RELEASE,
    OS_EVENT_TYPE_MOUSE_PRESS,

    OS_EVENT_TYPE_COUNT
};

struct OS_Event
{
    OS_Event* next;
    OS_Event* previous;
    OS_EventType type;

    Vec2u windowSize;

    u32 mouseX;
    u32 mouseY;
    bool isDown;
};

struct OS_EventList
{
    OS_Event* firstEvent;
    OS_Event* lastEvent;

    u32 eventCount;
};


OS_Window OS_CreateWindow(const char* title, Vec2u size);
void OS_ShowWindow(OS_Window* window);

OS_EventList OS_GetEventList(Arena* arena);
void OS_PushEvent(OS_EventList* eventList, OS_Event* event);

f32 OS_GetMonitorHZ();

// --AlNov: @NOTE Convertion time getted from this function can be not as presice.
// The reason that we delete small number to large inside (tick and frequency).
f32 OS_CurrentTimeSeconds();
Vec2f OS_MousePosition(OS_Window window);

// --AlNov: It is there because Vulkan neadds windows.h information
// But it could be removed if above problem is fixed.
#include "win32/os_gfx_win32.h"