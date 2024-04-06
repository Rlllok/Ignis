#pragma comment(lib, "Winmm.lib")

// --AlNov: .h -------------------------------------------------------
#include "../base/base_include.h"
#include "../os/os_include.h"
#include "../render/vulkan/r_init_vk.h"

// --AlNov: .cpp -----------------------------------------------------
#include "../base/base_include.cpp"
#include "../os/os_include.cpp"
#include "../render/vulkan/r_init_vk.cpp"

// --AlNov: TMP Globals ----------------------------------------------
global f32 pixels_per_meter = 20.0f;
global f32 particle_radius = pixels_per_meter / 4.0f;

struct Particle
{
  f32 mass;

  Vec2f position;
  Vec2f velocity;
  Vec2f acceleration;

  Vec2f sum_of_forces;

  Particle* next;
};

struct ParticleList
{
  Particle* first;
  Particle* last;

  u32 count;
};

func void PushParticle(ParticleList* list, Particle* particle)
{
  if (list->count == 0)
  {
    list->first = particle;
    list->last = particle;
    list->count += 1;
  }
  else
  {
    list->last->next = particle;
    list->last = particle;
    list->count += 1;
  };
}

func Particle* CreateParticle(Arena* arena, Vec2f position)
{
  Particle* particle = (Particle*)PushArena(arena, sizeof(Particle));
  particle->position = position;
  particle->mass = 1;
  particle->velocity = MakeVec2f(0.0f, 0.0f);
  particle->acceleration = MakeVec2f(0.0f, 0.0f);
  particle->sum_of_forces = MakeVec2f(0.0f, 0.0f);
  particle->next = 0;
  return particle;
}

func void ApplyForceToParticle(Particle* particle, Vec2f force)
{
  if (particle == 0) return;
  particle->sum_of_forces = AddVec2f(particle->sum_of_forces, force);
}

func void ApplyWeightToParticle(Particle* particle)
{
  Vec2f weight = MakeVec2f(0.0f, particle->mass * 9.8f * pixels_per_meter);
  particle->sum_of_forces = AddVec2f(particle->sum_of_forces, weight);
}

func void ApplyDragToParticle(Particle* particle)
{
  Vec2f drag = NormalizeVec2f(particle->velocity);
  // f32 ro = 1.2f; // Air density at 30 C
  // f32 K = 0.04f; // Drag coeff of "drop" like shape
  // f32 A = 1;
  // f32 drag_magnitude = 0.5f * ro * K * A * MagnitudeSquareVec2f(particle->velocity);
  // drag = MulVec2f(drag-, drag_magnitude);
  drag = MulVec2f(drag, -1 * MagnitudeSquareVec2f(particle->velocity));
  drag = MulVec2f(drag, 0.03f);
  particle->sum_of_forces = AddVec2f(particle->sum_of_forces, drag);
}

func void ApplySpringToParticle(Particle* particle, Vec2f anchor_position, f32 rest_length, f32 k)
{
  Vec2f distance = SubVec2f(particle->position, anchor_position);
  Vec2f direction = NormalizeVec2f(distance);
  f32 spring_force_magnitude = (rest_length - MagnitudeVec2f(distance)) * k;
  Vec2f spring_force = MulVec2f(direction, spring_force_magnitude);
  particle->sum_of_forces = AddVec2f(particle->sum_of_forces, spring_force);
}

func void IntegrateParticle(Particle* particle, f32 dt)
{
  // --AlNov: Euler Integration
  particle->acceleration = MulVec2f(particle->sum_of_forces, 1.0f / particle->mass);
  particle->velocity = AddVec2f(particle->velocity, MulVec2f(particle->acceleration, dt));
  particle->position = AddVec2f(particle->position, MulVec2f(particle->velocity, dt));
}

func void ResetParticleForce(Particle* particle)
{
  particle->sum_of_forces = {};
}

func void DrawParticle(Arena* arena, Particle* particle)
{
  Rect2f box = {};
  box.min = MakeVec2f(particle->position.x - particle_radius, particle->position.y - particle_radius);
  box.max = MakeVec2f(particle->position.x + particle_radius, particle->position.y + particle_radius);
  box.x0 = (box.x0 / 1280.0f) * 2 - 1;
  box.y0 = (box.y0 / 720.0f) * 2 - 1;
  box.x1 = (box.x1 / 1280.0f) * 2 - 1;
  box.y1 = (box.y1 / 720.0f) * 2 - 1;

  R_Mesh* mesh = (R_Mesh*)PushArena(arena, sizeof(R_Mesh));
  mesh->mvp.color = MakeRGB(1.0f, 1.0f, 1.0f);
  mesh->mvp.centerPosition = MakeVec3f(0.0f, 0.0f, 0.0f);
  mesh->vertecies[0].position = MakeVec3f(box.x0, box.y0, 0.0f);
  mesh->vertecies[1].position = MakeVec3f(box.x1, box.y0, 0.0f);
  mesh->vertecies[2].position = MakeVec3f(box.x1, box.y1, 0.0f);
  mesh->vertecies[3].position = MakeVec3f(box.x0, box.y1, 0.0f);
  mesh->indecies[0] = 0;
  mesh->indecies[1] = 1;
  mesh->indecies[2] = 2;
  mesh->indecies[3] = 2;
  mesh->indecies[4] = 3;
  mesh->indecies[5] = 0;

  R_AddMeshToDrawList(mesh);
}

