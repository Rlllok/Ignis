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
  PH_Particle*  first;
  PH_Particle*  last;
  u32           count;
};

func PH_Particle* PH_CreateParticle(Arena* arena, Vec2f position, f32 mass);
func void PH_ApplyForceToParticle(PH_Particle* particle, Vec2f force);
func void PH_ResetParticleForce(PH_Particle* particle);
func void PH_IntegrateParticle(PH_Particle* particle, f32 dt);

func void PH_PushParticle(PH_ParticleList* list, PH_Particle* particle);

// -------------------------------------------------------------------
// -AlNov: Shapes
enum PH_ShapeType
{
  PH_SHAPE_TYPE_NONE,
  PH_SHAPE_TYPE_CIRCLE,
  PH_SHAPE_TYPE_BOX,

  PH_SHAPE_TYPE_COUNT
};

// --AlNov: @TODO Using Tagged-Union
// Maybe ther is better solution
struct PH_Circle
{
  f32 radius;
};

struct PH_Box
{
  f32 height;
  f32 width;
};

struct PH_Shape
{
  f32   mass;
  f32   inv_mass;
  Vec2f position;
  Vec2f velocity;
  Vec2f acceleration;
  Vec2f sum_of_forces;

  f32 moment_of_inertia;
  f32 angle;
  f32 angular_velocity;
  f32 angular_acceleration;
  f32 sum_of_torque;

  PH_ShapeType type;
  union
  {
    PH_Circle circle;
    PH_Box    box;
  };

  PH_Shape* next;
};

struct PH_ShapeList
{
  PH_Shape* first;
  PH_Shape* last;
  u32 count;
};

func PH_Shape* PH_CreateCircleShape(Arena* arena, Vec2f position, f32 radius, f32 mass);
func PH_Shape* PH_CreateBoxShape(Arena* arena, Vec2f position, f32 height, f32 width, f32 mass);

func void PH_ApplyForceToShape(PH_Shape* shape, Vec2f force);
func void PH_ApplyImpulseToShape(PH_Shape* shape, Vec2f j);
func void PH_ApplyTorqueToShape(PH_Shape* shape, f32 torque);
func void PH_IntegrateShape(PH_Shape* shape, f32 dt);

func void PH_PushShapeList(PH_ShapeList* list, PH_Shape* shape);

// -------------------------------------------------------------------
// --AlNov: Forces ---------------------------------------------------
func Vec2f PH_CalculateWeight(f32 m, f32 g);
func Vec2f PH_CalculateDrag(Vec2f velocity, f32 k);
func Vec2f PH_CalculateSpring(Vec2f blob_position, Vec2f anchor_position, f32 rest_length, f32 k);

// -------------------------------------------------------------------
// --AlNov: Helpers --------------------------------------------------
func bool PH_IsStatic(PH_Shape* shape);

// -------------------------------------------------------------------
// --AlNov: Collision ------------------------------------------------
struct PH_CollisionInfo
{
  PH_Shape* shape_a;
  PH_Shape* shape_b;

  // from a to b
  Vec2f normal;
  Vec2f start_point;
  Vec2f end_point;
  f32   depth;
};

func f32 PH_CalculateImpulseValue(PH_CollisionInfo* collision_info);

func bool PH_CheckCollision(PH_CollisionInfo* out_collision_info, PH_Shape* shape_a, PH_Shape* shape_b);
func bool PH_CircleCircleCollision(PH_CollisionInfo* out_collision_info, PH_Shape* circle_a, PH_Shape* circle_b);
func void PH_ResolveCollisionProjection(PH_CollisionInfo* collision_info);
func void PH_ResolveCollisionImpulse(PH_CollisionInfo* collision_info);
