#pragma once
#include "../base/base_include.h"

// -------------------------------------------------------------------
// --AlNov: World ----------------------------------------------------
// --AlNov: @TODO Create World structure that contains all info/states
// related to phycis simulations (gravity, particle_list, shape_list,
// arena and so on)

// -------------------------------------------------------------------
// --AlNov: Types ----------------------------------------------------
// --AlNov: Particle
struct PH_Particle
{
  F32 mass;

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
  U32           count;
};

// --AlNov: Rigid Shape
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
  F32 radius;
};

struct PH_Box
{
  F32 height;
  F32 width;

  Vec2f vertecies[4];
};

struct PH_Shape
{
  F32   mass;
  F32   inv_mass;
  Vec2f position;
  Vec2f velocity;
  Vec2f acceleration;
  Vec2f sum_of_forces;

  F32 moment_of_inertia;
  F32 inv_moment_of_inertia;
  F32 angle;
  F32 angular_velocity;
  F32 angular_acceleration;
  F32 sum_of_torque;

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
  U32 count;
};

// --AlNov: Collision
struct PH_CollisionInfo
{
  PH_Shape* shape_a;
  PH_Shape* shape_b;

  // from a to b
  Vec2f normal;
  Vec2f start_point;
  Vec2f end_point;
  F32   depth;
};

// --AlNov: Constrain
struct PH_Constrain
{
  PH_Constrain* next;

  PH_Shape* shape_a;
  PH_Shape* shape_b;

  Vec2f point_a_space;
  Vec2f point_b_space;

  Vec2f ra;
  Vec2f rb;
  Vec2f n;
  F32   effective_mass;
  F32   cached_impulse_magnitude;
};

struct PH_ConstrainList
{
  PH_Constrain* first;
  PH_Constrain* last;
  U32           count;
};

// -------------------------------------------------------------------
// --AlNov: Forces ---------------------------------------------------
func Vec2f PH_CalculateWeight(F32 m, F32 g);
func Vec2f PH_CalculateDrag(Vec2f velocity, F32 k);
func Vec2f PH_CalculateSpring(Vec2f blob_position, Vec2f anchor_position, F32 rest_length, F32 k);

// -------------------------------------------------------------------
// --AlNov: Particle Operations --------------------------------------
func PH_Particle* PH_CreateParticle(Arena* arena, Vec2f position, F32 mass);

func void PH_ApplyForceToParticle(PH_Particle* particle, Vec2f force);
func void PH_ResetParticleForce(PH_Particle* particle);
func void PH_IntegrateParticle(PH_Particle* particle, F32 dt);

func void PH_PushParticle(PH_ParticleList* list, PH_Particle* particle);

// -------------------------------------------------------------------
// -AlNov: Shape Operations ------------------------------------------
func PH_Shape* PH_CreateCircleShape(Arena* arena, Vec2f position, F32 radius, F32 mass);
func PH_Shape* PH_CreateBoxShape(Arena* arena, Vec2f position, F32 height, F32 width, F32 mass);

func void PH_ApplyForceToShape(PH_Shape* shape, Vec2f force);
func void PH_ApplyImpulseToShape(PH_Shape* shape, Vec2f j, Vec2f apply_point);
func void PH_ApplyLinearImpulseToShape(PH_Shape* shape, Vec2f j);
func void PH_ApplyAngularImpulseToShape(PH_Shape* shape, F32 j);
func void PH_ApplyTorqueToShape(PH_Shape* shape, F32 torque);
func void PH_IntegrateShape(PH_Shape* shape, F32 dt);
func void PH_IntegrateForceShape(PH_Shape* shape, F32 dt);
func void PH_IntegrateVelocityShape(PH_Shape* shape, F32 dt);

func void PH_PushShapeList(PH_ShapeList* list, PH_Shape* shape);

// --AlNov: Shape Helpers
func bool  PH_IsStatic(PH_Shape* shape);
func Vec2f PH_LocalFromWorldSpace(PH_Shape* shape, Vec2f world_point);
func Vec2f PH_WorldFromLocalSpace(PH_Shape* shape, Vec2f local_point);
func Vec2f BoxVertexWorldFromLocal(PH_Shape* box, I32 vertex_index);

// -------------------------------------------------------------------
// --AlNov: Collision Operations -------------------------------------

// --AlNov: @TODO Create NIL object of PH_CollisionInfo
// And use it as collision result. If there is no collision - return NIL.
// It will get read of returning bool and inplace changes of out_coliision_info

func F32 PH_CalculateImpulseValue(PH_CollisionInfo* collision_info);

func bool PH_CheckCollision(PH_CollisionInfo* out_collision_info, PH_Shape* shape_a, PH_Shape* shape_b);
func bool PH_CircleCircleCollision(PH_CollisionInfo* out_collision_info, PH_Shape* circle_a, PH_Shape* circle_b);
func bool PH_BoxBoxCollision(PH_CollisionInfo* out_collision_info, PH_Shape* box_a, PH_Shape* box_b);
func bool PH_BoxCircleCollision(PH_CollisionInfo* out_collision_info, PH_Shape* box, PH_Shape* circle);

func void PH_ResolveCollisionProjection(PH_CollisionInfo* collision_info);
func void PH_ResolveCollisionImpulse(PH_CollisionInfo* collision_info);

// -------------------------------------------------------------------
// --AlNov: Constrains Operations ------------------------------------
func void          PH_PushConstrainList(PH_ConstrainList* list, PH_Constrain* constrain);
func PH_Constrain* PH_CreateDistanceConstrain(Arena* arena, PH_Shape* shape_a, PH_Shape* shape_b, Vec2f location);
func void          PH_PresolveConstrain(PH_Constrain* constrain, F32 dt);
func void          PH_SolveConstrain(PH_Constrain* constrain);
