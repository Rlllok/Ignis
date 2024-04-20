#pragma comment(lib, "Winmm.lib")

// --AlNov: .h -------------------------------------------------------
#include "../base/base_include.h"
#include "../os/os_include.h"
#include "../render/vulkan/r_init_vk.h"
#include "../physics/ph_core.h"

// --AlNov: .cpp -----------------------------------------------------
#include "../base/base_include.cpp"
#include "../os/os_include.cpp"
#include "../render/vulkan/r_init_vk.cpp"
#include "../physics/ph_core.cpp"

// --AlNov: TMP Globals ----------------------------------------------
global f32 pixels_per_meter = 20.0f;

func void DrawBox(Arena* arena, Vec2f position, f32 width, f32 height, f32 angle);
func void DrawCircle(Arena* arena, Vec2f position, f32 radius, f32 angle);
func void DrawPhysicsShape(Arena* arena, PH_Shape* shape);
func void DrawLine(Arena* arena, Vec3f p0, Vec3f p1);

i32 main()
{
  OS_Window window = OS_CreateWindow("Physics", MakeVec2u(1280, 720));
  R_VK_Init(&window);

  // --AlNov: change Windows schedular granuality
  // Should be added to win32 layer
  u32 desired_schedular_ms = 1;
  timeBeginPeriod(desired_schedular_ms);

  OS_ShowWindow(&window);

  LARGE_INTEGER win32_freq;
  QueryPerformanceFrequency(&win32_freq);
  u64 frequency = win32_freq.QuadPart;

  LARGE_INTEGER win32_cycles;
  QueryPerformanceCounter(&win32_cycles);
  u64 start_cycles = win32_cycles.QuadPart;

  Arena* frame_arena = AllocateArena(Megabytes(256));

  f32 time_sec = 0.0f;

  Arena* shape_arena = AllocateArena(Megabytes(64));

  PH_ShapeList shape_list;
  PH_Shape* circle_a = PH_CreateCircleShape(shape_arena, MakeVec2f(200.0f, 400.0f), 0.03f, 1.0f);
  PH_PushShapeList(&shape_list, circle_a);
  PH_Shape* circle_b = PH_CreateCircleShape(shape_arena, MakeVec2f(400.0f, 400.0f), 0.08f, 1.0f);
  PH_PushShapeList(&shape_list, circle_b);

  Vec2f anchor_position = MakeVec2f(200.0f, 200.0f);

  Vec2f wind_force = {};
  f32 wind_force_magnitude = 100.0f * pixels_per_meter;

  bool b_finished = false;
  while (!b_finished)
  {
    OS_EventList event_list = OS_GetEventList(frame_arena);

    OS_Event* current_event = event_list.firstEvent;
    while (current_event)
    {
      switch (current_event->type)
      {
        case OS_EVENT_TYPE_EXIT:
        {
          b_finished = true;
        } break;

        case OS_EVENT_TYPE_KEYBOARD:
        {
          if (current_event->key == OS_KEY_ARROW_UP)
          {
            wind_force = MakeVec2f(0.0f, -1 * wind_force_magnitude);
          }
          if (current_event->key == OS_KEY_ARROW_DOWN)
          {
            wind_force = MakeVec2f(0.0f, wind_force_magnitude);
          }
          if (current_event->key == OS_KEY_ARROW_LEFT)
          {
            wind_force = MakeVec2f(-1 * wind_force_magnitude, 0.0f);
          }
          if (current_event->key == OS_KEY_ARROW_RIGHT)
          {
            wind_force = MakeVec2f(wind_force_magnitude, 0.0f);
          }
        } break;

        case OS_EVENT_TYPE_MOUSE_RELEASE:
        {
          // Vec2f particle_position = MakeVec2f(current_event->mouseX, current_event->mouseY);
          // PH_Particle* particle = PH_CreateParticle(particle_arena, particle_position, 1.0f);
          // PH_PushParticle(&particle_list, particle);
        }

        default: break;
      }

      current_event = current_event->next;
    }

    // --AlNov: Update Physics
    for (PH_Shape* shape = shape_list.first;
         shape;
         shape = shape->next)
    {
      Vec2f weight = PH_CalculateWeight(shape->mass, 9.8 * pixels_per_meter);
      PH_ApplyForceToShape(shape, weight);
      
      Vec2f wind = MakeVec2f(50.0f, 0.0f);
      PH_ApplyForceToShape(shape, wind);

      PH_IntegrateShape(shape, time_sec);

      if (shape->type == PH_SHAPE_TYPE_CIRCLE)
      {
        if (shape->position.y + shape->circle.radius > window.height)
        {
          shape->position.y = window.height - shape->circle.radius;
          shape->velocity.y *= -0.9f;
        }
        else if (shape->position.y - shape->circle.radius < 0)
        {
          shape->position.y = shape->circle.radius;
          shape->velocity.y *= -0.9f;
        }

        if (shape->position.x + shape->circle.radius > window.width)
        {
          shape->position.x = window.width - shape->circle.radius;
          shape->velocity.x *= -0.9f;
        }
        else if (shape->position.x - shape->circle.radius < 0)
        {
          shape->position.x = shape->circle.radius;
          shape->velocity.x *= -0.9f;
        }
      }
    }

    // --AlNov: Collision
    for (PH_Shape* shape_a = shape_list.first;
         shape_a;
         shape_a = shape_a->next)
    {
      for (PH_Shape* shape_b = shape_a->next;
          shape_b;
          shape_b = shape_b->next)
      {
        PH_CollisionInfo collision_info = {};
        if (PH_CheckCollision(&collision_info, shape_a, shape_b))
        {
          printf("Depth: %f\n", collision_info.depth);
        }
      }
    }

    // --AlNov: Drawing
    for (PH_Shape* shape = shape_list.first;
         shape;
         shape = shape->next)
    {
      DrawPhysicsShape(frame_arena, shape);
    }

    R_DrawFrame();

    wind_force = {};

    // --AlNov: @TODO Not really understand how fixed fps works.
    // Because of this, it is looks ugly as ...
    QueryPerformanceCounter(&win32_cycles);
    u64 end_cycles    = win32_cycles.QuadPart;
    u64 cycles_delta  = end_cycles - start_cycles;
    start_cycles      = end_cycles;
    time_sec          = (f32)cycles_delta / (f32)frequency;

    const f32 fps           = 60.0f;
    const f32 ms_per_frame  = 1000.0f / fps; 
    f32 sleep_time          = ms_per_frame - (time_sec * 1000.0f);
    if (sleep_time > 0)
    {
      Sleep(sleep_time);
    }

    QueryPerformanceCounter(&win32_cycles);
    end_cycles    = win32_cycles.QuadPart;
    cycles_delta  = end_cycles - start_cycles;
    start_cycles  = end_cycles;
    time_sec      = (f32)cycles_delta / (f32)frequency;

    R_EndFrame();
    ResetArena(frame_arena);
  }

  return 0;
}

