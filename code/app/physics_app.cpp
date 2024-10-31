#include "base/base_include.h"
#include "os/gfx/os_gfx.h"
#include "os/os_include.h"
#include "render/r_include.h"
#include "draw/d_include.h"

#include "physics/physics_2d.h"

#include "base/base_include.cpp"
#include "os/os_include.cpp"
#include "render/r_include.cpp"
#include "draw/d_include.cpp"

#include "physics/physics_2d.cpp"

#include <winuser.h>

#define PIXELS_PER_METER 10.0f

struct RigidBody2D
{
  F32 mass;
  Vec2f size;
  Vec2f position;
  F32 rotation;
  Vec2f velocity;
  Vec2f acceleration;
  Vec2f sum_of_forces;
};

func void ApplyForceRigidBody2D(RigidBody2D* body, Vec2f force);
func void UpdateRigidBody2D(RigidBody2D* body, F32 dt);

I32 main()
{
  Arena* arena = AllocateArena(Megabytes(64));

  OS_Window window = OS_CreateWindow("PhysicsApp", MakeVec2u(1280, 720));
  R_Init(&window);
  D_Init(arena);

  RECT window_rect = {};
  GetClientRect(window.handle, &window_rect);

  OS_ShowWindow(&window);
  F32 delta_time = 1.0f / 60.0f;

  World2D world = {};
  world.gravity = MakeVec2f(0.0f, 9.8f*PIXELS_PER_METER);
  world.forces = MakeVec2f(30.0f, 0.0f);

  RigidBody2D box = {};
  box.mass = 10.0f;
  box.position = MakeVec2f(50.0f, 50.0f);
  box.size = MakeVec2f(75.0f, 25.0f);

  F32 particle_radius = 8.0f;
  for (I32 i = 0; i < 10; i += 1)
  {
    for (I32 j = 0; j < 10; j += 1)
    {
      Vec2f position = MakeVec2f(
        30.0f + (3*particle_radius)*j,
        50.0f + (3*particle_radius)*i);

      PH_CreateParticle2D(&world, position, 1.0f, particle_radius);
    }
  }

  B32 is_window_closed = false;
  while(!is_window_closed)
  {
    F32 begin_time = OS_CurrentTimeSeconds();

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

    // AlNov: Update Physics
    #if 0
    RectI bound_box = {};
    bound_box.x = window_rect.left;
    bound_box.y = window_rect.top;
    bound_box.w = window_rect.right;
    bound_box.h = window_rect.bottom;
    PH_ParticleBoundBoxCollision(&world, bound_box);
    PH_ParticleParticleCollision(&world);
    PH_UpdateWorld2D(&world, delta_time);
    #endif

    // ApplyForceRigidBody2D(&box, MakeVec2f(0.0f, 98.0f));
    UpdateRigidBody2D(&box, delta_time);

    // AlNov: Render
    R_FrameInfo frame_info = {};
    Renderer.BeginFrame();
    {
      Vec4f clear_color = MakeVec4f(0.2f, 0.2f, 0.2f, 1.0f);
      F32 depth_clear = 1.0f;
      F32 stencil_clear = 0.0f;
      Renderer.BeginRenderPass(clear_color, depth_clear, stencil_clear);
      {
        #if 0
        for (I32 i = 0; i < world.particles.count; i += 1)
        {
          D_DrawCircle(
            Vec2IFromVec(world.particles.position[i]),
            world.particles.radius[i], MakeVec3f(0.8f, 0.3f, 0.02f));
        }
        #endif

        RectI box_rect = {};
        box_rect.position = Vec2IFromVec(box.position);
        box_rect.size = Vec2IFromVec(box.size);
        D_DrawRectangle(box_rect, MakeVec3f(0.65f, 0.77f, 0.09f), box.rotation);
      }
      Renderer.EndRenderPass();
    }
    Renderer.EndFrame();

    F32 end_time = OS_CurrentTimeSeconds();
    F32 wait_time = delta_time - (end_time - begin_time);
    OS_Wait(wait_time);
  }
}

func void
ApplyForceRigidBody2D(RigidBody2D* body, Vec2f force)
{
  body->sum_of_forces += force;
}

func void
UpdateRigidBody2D(RigidBody2D* body, F32 dt)
{
  F32 inv_m = 1.0f/body->mass;
  Vec2f a = body->sum_of_forces*inv_m;

  body->velocity = body->velocity + a*dt;
  body->position = body->position + body->velocity*dt;

  body->rotation += 0.4f*dt;

  body->sum_of_forces = MakeVec2f(0.0f, 0.0f);
}
