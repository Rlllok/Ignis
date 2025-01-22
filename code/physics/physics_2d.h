#pragma once

#include "base/base_include.h"

#define MAX_PARTICLE_COUNT 10000

typedef U32 ParticleID;

struct ParticlesSoA
{
  Vec2f position[MAX_PARTICLE_COUNT];
  Vec2f velocity[MAX_PARTICLE_COUNT];
  Vec2f force[MAX_PARTICLE_COUNT];
  F32 mass[MAX_PARTICLE_COUNT];
  F32 radius[MAX_PARTICLE_COUNT];

  U32 count;
};

func ParticleID PH_CreateParticle2D(struct World2D* world, Vec2f position, F32 mass,  F32 radius);
func void PH_ParticleParticleCollision(struct World2D* world);
func void PH_ParticleBoundBoxCollision(struct World2D* world, RectI bound_box);

struct World2D
{
  Vec2f gravity;
  Vec2f forces;

  ParticlesSoA particles;
};

func void PH_UpdateWorld2D(World2D* world, F32 delta_time);
