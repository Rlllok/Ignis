#include "base/base_include.h"
#include "os/os_include.h"
#include "render/r_include.h"
#include "draw/d_include.h"

#include "base/base_include.cpp"
#include "os/os_include.cpp"
#include "render/r_include.cpp"
#include "draw/d_include.cpp"

I32 main()
{
  Arena* arena = AllocateArena(Megabytes(64));

  OS_Window window = OS_CreateWindow("PhysicsApp", MakeVec2u(1280, 720));
  R_Init(&window);
  D_Init(arena);

  OS_ShowWindow(&window);

  B32 is_window_closed = false;
  while(!is_window_closed)
  {
    OS_EventList event_list = OS_GetEventList(arena);
    
    for (OS_Event* event = event_list.first;
         event;
         event = event->next)
    {
      switch(event->type)
      {
        case OS_EVENT_TYPE_EXIT:
        {
          is_window_closed = true;
        } break;

        default: break;
      }
    }

    R_FrameInfo frame_info = {};
    Renderer.BeginFrame();
    {
      Vec4f clear_color = MakeVec4f(0.2f, 0.2f, 0.2f, 1.0f);
      F32 depth_clear = 1.0f;
      F32 stencil_clear = 0.0f;
      Renderer.BeginRenderPass(clear_color, depth_clear, stencil_clear);
      {
        for (I32 i = 0; i < 2; i += 1)
        {
          D_DrawBox(
            MakeVec2f(0.0f + 300.0f*i, 0.0 + 300.0f*i),
            MakeVec2f(25.0f, 25.0f),
            MakeVec3f(1.0f, 0.5f, 0.0f)
          );
        }

        D_DrawCircle(MakeVec2f(400.0f, 400.0f), 40.0f, MakeVec3f(0.0f, 0.3f, 0.2f));
      }
      Renderer.EndRenderPass();
    }
    Renderer.EndFrame();
  }
}