func void DrawCircle(Arena* arena, Vec2f position, f32 radius)
{
  const u32 points_number = 4;
  f32 angle_step = 2.0f * 3.141592654f / (f32)points_number;
  R_Mesh* mesh = (R_Mesh*)PushArena(arena, sizeof(R_Mesh));
  mesh->mvp.color = MakeRGB(1.0f, 1.0f, 1.0f);
  // mesh->mvp.centerPosition = MakeVec3f(position.x, position.y, 0.0f);
  mesh->mvp.centerPosition = MakeVec3f(0.0f, 0.0f, 0.0f);

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

  for (u32 i = 0; i < points_number - 2; i += 1)
  {
    mesh->indecies[i + 2*i] = 0;
    mesh->indecies[i + 1 + 2*i] = i + 1;
    mesh->indecies[i + 2 + 2*i] = i + 2;
  }

  R_AddMeshToDrawList(mesh);
}

func void DrawLine(Arena* arena, Vec3f p0, Vec3f p1)
{
  R_Line* line = (R_Line*)PushArena(arena, sizeof(R_Line));
  line->vertecies[0].position = p0;
  line->vertecies[1].position = p1;

  R_AddLineToDrawList(line);
}

func void DrawWater(Arena* arena, Rect2f water_rect)
{
    Rect2f box = water_rect;
    
    box.x0 = (box.x0 / 1280.0f) * 2 - 1;
    box.y0 = (box.y0 / 720.0f) * 2 - 1;
    box.x1 = (box.x1 / 1280.0f) * 2 - 1;
    box.y1 = (box.y1 / 720.0f) * 2 - 1;
    
    R_Mesh* mesh = (R_Mesh*)PushArena(arena, sizeof(R_Mesh));
    mesh->mvp.color = MakeRGB(0.05f, 0.1f, 0.8f);
    mesh->mvp.centerPosition = MakeVec3f(0.0f, 0.0f, 0.0f);
    mesh->vertecies[0].position = MakeVec3f(box.x0, box.y0, 0.0f);
    mesh->vertecies[1].position = MakeVec3f(box.x1, box.y0, 0.0f);
    mesh->vertecies[2].position = MakeVec3f(box.x1, box.y1, 0.0f);
    mesh->vertecies[3].position = MakeVec3f(box.x0, box.y1, 0.0f);
    mesh->indecies[0] = 0;
    mesh->indecies[1] = 1;
    mesh->indecies[2] = 2;
    mesh->indecies[3] = 2;
    mesh->indecies[4] = 3;
    mesh->indecies[5] = 0;
    
    R_AddMeshToDrawList(mesh);
}

int main()
{
  OS_Window window = OS_CreateWindow("Game", MakeVec2u(1280, 720));

  // --AlNov: change Windows schedular granuality
  // Should be added to win32 layer
  u32 desired_schedular_ms = 1;
  timeBeginPeriod(desired_schedular_ms);

  R_Init(window);

  OS_ShowWindow(&window);

  LARGE_INTEGER win32_freq;
  QueryPerformanceFrequency(&win32_freq);
  u64 frequency = win32_freq.QuadPart;

  LARGE_INTEGER win32_cycles;
  QueryPerformanceCounter(&win32_cycles);
  u64 start_cycles = win32_cycles.QuadPart;

  Arena* tmp_arena = AllocateArena(Megabytes(64));

  f32 time_sec = 0.0f;

  Arena* particle_arena = AllocateArena(Megabytes(8));
  ParticleList particle_list;
  Particle* particle0 = CreateParticle(particle_arena, MakeVec2f(200.0f, 300.0f));
  PushParticle(&particle_list, particle0);
  Particle* particle1 = CreateParticle(particle_arena, MakeVec2f(200.0f, 400.0f));
  PushParticle(&particle_list, particle1);
  Particle* particle2 = CreateParticle(particle_arena, MakeVec2f(300.0f, 400.0f));
  PushParticle(&particle_list, particle2);
  Particle* particle3 = CreateParticle(particle_arena, MakeVec2f(300.0f, 300.0f));
  PushParticle(&particle_list, particle3);

  Vec2f wind_force = {};
  f32 wind_force_magnitude = 100.0f * pixels_per_meter;

  bool b_finished = false;
  while (!b_finished)
  {
    OS_EventList event_list = OS_GetEventList(tmp_arena);

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
          Particle* particle = CreateParticle(particle_arena, particle_position);
          PushParticle(&particle_list, particle);
        }

        default: break;
      }

      current_event = current_event->next;
    }

    // --AlNov: Update Particle
    for (Particle* particle = particle_list.first;
         particle != 0;
         particle = particle->next)
    {
      ApplyForceToParticle(particle, wind_force);
      ApplyWeightToParticle(particle);
      ApplyDragToParticle(particle);
      if (particle->next)
      {
        ApplySpringToParticle(particle, particle->next->position, 100.0f, 10.0f);
      }
      else
      {
        ApplySpringToParticle(particle, particle_list.last->position, 100.0f, 10.0f);
      }
      IntegrateParticle(particle, time_sec);

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

      DrawParticle(tmp_arena, particle);
      ResetParticleForce(particle);
    }

    R_DrawMesh();

    // DrawLine(tmp_arena, MakeVec3f(50.0f, 50.0f, 0.0f), MakeVec3f(150.0f, 150.0f, 0.0f));
    // R_DrawLine();

    wind_force = {};

    R_EndFrame();
    ResetArena(tmp_arena);

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
  }

  return 0;
}
