#include "base/base_include.h"
#include "os/os_include.h"
#include "render/r_include.h"
#include "assets/animation.h"
#include "assets/mesh.h"
#include "ui/ui_include.h"
#include "draw/d_include.h"

#include "base/base_include.c"
#include "os/os_include.c"
#include "render/r_include.c"
#include "assets/animation.c"
#include "assets/mesh.c"
#include "ui/ui_include.c"
#include "draw/d_include.c"

#include "ignis.h"
#include "ignis_r.h"
#include "ignis_ui.h"

#include "ignis.c"
#include "ignis_r.c"
#include "ignis_ui.c"

typedef struct Ignis_State Ignis_State;
struct Ignis_State
{
  Arena* arena;

  OS_Window window;

  Ignis_Scene scene;

  B32 finished;
  F32 dt;
} _ignis_state;

func void Init_Ignis();
func void Ignis_HandleEvents(Arena* arena);

I32 main()
{
  Init_Ignis();

  U64 begin_ms = OS_GetTimeTicks();
  while (!_ignis_state.finished)
  {
    Ignis_HandleEvents(_ignis_state.arena);

    Ignis_UI_Configure(&_ignis_state.scene, MakeVec2I32(_ignis_state.window.size.x, _ignis_state.window.size.y), OS_MousePosition(_ignis_state.window), 0.0f);
    
    Ignis_R_BeginFrame();
    {
      Ignis_R_RenderScene(&_ignis_state.scene);
      Ignis_R_RenderUI(Ignis_UI_GetDrawCommands());
    }
    Ignis_R_EndFrame();

    U64 end_ms = OS_GetTimeTicks();
    F32 sleep_dt = _ignis_state.dt - (F32)(end_ms - begin_ms);
    if (sleep_dt > 0)
    {
      OS_Sleep((U64)(sleep_dt*1000.0f));
    }
  }

  return 0;
}

func void
Init_Ignis()
{
  _ignis_state = (Ignis_State){0};
  _ignis_state.arena = AllocateArena(Megabytes(32));
  _ignis_state.scene = Ignis_CreateScene(_ignis_state.arena, 64);
  _ignis_state.dt    = 1.0f/60.0f;

  Ignis_CreateEntity(&_ignis_state.scene, (Ignis_Entity){
    .name = Str8C("Camera"),
    .type = Ignis_EntityType_Camera,
    .transform.translation = MakeVec3F32(1.0f, 5.0f, 10.0f),
    .camera = {
      .front = MakeVec3(1.0f, 0.0f, -1.0f),
      .right = MakeVec3(1.0f, 0.0f, 1.0f),
      .up = MakeVec3(0.0f, 1.0f, 0.0f),
      .yaw = -90.0f,
      .pitch = -30.0f,
    },
  });

  AST_StaticMesh mesh = AST_LoadStaticMeshFromGLTF(_ignis_state.arena, Str8C("data/sphere_gltf/sphere.gltf"));
  // UpdateSkeletonGlobalTransform(&mesh.skeleton);
  Ignis_CreateEntity(&_ignis_state.scene, (Ignis_Entity){
    .name      = Str8C("Actor"),
    .type      = Ignis_EntityType_Actor,
    .transform = (Transform){
      .translation = MakeVec3F32(0.0f, 0.0f, 0.0f),
      .rotation    = IdentityQuaternion(),
      .scale       = MakeVec3F32(1.0f, 1.0f, 1.0f),
    },
    .actor.mesh          = mesh,
    .actor.color_texture = _ignis_r_state.default_color_texture,
  });

  OS_Init(Megabytes(32));
  OS_CreateWindow(Str8C("Ignis"), MakeVec2U32(1280, 720), &_ignis_state.window);
  OS_ShowWindow(&_ignis_state.window);

  Ignis_R_Init(R_RENDERER_TYPE_VK, &_ignis_state.window);
  Ignis_UI_Init(_ignis_state.arena, 1024);
}

func void
Ignis_HandleEvents(Arena* arena)
{
  OS_EventList event_list = OS_GetEventList(arena, &_ignis_state.window);

  if (OS_IsKeyPressed(OS_KEY_ESC))
  {
    _ignis_state.finished = 1;
  }

  Ignis_Entity* camera = Ignis_GetCamera(&_ignis_state.scene);

  Vec3F32 direction = MakeVec3(0.0f, 0.0f, 0.0f);
  F32 speed = 2.0f;

  if (OS_IsKeyDown(OS_KEY_W))
  {
    direction = AddVec3(direction, camera->camera.front);
  }
  if (OS_IsKeyDown(OS_KEY_S))
  {
    direction = SubVec3(direction, camera->camera.front);
  }
  if (OS_IsKeyDown(OS_KEY_D))
  {
    direction = AddVec3(direction, camera->camera.right);
  }
  if (OS_IsKeyDown(OS_KEY_A))
  {
    direction = SubVec3(direction, camera->camera.right);
  }
  camera->transform.translation= AddVec3(camera->transform.translation, ScaleVec3(NormalizeVec3(direction), speed*_ignis_state.dt));

  if (OS_IsKeyDown(OS_KEY_ARROW_LEFT))
  {
    camera->camera.yaw -= 25.0f*_ignis_state.dt;
  }
  if (OS_IsKeyDown(OS_KEY_ARROW_RIGHT))
  {
    camera->camera.yaw += 25.0f*_ignis_state.dt;
  }
  if (OS_IsKeyDown(OS_KEY_ARROW_UP))
  {
    camera->camera.pitch += 25.0f*_ignis_state.dt;
  }
  if (OS_IsKeyDown(OS_KEY_ARROW_DOWN))
  {
    camera->camera.pitch -= 25.0f*_ignis_state.dt;
  }
  Vec3 rotation = {0};
  rotation.x = cos(RadiansFromDegrees(camera->camera.yaw))*cos(RadiansFromDegrees(camera->camera.pitch));
  rotation.y = sin(RadiansFromDegrees(camera->camera.pitch));
  rotation.z = sin(RadiansFromDegrees(camera->camera.yaw))*cos(RadiansFromDegrees(camera->camera.pitch));
  camera->camera.front = rotation;
  camera->camera.right = NormalizeVec3(CrossVec3(camera->camera.front, MakeVec3(0.0f, 1.0f, 0.0f)));
  camera->camera.up = CrossVec3(camera->camera.right, camera->camera.front);

  for (OS_EventListNode *event_node = event_list.first; event_node; event_node = event_node->next)
  {
    OS_Event* event = &event_node->data;
    switch (event->type)
    {
      case OS_EVENT_TYPE_RESIZE:
      {
        if ((_ignis_state.window.size.w != event->window_size.w) || (_ignis_state.window.size.h != event->window_size.h))
        {
          _ignis_state.window.size = event->window_size;
          Ignis_R_Resize(MakeVec2I32(event->window_size.x, event->window_size.y));
        }
      }
    }
  }
}
