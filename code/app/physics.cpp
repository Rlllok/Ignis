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
#define RED_COLOR    MakeVec3f(1.0f, 0.0f, 0.0f)
#define GREEN_COLOR  MakeVec3f(0.0f, 1.0f, 0.0f)
#define BLUE_COLOR   MakeVec3f(0.0f, 0.0f, 1.0f)
#define YELLOW_COLOR MakeVec3f(1.0f, 1.0f, 0.0f)
#define PINK_COLOR   MakeVec3f(1.0f, 0.0f, 1.0f)
#define WHITE_COLOR  MakeVec3f(1.0f, 1.0f, 1.0f)

#define PIXELS_PER_METER 40.0f

func void DrawBox(Arena* arena, PH_Shape* box, Vec3f color);
func void DrawCircle(Arena* arena, Vec2f position, f32 radius, f32 angle, Vec3f color);
func void DrawPhysicsShape(Arena* arena, PH_Shape* shape, Vec3f color);
func void DrawLine(Arena* arena, Vec2f p0, Vec2f p1);

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
  PH_Shape* floor = PH_CreateBoxShape(shape_arena, MakeVec2f(640.0f, 700.0f), 20.0f, 1200.0f, 0.0f);
  PH_PushShapeList(&shape_list, floor);
  PH_Shape* right_wall = PH_CreateBoxShape(shape_arena, MakeVec2f(50.0f, 450.0f), 600.0f, 30.0f, 0.0f);
  PH_PushShapeList(&shape_list, right_wall);
  PH_Shape* left_wall = PH_CreateBoxShape(shape_arena, MakeVec2f(1240.0f, 450.0f), 600.0f, 30.0f, 0.0f);
  PH_PushShapeList(&shape_list, left_wall);

  Vec2f box_position = MakeVec2f(640.0f, 100.0f);
  Vec2f box_distance = MakeVec2f(30.0f, 70.0f);
  PH_Shape* box = PH_CreateBoxShape(shape_arena, box_position, 25.0f, 25.0f, 0.0f);
  PH_PushShapeList(&shape_list, box);
  PH_Shape* box1 = PH_CreateBoxShape(shape_arena, AddVec2f(box->position, box_distance), 30.0f, 30.0f, 1.0f);
  PH_PushShapeList(&shape_list, box1);
  PH_Shape* box2 = PH_CreateBoxShape(shape_arena, AddVec2f(box1->position, box_distance), 30.0f, 30.0f, 1.0f);
  PH_PushShapeList(&shape_list, box2);
  PH_Shape* box3 = PH_CreateBoxShape(shape_arena, AddVec2f(box2->position, box_distance), 30.0f, 30.0f, 1.0f);
  PH_PushShapeList(&shape_list, box3);

  Arena*        constrain_arena     = AllocateArena(Megabytes(64));
  PH_Constrain* distance_constrain  = PH_CreateDistanceConstrain(constrain_arena, box, box1, box->position);
  PH_Constrain* distance_constrain1 = PH_CreateDistanceConstrain(constrain_arena, box1, box2, box1->position);
  PH_Constrain* distance_constrain2 = PH_CreateDistanceConstrain(constrain_arena, box2, box3, box2->position);

  PH_ConstrainList constrain_list = {};
  PH_PushConstrainList(&constrain_list, distance_constrain);
  PH_PushConstrainList(&constrain_list, distance_constrain1);
  PH_PushConstrainList(&constrain_list, distance_constrain2);

  bool b_finished = false;
  while (!b_finished)
  {
    OS_EventList event_list = OS_GetEventList(frame_arena);

    OS_Event* event = event_list.first;
    while (event)
    {
      switch (event->type)
      {
        case OS_EVENT_TYPE_EXIT:
        {
          b_finished = true;
        } break;

        case OS_EVENT_TYPE_KEYBOARD:
        {
          switch (event->key)
          {
            case OS_KEY_ARROW_UP:
            {
              if (event->was_down)
              {
                Vec2f     position = OS_MousePosition(window);
                PH_Shape* circle   = PH_CreateCircleShape(shape_arena, position, 25.0f, 1.0f);
                PH_PushShapeList(&shape_list, circle);
              }
            } break;

            case OS_KEY_ARROW_DOWN:
            {
              if (event->was_down)
              {
                Vec2f     position = OS_MousePosition(window);
                PH_Shape* box      = PH_CreateBoxShape(shape_arena, position, 50.0f, 50.0f, 1.0f);
                PH_PushShapeList(&shape_list, box);
              }
            } break;
            
            default: break;
          }
        } break;

        case OS_EVENT_TYPE_MOUSE_RELEASE:
        {
        }

        default: break;
      }

      event = event->next;
    }

    // --AlNov: @TODO There is bug if list is empty
    // --AlNov: Update Physics
    for (PH_Shape* shape = shape_list.first;
         shape;
         shape = shape->next)
    {
      Vec2f weight = PH_CalculateWeight(shape->mass, 9.8 * PIXELS_PER_METER);
      PH_ApplyForceToShape(shape, weight);
    }

    for (PH_Shape* shape = shape_list.first;
         shape;
         shape = shape->next)
    {
      PH_IntegrateForceShape(shape, time_sec);
    }

    for (PH_Constrain* constrain = constrain_list.first;
        constrain;
        constrain = constrain->next)
    {
      PH_PresolveConstrain(constrain, time_sec);
    }

    for (i32 i = 0; i < 10; i += 1)
    {
      for (PH_Constrain* constrain = constrain_list.first;
          constrain;
          constrain = constrain->next)
      {
        PH_SolveConstrain(constrain);
      }
    }
    DrawLine(frame_arena, box->position, box1->position);
    DrawLine(frame_arena, box1->position, box2->position);
    DrawLine(frame_arena, box2->position, box3->position);

    for (PH_Shape* shape = shape_list.first;
         shape;
         shape = shape->next)
    {
      PH_IntegrateVelocityShape(shape, time_sec);
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
          PH_ResolveCollisionImpulse(&collision_info);
        }
      }
    }
    // --AlNov: Drawing
    for (PH_Shape* shape = shape_list.first;
         shape;
         shape = shape->next)
    {
      DrawPhysicsShape(frame_arena, shape, WHITE_COLOR);
    }


    R_DrawFrame();

    // --AlNov: @TODO Not really understand how fixed fps works.
    // Because of this, it is looks ugly as ...
    QueryPerformanceCounter(&win32_cycles);
    u64 end_cycles    = win32_cycles.QuadPart;
    u64 cycles_delta  = end_cycles - start_cycles;
    start_cycles      = end_cycles;
    time_sec          = (f32)cycles_delta / (f32)frequency;

    // --AlNov: @TODO FPS Limitation doesn't work properly
    /*
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
    */

    R_EndFrame();
    ResetArena(frame_arena);
  }

  return 0;
}

