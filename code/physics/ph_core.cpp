#include "ph_core.h"

func PH_Particle* PH_CreateParticle(Arena* arena, Vec2f position, f32 mass)
{
  PH_Particle* particle = (PH_Particle*)PushArena(arena, sizeof(PH_Particle));
  particle->position = position;
  particle->mass = mass;
  particle->velocity = MakeVec2f(0.0f, 0.0f);
  particle->acceleration = MakeVec2f(0.0f, 0.0f);
  particle->sum_of_forces = MakeVec2f(0.0f, 0.0f);
  particle->next = 0;
  return particle;
}

func void PH_ApplyForceToParticle(PH_Particle* particle, Vec2f force)
{
  if (particle == 0) return;
  particle->sum_of_forces = AddVec2f(particle->sum_of_forces, force);
}

func void PH_ResetParticleForce(PH_Particle* particle)
{
  particle->sum_of_forces = {};
}

func void PH_IntegrateParticle(PH_Particle* particle, f32 dt)
{
  // --AlNov: Euler Integration
  particle->acceleration = MulVec2f(particle->sum_of_forces, 1.0f / particle->mass);
  particle->velocity = AddVec2f(particle->velocity, MulVec2f(particle->acceleration, dt));
  particle->position = AddVec2f(particle->position, MulVec2f(particle->velocity, dt));
}

func void PH_PushParticle(PH_ParticleList* list, PH_Particle* particle)
{
  if (list->count == 0)
  {
    list->first = particle;
    list->last = particle;
    list->count += 1;

    particle->next = 0;
  }
  else
  {
    particle->next = 0;
    list->last->next = particle;
    list->last = particle;
    list->count += 1;
  };
}

// -------------------------------------------------------------------
// --AlNov: Forces ---------------------------------------------------
func Vec2f PH_CalculateWeight(f32 m, f32 g)
{
  return MakeVec2f(0.0f, m * g);
}

func Vec2f PH_CalculateDrag(Vec2f velocity, f32 k)
{
  Vec2f drag = NormalizeVec2f(velocity);
  // f32 ro = 1.2f; // Air density at 30 C
  // f32 K = 0.04f; // Drag coeff of "drop" like shape
  // f32 A = 1;
  // f32 drag_magnitude = 0.5f * ro * K * A * MagnitudeSquareVec2f(particle->velocity);
  // drag = MulVec2f(drag-, drag_magnitude);
  drag = MulVec2f(drag, -1 * MagnitudeSquareVec2f(velocity));
  drag = MulVec2f(drag, k);
  return drag;
}

func Vec2f PH_CalculateSpring(Vec2f blob_position, Vec2f anchor_position, f32 rest_length, f32 k)
{
  Vec2f distance = SubVec2f(blob_position, anchor_position);
  Vec2f direction = NormalizeVec2f(distance);
  f32 spring_force_magnitude = (rest_length - MagnitudeVec2f(distance)) * k;
  Vec2f spring_force = MulVec2f(direction, spring_force_magnitude);
  return spring_force;
}
