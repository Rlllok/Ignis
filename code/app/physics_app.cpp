#include "base/base_include.h"
#include "os/os_include.h"
#include "render/r_include.h"
#include "draw/d_include.h"

#include "base/base_include.cpp"
#include "os/os_include.cpp"
#include "render/r_include.cpp"
#include "draw/d_include.cpp"
#include <winuser.h>

struct Particle
{
  Vec2f position;
  Vec2f velocity;
  Vec2f force;
  F32   radius;
  F32   mass;
};

#define PIXELS_PER_METER 10.0f
#define GROUND_LEVEL 200.0f

#define MAX_PARTICLES 10
global Particle particles[MAX_PARTICLES] = {};

func void ParticleApplyForce(Particle* particle, Vec2f force);
func void ParticleIntegrateEuler(Particle* particle, F32 dt);
func void ParticleGroundCollision(Particle* particle, F32 ground_level);
func void ParticleBoundBoxCollision(Particle* particle, RectI bound_box);

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

  for (I32 i = 0; i < MAX_PARTICLES; i += 1)
  {
    particles[i].position = MakeVec2f(30.0f + i*50.0f, 40.0f);
    particles[i].velocity = MakeVec2f(0.0f, 0.0f);
    particles[i].radius = 10.0f;
    particles[i].mass = 1.0f + i;
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

    RectI bound_box = {};
    bound_box.x = window_rect.left;
    bound_box.y = window_rect.top;
    bound_box.w = window_rect.right;
    bound_box.h = window_rect.bottom;
    for (I32 i = 0; i < MAX_PARTICLES; i += 1)
    {
      ParticleApplyForce(&particles[i], MakeVec2f(0.0f, 1*9.8f*PIXELS_PER_METER));
      ParticleApplyForce(&particles[i], MakeVec2f(10.0f, 5.0f));
      ParticleIntegrateEuler(&particles[i], delta_time);

      ParticleBoundBoxCollision(&particles[i], bound_box);
    }

    R_FrameInfo frame_info = {};
    Renderer.BeginFrame();
    {
      Vec4f clear_color = MakeVec4f(0.2f, 0.2f, 0.2f, 1.0f);
      F32 depth_clear = 1.0f;
      F32 stencil_clear = 0.0f;
      Renderer.BeginRenderPass(clear_color, depth_clear, stencil_clear);
      {
        RectI ground = {};
        ground.x = 0;
        ground.y = GROUND_LEVEL;
        // AlNov: @TODO Viewport size is not equal window size
        //        Get Viewport size from Render layer could be useful
        ground.w = window_rect.right;
        ground.h = 30;
        D_DrawRectangle(ground, MakeVec3f(1.0f, 0.5f, 0.0f));

        D_DrawCircle(
          MakeVec2I(0, 0), 50, MakeVec3f(0.8f, 0.3f, 0.02f));

        for (I32 i = 0; i < MAX_PARTICLES; i += 1)
        {
          Particle* particle = particles + i;

          D_DrawCircle(
            Vec2IFromVec(particle->position),
            particle->radius, MakeVec3f(0.8f, 0.3f, 0.02f));
        }
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
ParticleApplyForce(Particle* particle, Vec2f force)
{
  particle->force += force;
}

func void
ParticleIntegrateEuler(Particle* particle, F32 dt)
{
  Vec2f F = particle->force;
  F32 m = particle->mass;
  Vec2f a = F / m;

  Vec2f v = particle->velocity + a*dt;
  Vec2f s = particle->position + v*dt;

  particle->velocity = v;
  particle->position = s;
  particle->force = {};
}

func void
ParticleGroundCollision(Particle* particle, F32 ground_level)
{
  F32 low_point_position = particle->position.y + particle->radius;

  F32 delta = ground_level - low_point_position;

  if (delta < 0.0f)
  {
    // particle->position.y = ground_level - 2*particle->radius;
    particle->velocity.y *= -1;
  }
}

func void
ParticleBoundBoxCollision(Particle* particle, RectI bound_box)
{
  Vec2f position = particle->position;
  Vec2I box_min = bound_box.position;
  Vec2I box_max = box_min + bound_box.size;

  if ((position.x < box_min.x) || (position.x > box_max.x))
  {
    particle->velocity.x *= -1;
  }
  if ((position.y < box_min.y) || (position.y > box_max.y))
  {
    particle->velocity.y *= -1;
  }
}
