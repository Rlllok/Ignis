#pragma comment(lib, "Winmm.lib")
// --AlNov: .h -------------------------------------------------------
#include "../base/base_include.h"
#include "../os/os_include.h"
#include "../render/r_include.h"
#include "../physics/ph_core.h"

// --AlNov: .cpp -----------------------------------------------------
#include "../base/base_include.cpp"
#include "../os/os_include.cpp"
#include "../render/r_include.cpp"
#include "../physics/ph_core.cpp"

#define PIXELS_PER_METER 40.0f

#define RED_COLOR    MakeVec3f(1.0f, 0.0f, 0.0f)
#define GREEN_COLOR  MakeVec3f(0.0f, 1.0f, 0.0f)
#define BLUE_COLOR   MakeVec3f(0.0f, 0.0f, 1.0f)
#define YELLOW_COLOR MakeVec3f(1.0f, 1.0f, 0.0f)
#define PINK_COLOR   MakeVec3f(1.0f, 0.0f, 1.0f)
#define WHITE_COLOR  MakeVec3f(1.0f, 1.0f, 1.0f)

func R_SceneObject* GenerateQuad(Arena* arena);

global R_SceneObject* QUAD;
global R_Pipeline    PIPELINE;

func void DrawBox(Vec2f position, Vec2f size);
func void DrawCircle(Vec2f position, f32 radius);
func void DrawPhysicsShape(Arena* arena, PH_Shape* shape, Vec3f color);

i32 main()
{
  OS_Window window = OS_CreateWindow("Physics", MakeVec2u(1280, 720));
  R_Init(&window);

  OS_ShowWindow(&window);

  LARGE_INTEGER win32_freq;
  QueryPerformanceFrequency(&win32_freq);
  u64 frequency = win32_freq.QuadPart;

  LARGE_INTEGER win32_cycles;
  QueryPerformanceCounter(&win32_cycles);
  u64 start_cycles = win32_cycles.QuadPart;

  Arena* frame_arena = AllocateArena(Megabytes(256));

  f32 time_sec = 1.0f / 60.0f;

  Arena* shape_arena = AllocateArena(Megabytes(128));

  {
    // --AlNov: @TODO
    // R_VertexAttributeFormat attributes[] = { R_VERTEX_ATTRIBUTE_FORMAT_VEC3F };
    R_VertexAttributeFormat attributes[] = {
      R_VERTEX_ATTRIBUTE_FORMAT_VEC3F,
      R_VERTEX_ATTRIBUTE_FORMAT_VEC3F,
      R_VERTEX_ATTRIBUTE_FORMAT_VEC2F
    };

    R_PipelineAssignAttributes(&PIPELINE, attributes, CountArrayElements(attributes));

    R_BindingInfo bindings[] =
    {
      {R_BINDING_TYPE_UNIFORM_BUFFER, R_SHADER_TYPE_VERTEX},
    };
    R_PipelineAssignBindingLayout(&PIPELINE, bindings, CountArrayElements(bindings));

    R_H_LoadShader(shape_arena, "data/shaders/sdf2D.vert", "main", R_SHADER_TYPE_VERTEX, &PIPELINE.shaders[R_SHADER_TYPE_VERTEX]);
    R_H_LoadShader(shape_arena, "data/shaders/sdf2D.frag", "main", R_SHADER_TYPE_FRAGMENT, &PIPELINE.shaders[R_SHADER_TYPE_FRAGMENT]);
    R_CreatePipeline(&PIPELINE);
  }

  QUAD = GenerateQuad(shape_arena);

  PH_ShapeList shape_list = {};
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
    f32 begin_time = OS_CurrentTimeSeconds();

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
    R_BeginFrame();
    {
      R_BeginRenderPass(MakeVec4f(0.3f, 0.3f, 0.3f, 1.0f), 1.0f, 0.0f);
      {
        for (PH_Shape* shape = shape_list.first;
          shape;
          shape = shape->next)
        {
          DrawPhysicsShape(frame_arena, shape, WHITE_COLOR);
        }

        // DrawCircle(MakeVec2f(250.0f, 250.0f), 50.0f);
        // DrawBox(MakeVec2f(550.0f, 250.0f), MakeVec2f(75.0f, 50.0f));
      }
      R_EndRenderPass();
    }
    R_EndFrame();

    ResetArena(frame_arena);

    f32 end_time = OS_CurrentTimeSeconds();
    f32 wait_time = time_sec - (end_time - begin_time);
    if (wait_time > 0) { OS_Wait(wait_time); };
  }

  return 0;
}

