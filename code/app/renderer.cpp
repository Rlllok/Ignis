#include "base/base_include.h" // @TODO Base layer doesn't work without os (Memory related)
#include "os/os_include.h"
#include "render/r_include.h"
#include "draw/d_include.h"

#include "base/base_include.cpp"
#include "os/os_include.cpp"
#include "render/r_include.cpp"
#include "draw/d_include.cpp"

struct AppState
{
  Arena* arena;

  OS_Window window;
  B32 is_window_closed;
};

func void HandleOSEvents(AppState* app_state);

I32 main()
{
  AppState app_state = {};
  app_state.arena = AllocateArena(Megabytes(64));
  app_state.window = OS_CreateWindow("Renderer", MakeVec2u(1280, 720));
  app_state.is_window_closed = false;

  R_InitBackend(&app_state.window);

  OS_ShowWindow(&app_state.window);

  while (!app_state.is_window_closed)
  {
    HandleOSEvents(&app_state);
  }

  Renderer.Destroy();

  return 0;
}

func void
HandleOSEvents(AppState* app_state)
{
  OS_EventList event_list = OS_GetEventList(app_state->arena);

  for (OS_Event* event = event_list.first;
       event;
       event = event->next)
  {
    switch(event->type)
    {
      case OS_EVENT_TYPE_EXIT:
      {
        app_state->is_window_closed = true;
      } break;

      default: break;
    }
  }
}