func void DrawBox(Arena* arena, PH_Shape* box, Vec3f color)
{
  R_Mesh* mesh = (R_Mesh*)PushArena(arena, sizeof(R_Mesh));
  mesh->mvp.color             = color;
  mesh->mvp.center_position.x = box->position.x;
  mesh->mvp.center_position.y = box->position.y;
  mesh->mvp.center_position.z = 0.0f;
  mesh->vertex_count          = 4;
  mesh->vertecies             = (R_MeshVertex*)PushArena(arena, sizeof(R_MeshVertex) * mesh->vertex_count);
  mesh->vertecies[0].position = Vec3fFromVec2(BoxVertexWorldFromLocal(box, 0));
  mesh->vertecies[1].position = Vec3fFromVec2(BoxVertexWorldFromLocal(box, 1));
  mesh->vertecies[2].position = Vec3fFromVec2(BoxVertexWorldFromLocal(box, 2));
  mesh->vertecies[3].position = Vec3fFromVec2(BoxVertexWorldFromLocal(box, 3));
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

func void DrawCircle(Arena* arena, Vec2f position, f32 radius, f32 angle, Vec3f color)
{
  const u32 points_number   = 40;
  f32 angle_step            = 2.0f * 3.141592654f / (f32)points_number;

  R_Mesh* mesh              = (R_Mesh*)PushArena(arena, sizeof(R_Mesh));
  mesh->mvp.color           = color;
  mesh->mvp.center_position = MakeVec3f(position.x, position.y, 0.0f);
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

    mesh->vertecies[i].position.x += position.x;
    mesh->vertecies[i].position.y += position.y;
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
}

func void DrawPhysicsShape(Arena* arena, PH_Shape* shape, Vec3f color)
{
  switch (shape->type)
  {
    case PH_SHAPE_TYPE_CIRCLE:
    {
      DrawCircle(arena, shape->position, shape->circle.radius, shape->angle, color);
    } break;
    case PH_SHAPE_TYPE_BOX:
    {
      DrawBox(arena, shape, color);
    } break;
    default: break;
  }
}

func void DrawLine(Arena* arena, Vec2f p0, Vec2f p1)
{
  p0.x = (p0.x / 1280.0f) * 2 - 1;
  p0.y = (p0.y / 720.0f) * 2 - 1;
  p1.x = (p1.x / 1280.0f) * 2 - 1;
  p1.y = (p1.y / 720.0f) * 2 - 1;

  R_Line* line = (R_Line*)PushArena(arena, sizeof(R_Line));
  line->vertecies[0].position = Vec3fFromVec2(p0);
  line->vertecies[1].position = Vec3fFromVec2(p1);

  R_AddLineToDrawList(line);
}