func void DrawBox(Arena* arena, Vec2f position, f32 width, f32 height, f32 angle)
{
  f32 local_width   = width / 2.0f;
  local_width       = (local_width / 1280.0f);
  f32 local_height  = height / 2.0f;
  local_height      = (local_height / 720.0f);

  R_Mesh* mesh = (R_Mesh*)PushArena(arena, sizeof(R_Mesh));
  mesh->mvp.color             = MakeRGB(1.0f, 1.0f, 1.0f);
  mesh->mvp.center_position.x = position.x / 1280.0f * 2.0f - 1.0f;
  mesh->mvp.center_position.y = position.y / 720.0f * 2.0f - 1.0f;
  mesh->mvp.center_position.z = 0.0f;
  mesh->vertex_count          = 4;
  mesh->vertecies             = (R_MeshVertex*)PushArena(arena, sizeof(R_MeshVertex) * mesh->vertex_count);
  mesh->vertecies[0].position = MakeVec3f(-local_width, -local_height, 0.0f);
  mesh->vertecies[0].position = Vec3fFromVec2f(RotateVec2f(MakeVec2f(mesh->vertecies[0].position.x, mesh->vertecies[0].position.y), angle));
  mesh->vertecies[1].position = MakeVec3f(local_width, -local_height, 0.0f);
  mesh->vertecies[1].position = Vec3fFromVec2f(RotateVec2f(MakeVec2f(mesh->vertecies[1].position.x, mesh->vertecies[1].position.y), angle));
  mesh->vertecies[2].position = MakeVec3f(local_width, local_height, 0.0f);
  mesh->vertecies[2].position = Vec3fFromVec2f(RotateVec2f(MakeVec2f(mesh->vertecies[2].position.x, mesh->vertecies[2].position.y), angle));
  mesh->vertecies[3].position = MakeVec3f(-local_width, local_height, 0.0f);
  mesh->vertecies[3].position = Vec3fFromVec2f(RotateVec2f(MakeVec2f(mesh->vertecies[3].position.x, mesh->vertecies[3].position.y), angle));
  mesh->index_count           = 6;
  mesh->indecies              = (u32*)PushArena(arena, sizeof(R_MeshVertex) * mesh->index_count);
  mesh->indecies[0]           = 0;
  mesh->indecies[1]           = 1;
  mesh->indecies[2]           = 2;
  mesh->indecies[3]           = 2;
  mesh->indecies[4]           = 3;
  mesh->indecies[5]           = 0;
  R_AddMeshToDrawList(mesh);
} 

