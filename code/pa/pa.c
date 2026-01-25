#include "pa.h"

// -- Area -----------------------------------------------------------
func void
PA_StartArea(PA_Area* area)
{
  I32 x = 3;
  I32 y = 3;
  I32 layers = 2;

  // -- Alnov 3 January 2026: @TODO
  // DestroyScene pair? Or InitScene, not Create.
  area->scene = Ignis_CreateScene(_pa_state.arena, x*y*layers);

  Ignis_CreateEntity(&area->scene, (Ignis_Entity){
    .name = Str8C("PA_Camera"),
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

  for (I32 i = 0; i < x; i += 1)
  {
    for (I32 j = 0; j < y; j += 1)
    {
      char entity_name[128] = {0};
      sprintf(entity_name, "Grid_x%d_y%d", i, j);
      Str8 entity_str8 = CopyStr8(_pa_state.arena, Str8C(entity_name));
      Ignis_CreateEntity(&area->scene, (Ignis_Entity){
        .name = entity_str8,
        .type = Ignis_EntityType_Actor,
        .transform = (Transform){
          .translation = MakeVec3F32(i*_pa_state.grid_size, 0.0f, j*_pa_state.grid_size),
          .rotation = IdentityQuaternion(),
          .scale = MakeVec3F32(0.95f, 0.95f, 0.95f),
        },
        .actor = {
          .mesh = _ignis_state.primitives.cube,
        },
      });
    }
  }
}

func void
PA_EndArea(PA_Area* area)
{
  ResetArena(_pa_state.arena);
}

// -- State ----------------------------------------------------------
func void
PA_Init()
{
  _pa_state = (PA_State)ZeroStruct();

  _pa_state.arena = AllocateArena(Gigabytes(32), Kilobytes(64));

  _pa_state.grid_size = 1.0f;

  PA_StartArea(&_pa_state.area);
}

func void
PA_Input (F32 dt)
{
  // -- Camera Input
  {
    Ignis_Entity* camera = Ignis_GetCamera(&_pa_state.area.scene);

    Vec3F32 direction = MakeVec3(0.0f, 0.0f, 0.0f);
    F32 speed = 2.0f;

    if (OS_KeyDown(OS_KEY_W))
    {
      direction = AddVec3(direction, camera->camera.front);
    }
    if (OS_KeyDown(OS_KEY_S))
    {
      direction = SubVec3(direction, camera->camera.front);
    }
    if (OS_KeyDown(OS_KEY_D))
    {
      direction = AddVec3(direction, camera->camera.right);
    }
    if (OS_KeyDown(OS_KEY_A))
    {
      direction = SubVec3(direction, camera->camera.right);
    }
    camera->transform.translation= AddVec3(camera->transform.translation, ScaleVec3(NormalizeVec3(direction), speed*dt));

    if (OS_KeyDown(OS_KEY_ARROW_LEFT))
    {
      camera->camera.yaw -= 25.0f*dt;
    }
    if (OS_KeyDown(OS_KEY_ARROW_RIGHT))
    {
      camera->camera.yaw += 25.0f*dt;
    }
    if (OS_KeyDown(OS_KEY_ARROW_UP))
    {
      camera->camera.pitch += 25.0f*dt;
    }
    if (OS_KeyDown(OS_KEY_ARROW_DOWN))
    {
      camera->camera.pitch -= 25.0f*dt;
    }
    Vec3 rotation = {0};
    rotation.x = cos(RadiansFromDegrees(camera->camera.yaw))*cos(RadiansFromDegrees(camera->camera.pitch));
    rotation.y = sin(RadiansFromDegrees(camera->camera.pitch));
    rotation.z = sin(RadiansFromDegrees(camera->camera.yaw))*cos(RadiansFromDegrees(camera->camera.pitch));
    camera->camera.front = rotation;
    camera->camera.right = NormalizeVec3(CrossVec3(camera->camera.front, MakeVec3(0.0f, 1.0f, 0.0f)));
    camera->camera.up = CrossVec3(camera->camera.right, camera->camera.front);
  }
}

func void
PA_Update(F32 dt)
{
  PA_Input(dt);
}
