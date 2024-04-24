#include "ph_core.h"

// -------------------------------------------------------------------
// -AlNov: Particles
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
// -AlNov: Shapes
func PH_Shape* PH_CreateCircleShape(Arena* arena, Vec2f position, f32 radius, f32 mass)
{
  PH_Shape* shape = (PH_Shape*)PushArena(arena, sizeof(PH_Shape));
  shape->type                 = PH_SHAPE_TYPE_CIRCLE;
  shape->mass                 = mass;
  shape->inv_mass             = mass ? 1.0f / mass : 0.0f;
  shape->position             = position;
  shape->velocity             = MakeVec2f(0.0f, 0.0f);
  shape->acceleration         = MakeVec2f(0.0f, 0.0f);
  shape->sum_of_forces        = MakeVec2f(0.0f, 0.0f);
  shape->moment_of_inertia    = 0.5f * mass * radius * radius;
  shape->angle                = 0.0f;
  shape->angular_velocity     = 0.0f;
  shape->angular_acceleration = 0.0f;
  shape->sum_of_torque        = 0.0f;
  shape->circle.radius        = radius;
  shape->next                 = 0;
  return shape;
}

func PH_Shape* PH_CreateBoxShape(Arena* arena, Vec2f position, f32 height, f32 width, f32 mass)
{
  PH_Shape* shape = (PH_Shape*)PushArena(arena, sizeof(PH_Shape));
  shape->type                 = PH_SHAPE_TYPE_BOX;
  shape->mass                 = mass;
  shape->position             = position;
  shape->velocity             = MakeVec2f(0.0f, 0.0f);
  shape->acceleration         = MakeVec2f(0.0f, 0.0f);
  shape->sum_of_forces        = MakeVec2f(0.0f, 0.0f);
  shape->moment_of_inertia    = 0.08334 * mass * (height*height + width*width);
  shape->angle                = 0.0f;
  shape->angular_velocity     = 0.0f;
  shape->angular_acceleration = 0.0f;
  shape->sum_of_torque        = 0.0f;
  shape->box.height           = height;
  shape->box.width            = width;
  shape->next                 = 0;
  return shape;
}

func void PH_ApplyForceToShape(PH_Shape* shape, Vec2f force)
{
  shape->sum_of_forces = AddVec2f(shape->sum_of_forces, force);
}

func void PH_ApplyImpulseToShape(PH_Shape* shape, Vec2f j)
{
  if (PH_IsStatic(shape)) { return; }

  shape->velocity = AddVec2f(shape->velocity, MulVec2f(j, shape->inv_mass));
}

func void PH_ApplyTorqueToShape(PH_Shape* shape, f32 torque)
{
  shape->sum_of_torque += torque;
}

func void PH_IntegrateShape(PH_Shape* shape, f32 dt)
{
  if (PH_IsStatic(shape)) { return; }

  // --AlNov: Euler Integration
  shape->acceleration   = MulVec2f(shape->sum_of_forces, shape->inv_mass);
  shape->velocity       = AddVec2f(shape->velocity, MulVec2f(shape->acceleration, dt));
  shape->position       = AddVec2f(shape->position, MulVec2f(shape->velocity, dt));
  shape->sum_of_forces  = {};
  
  shape->angular_acceleration = shape->sum_of_torque * (1.0f / shape->moment_of_inertia);
  shape->angular_velocity     = shape->angular_velocity + shape->angular_acceleration * dt;
  shape->angle                = shape->angle + shape->angular_velocity * dt;
  shape->sum_of_torque        = 0.0f;
}

