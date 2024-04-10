#pragma once
#include "../base/base_include.h"

// -------------------------------------------------------------------
// -AlNov: Particles
struct PH_Particle
{
  f32 mass;

  Vec2f position;
  Vec2f velocity;
  Vec2f acceleration;

  Vec2f sum_of_forces;

  PH_Particle* next;
};

struct PH_ParticleList
{
  PH_Particle* first;
  PH_Particle* last;

  u32 count;
};

func PH_Particle* PH_CreateParticle(Arena* arena, Vec2f position, f32 mass);
func void PH_ApplyForceToParticle(PH_Particle* particle, Vec2f force);
func void PH_ResetParticleForce(PH_Particle* particle);
func void PH_IntegrateParticle(PH_Particle* particle, f32 dt);

func void PH_PushParticle(PH_ParticleList* list, PH_Particle* particle);

// -------------------------------------------------------------------
// --AlNov: Forces ---------------------------------------------------
func Vec2f PH_CalculateWeight(f32 m, f32 g);
func Vec2f PH_CalculateDrag(Vec2f velocity, f32 k);
func Vec2f PH_CalculateSpring(Vec2f blob_position, Vec2f anchor_position, f32 rest_length, f32 k);
