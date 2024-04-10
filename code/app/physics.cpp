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
global f32 particle_radius = pixels_per_meter / 4.0f;

func void DrawCircle(Arena* arena, Vec2f position, f32 radius)
{
  const u32 points_number = 40;
  f32 angle_step = 2.0f * 3.141592654f / (f32)points_number;
  R_Mesh* mesh = (R_Mesh*)PushArena(arena, sizeof(R_Mesh));
  mesh->mvp.color = MakeRGB(1.0f, 1.0f, 1.0f);
  mesh->mvp.center_position = MakeVec3f((position.x / 1280.0f) * 2 - 1, (position.y / 720.0f) * 2 - 1, 0.0f);
  mesh->vertex_count = points_number;
  mesh->vertecies = (R_MeshVertex*)PushArena(arena, sizeof(R_MeshVertex) * mesh->vertex_count);
  mesh->index_count = (points_number - 2) * 3;
  mesh->indecies = (u32*)PushArena(arena, sizeof(R_MeshVertex) * mesh->index_count);

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
    mesh->indecies[i] = 0;
    mesh->indecies[i + 1] = vertex_index + 1;
    mesh->indecies[i + 2] = vertex_index + 2;

    vertex_index += 1;
  }

  R_AddMeshToDrawList(mesh);
}

func void DrawParticle(Arena* arena, PH_Particle* particle)
{
  DrawCircle(arena, particle->position, 0.01f);
}

func void DrawLine(Arena* arena, Vec3f p0, Vec3f p1)
{
  R_Line* line = (R_Line*)PushArena(arena, sizeof(R_Line));
  p0.x = (p0.x / 1280.0f) * 2 - 1;
  p0.y = (p0.y / 720.0f) * 2 - 1;
  p1.x = (p1.x / 1280.0f) * 2 - 1;
  p1.y = (p1.y / 720.0f) * 2 - 1;
  line->vertecies[0].position = p0;
  line->vertecies[1].position = p1;

  R_AddLineToDrawList(line);
}

int main()
{
  OS_Window window = OS_CreateWindow("Physics", MakeVec2u(1280, 720));

  // --AlNov: change Windows schedular granuality
  // Should be added to win32 layer
  u32 desired_schedular_ms = 1;
  timeBeginPeriod(desired_schedular_ms);

  R_VK_Init(&window);

  OS_ShowWindow(&window);

  LARGE_INTEGER win32_freq;
  QueryPerformanceFrequency(&win32_freq);
  u64 frequency = win32_freq.QuadPart;

  LARGE_INTEGER win32_cycles;
  QueryPerformanceCounter(&win32_cycles);
  u64 start_cycles = win32_cycles.QuadPart;

  Arena* frame_arena = AllocateArena(Megabytes(128));

  f32 time_sec = 0.0f;

  Arena* particle_arena = AllocateArena(Megabytes(8));
  PH_ParticleList particle_list;
  PH_Particle* particle0 = PH_CreateParticle(particle_arena, MakeVec2f(200.0f, 400.0f), 1.0f);
  PH_PushParticle(&particle_list, particle0);
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
          Vec2f particle_position = MakeVec2f(current_event->mouseX, current_event->mouseY);
          PH_Particle* particle = PH_CreateParticle(particle_arena, particle_position, 1.0f);
          PH_PushParticle(&particle_list, particle);
        }

        default: break;
      }

      current_event = current_event->next;
    }

    // --AlNov: Update Particle
    for (PH_Particle* particle = particle_list.first;
         particle;
         particle = particle->next)
    {
      Vec2f weight = PH_CalculateWeight(particle->mass, 9.8 * pixels_per_meter);
      PH_ApplyForceToParticle(particle, weight);
      Vec2f drag = PH_CalculateDrag(particle->velocity, 0.03f);
      PH_ApplyForceToParticle(particle, drag);
      Vec2f spring = PH_CalculateSpring(particle->position, anchor_position, 100.0f, 10.0f);
      PH_ApplyForceToParticle(particle, spring);
      PH_ApplyForceToParticle(particle, wind_force);

      PH_IntegrateParticle(particle, time_sec);

      if (particle->position.y + particle_radius > window.height)
      {
        particle->position.y = window.height - particle_radius;
        particle->velocity.y *= -0.9f;
      }
      else if (particle->position.y - particle_radius < 0)
      {
        particle->position.y = particle_radius;
        particle->velocity.y *= -0.9f;
      }

      if (particle->position.x + particle_radius > window.width)
      {
        particle->position.x = window.width - particle_radius;
        particle->velocity.x *= -0.9f;
      }
      else if (particle->position.x - particle_radius < 0)
      {
        particle->position.x = particle_radius;
        particle->velocity.x *= -0.9f;
      }

      Vec3f first_point = MakeVec3f(anchor_position.x, anchor_position.y, 0.0f);
      Vec3f second_point = MakeVec3f(particle->position.x, particle->position.y, 0.0f);
      DrawLine(frame_arena, first_point, second_point);
      DrawParticle(frame_arena, particle);
      PH_ResetParticleForce(particle);
    }

    R_DrawFrame();

    // DrawLine(tmp_arena, MakeVec3f(50.0f, 50.0f, 0.0f), MakeVec3f(150.0f, 150.0f, 0.0f));
    // R_DrawLine();

    wind_force = {};

    // --AlNov: @TODO Not really understand how fixed fps works.
    // Because of this, it is looks ugly as ...
    QueryPerformanceCounter(&win32_cycles);
    u64 end_cycles = win32_cycles.QuadPart;
    u64 cycles_delta = end_cycles - start_cycles;
    start_cycles = end_cycles;
    time_sec = (f32)cycles_delta / (f32)frequency;

    const f32 fps = 60.0f;
    const f32 ms_per_frame = 1000.0f / fps; 
    f32 sleep_time = ms_per_frame - (time_sec * 1000.0f);
    if (sleep_time > 0)
    {
      Sleep(sleep_time);
    }

    QueryPerformanceCounter(&win32_cycles);
    end_cycles = win32_cycles.QuadPart;
    cycles_delta = end_cycles - start_cycles;
    start_cycles = end_cycles;
    time_sec = (f32)cycles_delta / (f32)frequency;

    R_EndFrame();
    ResetArena(frame_arena);
  }

  return 0;
}
