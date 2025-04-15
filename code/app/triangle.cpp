#include "base/base_core.h"
#include "base/base_include.h"
#include "os/os_include.h"
#include "render/r_include.h"

#include "base/base_include.cpp"
#include "os/os_include.cpp"
#include "render/r_include.cpp"
#include "render/r_gltf.h"

#include "render/r_gltf.cpp"

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
  GLTFReader gltf_reader = {};
  gltf_reader.file_buffer = ReadFile("data/box_gltf/test.gltf");
  ParseGLTF(&gltf_reader);

  GLTFElement* buffers_list_element = LookUpElement(gltf_reader.element, ConstString("buffers"));
  GLTFElement* buffer_element = buffers_list_element->first_sub_element->first_sub_element;
  Buffer mesh_buffer = buffer_element->value;
  
  U64 comma_position = FindPosition(mesh_buffer, ',');
  U64 quat_position = FindPosition(mesh_buffer, '"');
  mesh_buffer.data = mesh_buffer.data + comma_position + 1;
  mesh_buffer.size = mesh_buffer.size - comma_position - 1;
  Buffer decoded = Base64Decode(mesh_buffer);  
  
  R_Geometry geometry = {};
  geometry.index_data = decoded.data;
  geometry.index_size = sizeof(U16);
  geometry.index_count = 3;
  geometry.vertex_data = decoded.data + 8;
  geometry.vertex_size = sizeof(Vec3f);
  geometry.vertex_count = 3;
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