func void PH_PushShapeList(PH_ShapeList* list, PH_Shape* shape)
{
  if (list->count == 0)
  {
    list->first = shape;
    list->last  = shape;
    list->count += 1;

    shape->next = 0;
  }
  else
  {
    shape->next       = 0;
    list->last->next  = shape;
    list->last        = shape;
    list->count       += 1;
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
  Vec2f distance               = SubVec2f(blob_position, anchor_position);
  Vec2f direction              = NormalizeVec2f(distance);
  f32   spring_force_magnitude = (rest_length - MagnitudeVec2f(distance)) * k;
  Vec2f spring_force           = MulVec2f(direction, spring_force_magnitude);
  return spring_force;
}

// -------------------------------------------------------------------
// --AlNov: Helpers --------------------------------------------------
func bool PH_IsStatic(PH_Shape* shape)
{
  // --AlNov: @TODO This is float, so default comparison can be buggy.
  return shape->mass == 0.0f;
}

// -------------------------------------------------------------------
// --AlNov: Collision ------------------------------------------------
func f32 PH_CalculateImpulseValue(PH_CollisionInfo* collision_info)
{
  // --AlNov: @TODO Not Working :(
  PH_Shape* shape_a    = collision_info->shape_a;
  PH_Shape* shape_b    = collision_info->shape_b;
  f32       e          = 0.9f;
  Vec2f     vrel       = SubVec2f(collision_info->shape_a->velocity, collision_info->shape_b->velocity);
  f32       vrel_n_dot = Dot2f32(vrel, collision_info->normal);

  return -(e + 1) * vrel_n_dot / (shape_a->inv_mass + shape_b->inv_mass);
}

func bool PH_CheckCollision(PH_CollisionInfo* out_collision_info, PH_Shape* shape_a, PH_Shape* shape_b)
{
  if (shape_a->type == PH_SHAPE_TYPE_CIRCLE && shape_b->type == PH_SHAPE_TYPE_CIRCLE)
  {
    return PH_CircleCircleCollision(out_collision_info, shape_a, shape_b);
  }

  return false;
}

func bool PH_CircleCircleCollision(PH_CollisionInfo* out_collision_info, PH_Shape* circle_a, PH_Shape* circle_b)
{
  f32   radius_a          = circle_a->circle.radius;
  f32   radius_b          = circle_b->circle.radius;
  f32   radius_sum_sqr    = (radius_a + radius_b) * (radius_a + radius_b);
  Vec2f distance_vector   = SubVec2f(circle_a->position, circle_b->position);
  f32   distance_sqr      = MagnitudeSquareVec2f(distance_vector);

  if (distance_sqr > radius_sum_sqr)
  {
    out_collision_info = {};
    return false;
  }

  out_collision_info->shape_a     = circle_a;
  out_collision_info->shape_b     = circle_b;
  out_collision_info->normal      = NormalizeVec2f(SubVec2f(circle_b->position, circle_a->position));
  out_collision_info->start_point = AddVec2f(circle_b->position, MulVec2f(out_collision_info->normal, -circle_b->circle.radius));
  out_collision_info->end_point   = AddVec2f(circle_a->position, MulVec2f(out_collision_info->normal, circle_a->circle.radius));
  out_collision_info->depth       = MagnitudeVec2f(SubVec2f(out_collision_info->start_point, out_collision_info->end_point));

  return true;
}

func void PH_ResolveCollisionProjection(PH_CollisionInfo* collision_info)
{
  PH_Shape* shape_a = collision_info->shape_a;
  PH_Shape* shape_b = collision_info->shape_b;

  if (PH_IsStatic(shape_a) && PH_IsStatic(shape_b)) { return; }

  f32 dp_a = collision_info->depth / (shape_a->inv_mass + shape_b->inv_mass) * shape_a->inv_mass;
  f32 dp_b = collision_info->depth / (shape_a->inv_mass + shape_b->inv_mass) * shape_b->inv_mass;

  shape_a->position = AddVec2f(shape_a->position, MulVec2f(collision_info->normal, -dp_a));
  shape_b->position = AddVec2f(shape_b->position, MulVec2f(collision_info->normal, dp_b));
}

func void PH_ResolveCollisionImpulse(PH_CollisionInfo* collision_info)
{
  PH_ResolveCollisionProjection(collision_info);

  PH_Shape* shape_a     = collision_info->shape_a;
  PH_Shape* shape_b     = collision_info->shape_b;
  f32       e           = 0.9f;
  Vec2f     vrel        = SubVec2f(shape_a->velocity, shape_b->velocity);
  f32       vrel_n_dot  = Dot2f32(vrel, collision_info->normal);
  f32       j_magnitude = -(e + 1) * vrel_n_dot / (shape_a->inv_mass + shape_b->inv_mass);
  Vec2f     j_a         = MulVec2f(collision_info->normal, j_magnitude);
  Vec2f     j_b         = MulVec2f(collision_info->normal, -j_magnitude);

  PH_ApplyImpulseToShape(shape_a, j_a);
  PH_ApplyImpulseToShape(shape_b, j_b);
}
