#include "physics_2d.h"

func ParticleID
PH_CreateParticle2D(World2D* world, Vec2f position, F32 mass,  F32 radius)
{
  ParticleID id = world->particles.count;

  world->particles.position[id] = position;
  world->particles.mass[id] = mass;
  world->particles.radius[id] = radius;

  world->particles.count += 1;

  return id;
}

func void
PH_ParticleParticleCollision(struct World2D* world)
{
  for (I32 i = 0; i < world->particles.count; i += 1)
  {
    for (I32 j = i + 1; j < world->particles.count; j += 1)
    {
      Vec2f p0 = world->particles.position[i];
      F32 r0 = world->particles.radius[i];
      Vec2f p1 = world->particles.position[j];
      F32 r1 = world->particles.radius[j];

      F32 distance = MagnitudeVec2f(p1 - p0);
      Vec2f collision_norm = NormalizeVec2f(p1 - p0);

      if (distance < (r0 + r1))
      {
        F32 speed0 = MagnitudeVec2f(world->particles.velocity[i]);
        world->particles.velocity[i] = collision_norm*-speed0;
        F32 speed1 = MagnitudeVec2f(world->particles.velocity[j]);
        world->particles.velocity[j] = collision_norm*speed1;
      }
    }
  }
}

func void
PH_ParticleBoundBoxCollision(struct World2D* world, RectI bound_box)
{
  ParticlesSoA* particles = &world->particles;

  Vec2I box_min = bound_box.position;
  Vec2I box_max = box_min + bound_box.size;

  for (I32 i = 0; i < world->particles.count; i += 1)
  {
    Vec2f position = particles->position[i];

    if (position.x < box_min.x)
    {
      particles->position[i].x = box_min.x + particles->radius[i];
      particles->velocity[i].x *= -0.8f;
    }
    if (position.x > box_max.x)
    {
      particles->position[i].x = box_max.x - particles->radius[i];
      particles->velocity[i].x *= -0.8f;
    }
    if (position.y < box_min.y)
    {
      particles->position[i].y = box_min.y + particles->radius[i];
      particles->velocity[i].y *= -0.8f;
    }
    if (position.y > box_max.y)
    {
      particles->position[i].y = box_max.y - particles->radius[i];
      particles->velocity[i].y *= -0.8f;
    }
  }
}

func void
PH_UpdateWorld2D(World2D* world, F32 dt)
{
  ParticlesSoA* particles = &world->particles;

  for (I32 i = 0; i < world->particles.count; i += 1)
  {
    Vec2f gravity_force = world->gravity*particles->mass[i];
    particles->force[i] += gravity_force;
    particles->force[i] += world->forces;

    Vec2f F = particles->force[i];
    F32 m = particles->mass[i];
    Vec2f a = F / m;

    Vec2f v = particles->velocity[i] + a*dt;
    Vec2f s = particles->position[i] + v*dt;

    particles->velocity[i] = v;
    particles->position[i] = s;
    particles->force[i] = {};
  }
}