func R_SceneObject* GenerateQuad(Arena* arena)
{
  R_SceneObject* quad = (R_SceneObject*)PushArena(arena, sizeof(R_SceneObject));
  quad->vertex_count          = 4;
  quad->vertecies             = (R_SceneObject::Vertex*)PushArena(arena, sizeof(R_SceneObject::Vertex) * quad->vertex_count);
  quad->vertecies[0].position = MakeVec3f(-1.0f, -1.0f, 0.0f);
  quad->vertecies[1].position = MakeVec3f(1.0f, -1.0f, 0.0f);
  quad->vertecies[2].position = MakeVec3f(1.0f, 1.0f, 0.0f);
  quad->vertecies[3].position = MakeVec3f(-1.0f, 1.0f, 0.0f);
  quad->index_count           = 6;
  quad->indecies              = (u32*)PushArena(arena, sizeof(u32) * quad->index_count);
  quad->indecies[0]           = 0;
  quad->indecies[1]           = 1;
  quad->indecies[2]           = 2;
  quad->indecies[3]           = 2;
  quad->indecies[4]           = 3;
  quad->indecies[5]           = 0;

  return quad;
}

func void DrawBox(Vec2f position, Vec2f size)
{
  struct UBO
  {
    alignas(16) Mat4x4f projection;
    alignas(8)  Vec2f   resolution;
    alignas(8)  Vec2f   position;
    alignas(8)  Vec2f   size;
    alignas(4)  f32     is_box;
  };

  Vec2f resolution = MakeVec2f(1280.0f, 720.0f);

  UBO ubo = {};
  ubo.projection  = MakeOrthographic4x4f(0.0f, resolution.x, 0.0f, resolution.y, -1.0f, 2.0f);
  ubo.resolution  = resolution;
  ubo.position    = position;
  ubo.size        = size;
  ubo.is_box      = 1.0f;

  R_DrawInfo draw_info = {};
  draw_info.pipeline          = &PIPELINE;
  draw_info.vertecies         = QUAD->vertecies;
  draw_info.vertex_size       = sizeof(QUAD->vertecies[0]);
  draw_info.vertex_count      = QUAD->vertex_count;
  draw_info.indecies          = QUAD->indecies;
  draw_info.index_size        = sizeof(QUAD->indecies[0]);
  draw_info.index_count       = QUAD->index_count;
  draw_info.uniform_data      = &ubo;
  draw_info.uniform_data_size = sizeof(ubo);

  R_DrawSceneObject(&draw_info);
} 

func void DrawCircle(Vec2f position, f32 radius)
{
  struct UBO
  {
    alignas(16) Mat4x4f projection;
    alignas(8)  Vec2f   resolution;
    alignas(8)  Vec2f   position;
    alignas(8)  Vec2f   size;
    alignas(4)  f32     is_box;
  };

  Vec2f resolution = MakeVec2f(1280.0f, 720.0f);

  UBO ubo = {};
  ubo.projection  = MakeOrthographic4x4f(0.0f, resolution.x, 0.0f, resolution.y, -1.0f, 2.0f);
  ubo.resolution  = resolution;
  ubo.position    = position;
  ubo.size        = MakeVec2f(radius, radius);
  ubo.is_box      = 0.0f;

  R_DrawInfo draw_info = {};
  draw_info.pipeline          = &PIPELINE;
  draw_info.vertecies         = QUAD->vertecies;
  draw_info.vertex_size       = sizeof(QUAD->vertecies[0]);
  draw_info.vertex_count      = QUAD->vertex_count;
  draw_info.indecies          = QUAD->indecies;
  draw_info.index_size        = sizeof(QUAD->indecies[0]);
  draw_info.index_count       = QUAD->index_count;
  draw_info.uniform_data      = &ubo;
  draw_info.uniform_data_size = sizeof(ubo);

  R_DrawSceneObject(&draw_info);
}

func void DrawPhysicsShape(Arena* arena, PH_Shape* shape, Vec3f color)
{
  switch (shape->type)
  {
    case PH_SHAPE_TYPE_CIRCLE:
    {
      DrawCircle(shape->position, shape->circle.radius);
    } break;
    case PH_SHAPE_TYPE_BOX:
    {
      DrawBox(shape->position, MakeVec2f(shape->box.width, shape->box.height));
    } break;
    default: break;
  }
}