func void DrawCircle(Arena* arena, Vec2f position, f32 radius, f32 angle)
{
  const u32 points_number   = 40;
  f32 angle_step            = 2.0f * 3.141592654f / (f32)points_number;

  R_Mesh* mesh              = (R_Mesh*)PushArena(arena, sizeof(R_Mesh));
  mesh->mvp.color           = MakeRGB(1.0f, 1.0f, 1.0f);
  mesh->mvp.center_position = MakeVec3f((position.x / 1280.0f) * 2 - 1, (position.y / 720.0f) * 2 - 1, 0.0f);
  mesh->vertex_count        = points_number;
  mesh->vertecies           = (R_MeshVertex*)PushArena(arena, sizeof(R_MeshVertex) * mesh->vertex_count);
  mesh->index_count         = (points_number - 2) * 3;
  mesh->indecies            = (u32*)PushArena(arena, sizeof(R_MeshVertex) * mesh->index_count);

  for (u32 i = 0; i < points_number; i += 1)
  {
    f32 current_angle = angle_step * i;
    Vec3f point_position = MakeVec3f(    
      radius * cos(current_angle), 
      radius * sin(current_angle),
      0.0f
    );

    mesh->vertecies[i].position = point_position;
  }

  u32 vertex_index = 0;
  for (u32 i = 0; i < mesh->index_count; i += 3)
  {
    mesh->indecies[i]     = 0;
    mesh->indecies[i + 1] = vertex_index + 1;
    mesh->indecies[i + 2] = vertex_index + 2;

    vertex_index += 1;
  }

  R_AddMeshToDrawList(mesh);

  f32 radius_screen_value = radius * 1280.0f / 2.0f;
  Vec2f rotation_second_point = RotateVec2f(MakeVec2f(radius_screen_value, 0.0f), angle);
  DrawLine(arena, MakeVec3f(position.x, position.y, 0.0f), MakeVec3f(position.x + rotation_second_point.x, position.y + rotation_second_point.y, 0));
}

func void DrawPhysicsShape(Arena* arena, PH_Shape* shape)
{
  switch (shape->type)
  {
    case PH_SHAPE_TYPE_CIRCLE:
    {
      DrawCircle(arena, shape->position, shape->circle.radius, shape->angle);
    } break;
    case PH_SHAPE_TYPE_BOX:
    {
      DrawBox(arena, shape->position, shape->box.width, shape->box.height, shape->angle);
    } break;
    default: break;
  }
}

func void DrawLine(Arena* arena, Vec3f p0, Vec3f p1)
{
  p0.x = (p0.x / 1280.0f) * 2 - 1;
  p0.y = (p0.y / 720.0f) * 2 - 1;
  p1.x = (p1.x / 1280.0f) * 2 - 1;
  p1.y = (p1.y / 720.0f) * 2 - 1;

  R_Line* line = (R_Line*)PushArena(arena, sizeof(R_Line));
  line->vertecies[0].position = p0;
  line->vertecies[1].position = p1;

  R_AddLineToDrawList(line);
}
