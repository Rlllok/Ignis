#include "base/base_core.h"
#include "base/base_include.h"
#include "os/os_include.h"
#include "render/r_include.h"
#include "assets/mesh.h"

#include "base/base_include.cpp"
#include "os/os_include.cpp"
#include "render/r_include.cpp"
#include "assets/mesh.cpp"

struct AppState
{
  Arena* arena;
  OS_Window window;

  B32 is_window_closed;
} app_state;

func void HandleEvents(AppState* state);

I32 main()
{
  app_state = {};
  app_state.arena = AllocateArena(Megabytes(64));
  app_state.is_window_closed = false;
 
  app_state.window = OS_CreateWindow("Vulkan Triangle", MakeVec2u(1270, 720));
  OS_ShowWindow(&app_state.window);

  R_Init(R_RENDERER_TYPE_VULKAN, &app_state.window);

  // @TODO Create Pipeline
  
  // @TODO @NOTE Hardcoded gltf information in R_Geometry
  AST_Geometry geometry = AST_LoadGeometryFromGLTF("data/box_gltf/test.gltf");
  Renderer.PushGeometry(&geometry);
  
  while (!app_state.is_window_closed)
  {
    HandleEvents(&app_state);

    Renderer.DrawGeometry(&geometry);
  }

  R_Shutdown();
  
  return 0;
}

func void
HandleEvents(AppState* state)
{
  OS_EventList event_list = OS_GetEventList(app_state.arena);
  
  for (OS_Event *event = event_list.first; event; event = event->next)
  {
    switch (event->type)
    {
      case OS_EVENT_TYPE_EXIT:
      {
        state->is_window_closed = true;
      } break;
      
      default: break;
    }
  }
}
