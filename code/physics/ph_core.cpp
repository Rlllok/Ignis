#include "ph_core.h"

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
// --AlNov: Particle Operations --------------------------------------
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
// -AlNov: Shape Operations ------------------------------------------
func PH_Shape* PH_CreateCircleShape(Arena* arena, Vec2f position, f32 radius, f32 mass)
{
  PH_Shape* shape = (PH_Shape*)PushArena(arena, sizeof(PH_Shape));
  shape->type                  = PH_SHAPE_TYPE_CIRCLE;
  shape->mass                  = mass;
  shape->inv_mass              = (mass == 0.0f) ? 0.0f : (1.0f / mass);
  shape->position              = position;
  shape->velocity              = MakeVec2f(0.0f, 0.0f);
  shape->acceleration          = MakeVec2f(0.0f, 0.0f);
  shape->sum_of_forces         = MakeVec2f(0.0f, 0.0f);
  shape->moment_of_inertia     = 0.5f * mass * radius * radius;
  shape->inv_moment_of_inertia = (mass == 0.0f) ? 0.0f : 1.0f / shape->moment_of_inertia;
  shape->angle                 = 0.0f;
  shape->angular_velocity      = 0.0f;
  shape->angular_acceleration  = 0.0f;
  shape->sum_of_torque         = 0.0f;
  shape->circle.radius         = radius;
  shape->next                  = 0;
  return shape;
}

func PH_Shape* PH_CreateBoxShape(Arena* arena, Vec2f position, f32 height, f32 width, f32 mass)
{
  PH_Shape* shape = (PH_Shape*)PushArena(arena, sizeof(PH_Shape));
  shape->type                  = PH_SHAPE_TYPE_BOX;
  shape->mass                  = mass;
  shape->inv_mass              = (mass == 0.0f) ? 0.0f : (1.0f / mass);
  shape->position              = position;
  shape->velocity              = MakeVec2f(0.0f, 0.0f);
  shape->acceleration          = MakeVec2f(0.0f, 0.0f);
  shape->sum_of_forces         = MakeVec2f(0.0f, 0.0f);
  shape->moment_of_inertia     = 0.08334 * mass * (height*height + width*width);
  shape->inv_moment_of_inertia = (mass == 0.0f) ? 0.0f : 1.0f / shape->moment_of_inertia;
  shape->angle                 = 0.0f;
  shape->angular_velocity      = 0.0f;
  shape->angular_acceleration  = 0.0f;
  shape->sum_of_torque         = 0.0f;
  shape->box.height            = height;
  shape->box.width             = width;
  shape->box.vertecies[0]      = MakeVec2f(-width / 2.0f, -height / 2.0f);
  shape->box.vertecies[1]      = MakeVec2f(width / 2.0f, -height / 2.0f);
  shape->box.vertecies[2]      = MakeVec2f(width / 2.0f, height / 2.0f);
  shape->box.vertecies[3]      = MakeVec2f(-width / 2.0f, height / 2.0f);
  shape->next                  = 0;
  return shape;
}

func void PH_ApplyForceToShape(PH_Shape* shape, Vec2f force)
{
  shape->sum_of_forces = AddVec2f(shape->sum_of_forces, force);
}

func void PH_ApplyImpulseToShape(PH_Shape* shape, Vec2f j, Vec2f apply_point)
{
  if (PH_IsStatic(shape)) { return; }

  shape->velocity = AddVec2f(shape->velocity, MulVec2f(j, shape->inv_mass));
  shape->angular_velocity += CrossVec2f(apply_point, j) * shape->inv_moment_of_inertia;
}


func void PH_ApplyLinearImpulseToShape(PH_Shape* shape, Vec2f j)
{
  if (PH_IsStatic(shape)) { return; }

  shape->velocity = AddVec2f(shape->velocity, MulVec2f(j, shape->inv_mass));
}

func void PH_ApplyAngularImpulseToShape(PH_Shape* shape, f32 j)
{
  if (PH_IsStatic(shape)) { return; }

  shape->angular_velocity += j * shape->inv_moment_of_inertia;
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

func void PH_IntegrateForceShape(PH_Shape* shape, f32 dt)
{
  if (PH_IsStatic(shape)) { return; }

  // --AlNov: Euler Integration
  shape->acceleration         = MulVec2f(shape->sum_of_forces, shape->inv_mass);
  shape->velocity             = AddVec2f(shape->velocity, MulVec2f(shape->acceleration, dt));
  shape->sum_of_forces        = {};
  shape->angular_acceleration = shape->sum_of_torque * (1.0f / shape->moment_of_inertia);
  shape->angular_velocity     = shape->angular_velocity + shape->angular_acceleration * dt;
  shape->sum_of_torque        = 0.0f;
}

func void PH_IntegrateVelocityShape(PH_Shape* shape, f32 dt)
{
  if (PH_IsStatic(shape)) { return; }

  // --AlNov: Euler Integration
  shape->position = AddVec2f(shape->position, MulVec2f(shape->velocity, dt));
  shape->angle    = shape->angle + shape->angular_velocity * dt;
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

func bool PH_IsStatic(PH_Shape* shape)
{
  // --AlNov: @TODO This is float, so default comparison can be buggy.
  return shape->mass == 0.0f;
}

func Vec2f PH_LocalFromWorldSpace(PH_Shape* shape, Vec2f world_point)
{
  Vec2f result = world_point;
  result = SubVec2f(world_point, shape->position);
  result = RotateVec2f(result, -shape->angle);
  return result;
}

func Vec2f PH_WorldFromLocalSpace(PH_Shape* shape, Vec2f local_point)
{
  Vec2f result = local_point;
  result = RotateVec2f(local_point, shape->angle);
  result = AddVec2f(result, shape->position);
  return result;
}

func Vec2f BoxVertexWorldFromLocal(PH_Shape* box, i32 vertex_index)
{
  return AddVec2f(box->position, RotateVec2f(box->box.vertecies[vertex_index], box->angle));
}

// -------------------------------------------------------------------
// --AlNov: Collision Operations -------------------------------------
func f32 PH_CalculateImpulseValue(PH_CollisionInfo* collision_info)
{
  // --AlNov: @TODO Not Working :(
  PH_Shape* shape_a    = collision_info->shape_a;
  PH_Shape* shape_b    = collision_info->shape_b;
  f32       e          = 0.9f;
  Vec2f     vrel       = SubVec2f(collision_info->shape_a->velocity, collision_info->shape_b->velocity);
  f32       vrel_n_dot = DotVec2f(vrel, collision_info->normal);

  return -(e + 1) * vrel_n_dot / (shape_a->inv_mass + shape_b->inv_mass);
}

func bool PH_CheckCollision(PH_CollisionInfo* out_collision_info, PH_Shape* shape_a, PH_Shape* shape_b)
{
  // --AlNov: @TODO Don't check collision for two static objects for now.
  if (PH_IsStatic(shape_a) && PH_IsStatic(shape_b)) { return false; }

  if (shape_a->type == PH_SHAPE_TYPE_CIRCLE && shape_b->type == PH_SHAPE_TYPE_CIRCLE)
  {
    return PH_CircleCircleCollision(out_collision_info, shape_a, shape_b);
  }
  if (shape_a->type == PH_SHAPE_TYPE_BOX && shape_b->type == PH_SHAPE_TYPE_BOX)
  {
    return PH_BoxBoxCollision(out_collision_info, shape_a, shape_b);
  }
  if (shape_a->type == PH_SHAPE_TYPE_BOX && shape_b->type == PH_SHAPE_TYPE_CIRCLE)
  {
    return PH_BoxCircleCollision(out_collision_info, shape_a, shape_b);
  }
  if (shape_a->type == PH_SHAPE_TYPE_CIRCLE && shape_b->type == PH_SHAPE_TYPE_BOX)
  {
    return PH_BoxCircleCollision(out_collision_info, shape_b, shape_a);
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

struct SeparationResult
{
  f32   separation;
  Vec2f axis;
  Vec2f vertex;
};
func SeparationResult 
FindMinBoxBoxSeparation(PH_Shape* box_a, PH_Shape* box_b)
{
  SeparationResult result = {};
  result.separation = F32_MIN;

  for (i32 i = 0; i < 4; i += 1)
  {
    Vec2f vertex_a       = BoxVertexWorldFromLocal(box_a, i);
    Vec2f next_vertex_a  = BoxVertexWorldFromLocal(box_a, (i + 1) % 4);
    Vec2f normal         = NormalToVec2f(SubVec2f(next_vertex_a, vertex_a));
    f32   min_separation = F32_MAX;
    Vec2f min_vertex     = {};

    for (i32 j = 0; j < 4; j += 1)
    {
      Vec2f vertex_b   = BoxVertexWorldFromLocal(box_b, j);
      Vec2f sub_b_a    = SubVec2f(vertex_b, vertex_a);
      f32   projection = DotVec2f(sub_b_a, normal);

      if (projection < min_separation)
      {
        min_separation = projection;
        min_vertex     = vertex_b;
      }
    }

    if (result.separation < min_separation)
    {
      result.separation = min_separation;
      result.axis       = normal;
      result.vertex     = min_vertex;
    }
  }

  return result;
}

func bool PH_BoxBoxCollision(PH_CollisionInfo* out_collision_info, PH_Shape* box_a, PH_Shape* box_b)
{
  SeparationResult a_b_result = FindMinBoxBoxSeparation(box_a, box_b);
  SeparationResult b_a_result = FindMinBoxBoxSeparation(box_b, box_a);

  if (a_b_result.separation >= 0) { return false; }
  if (b_a_result.separation >= 0) { return false; }

  if (a_b_result.separation > b_a_result.separation)
  {
    out_collision_info->shape_a     = box_a;
    out_collision_info->shape_b     = box_b;
    out_collision_info->normal      = a_b_result.axis;
    out_collision_info->start_point = a_b_result.vertex;
    out_collision_info->end_point   = AddVec2f(a_b_result.vertex, MulVec2f(out_collision_info->normal, -a_b_result.separation));
    out_collision_info->depth       = -a_b_result.separation;
  }
  else
  {
    out_collision_info->shape_a     = box_a;
    out_collision_info->shape_b     = box_b;
    out_collision_info->normal      = MulVec2f(b_a_result.axis, -1.0f);
    out_collision_info->start_point = AddVec2f(b_a_result.vertex, MulVec2f(out_collision_info->normal, b_a_result.separation));
    out_collision_info->end_point   = b_a_result.vertex;
    out_collision_info->depth       = -b_a_result.separation;
  }

  return true;
}

func bool PH_BoxCircleCollision(PH_CollisionInfo* out_collision_info, PH_Shape* box, PH_Shape* circle)
{
  // --AlNov: That procedure is not pretty. Maybe there is better solution of this problem.

  f32   min_projection             = F32_MIN;
  Vec2f min_projection_vertex      = {};
  Vec2f min_projection_next_vertex = {};
  
  bool is_outside = false;

  for (i32 i = 0; i < 4; i += 1)
  {
    Vec2f current_vertex          = BoxVertexWorldFromLocal(box, i);
    Vec2f next_vertex             = BoxVertexWorldFromLocal(box, (i + 1) % 4);
    Vec2f edge                    = SubVec2f(next_vertex, current_vertex);
    Vec2f edge_normal             = NormalToVec2f(edge);
    Vec2f vertex_to_circle_center = SubVec2f(circle->position, current_vertex);
    f32   projection_value        = DotVec2f(vertex_to_circle_center, edge_normal);

    if (projection_value > 0.0f )
    {
      if (min_projection < 0.0f)
      {
        min_projection              = projection_value;
        min_projection_vertex       = current_vertex;
        min_projection_next_vertex  = next_vertex;
      }
      else
      {
        if (projection_value < min_projection)
        {
          min_projection              = projection_value;
          min_projection_vertex       = current_vertex;
          min_projection_next_vertex  = next_vertex;
        }
      }
    }
    else
    {
      if (min_projection < 0.0f)
      {
        if (projection_value > min_projection)
        {
          min_projection              = projection_value;
          min_projection_vertex       = current_vertex;
          min_projection_next_vertex  = next_vertex;
        }
      }
    }
  }

    Vec2f edge                       = SubVec2f(min_projection_next_vertex, min_projection_vertex);
    f32   edge_length                = MagnitudeVec2f(edge);
    Vec2f vertex_to_circle_center    = SubVec2f(circle->position, min_projection_vertex);
    f32   projection_to_edge         = DotVec2f(vertex_to_circle_center, NormalizeVec2f(edge));
    f32   projection_to_edge_clamped = Clamp(projection_to_edge, 0.0f, edge_length);
    Vec2f point_on_edge              = AddVec2f(min_projection_vertex, MulVec2f(NormalizeVec2f(edge), projection_to_edge_clamped));
    f32   circle_edge_distance       = MagnitudeVec2f(SubVec2f(circle->position, point_on_edge));

    if (circle_edge_distance > circle->circle.radius) { return false; }

    out_collision_info->shape_a     = box;
    out_collision_info->shape_b     = circle;
    out_collision_info->normal      = NormalizeVec2f(SubVec2f(circle->position, point_on_edge));
    if (min_projection < 0.0f)
    {
      out_collision_info->normal    = MulVec2f(out_collision_info->normal, -1.0f);
    }
    out_collision_info->depth       = circle->circle.radius - circle_edge_distance;
    out_collision_info->start_point = AddVec2f(circle->position, MulVec2f(out_collision_info->normal, -circle->circle.radius));
    out_collision_info->end_point   = point_on_edge;

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

  PH_Shape* shape_a = collision_info->shape_a;
  PH_Shape* shape_b = collision_info->shape_b;

  f32   e    = 0.5f;
  Vec2f ra   = SubVec2f(collision_info->end_point, shape_a->position);
  Vec2f rb   = SubVec2f(collision_info->start_point, shape_b->position);
  // --AlNov: va = VelocityAtCenter + w x r_a
  Vec2f va   = AddVec2f(shape_a->velocity, MakeVec2f(-shape_a->angular_velocity * ra.y, shape_a->angular_velocity * ra.x));
  Vec2f vb   = AddVec2f(shape_b->velocity, MakeVec2f(-shape_b->angular_velocity * rb.y, shape_b->angular_velocity * rb.x));
  Vec2f vrel = SubVec2f(va, vb);

  Vec2f j = {};

  // --AlNov: Impulse along Normal of collison
  {
    f32   vrel_dot_normal     = DotVec2f(vrel, collision_info->normal);
    f32   ra_cross_normal_sqr = CrossVec2f(ra, collision_info->normal) * CrossVec2f(ra, collision_info->normal);
    f32   rb_cross_normal_sqr = CrossVec2f(rb, collision_info->normal) * CrossVec2f(rb, collision_info->normal);
    f32   j_magnitude         = -(1.0f + e) * vrel_dot_normal / ((shape_a->inv_mass + shape_b->inv_mass) + (ra_cross_normal_sqr * shape_a->inv_moment_of_inertia) + (rb_cross_normal_sqr * shape_b->inv_moment_of_inertia));
    Vec2f jn                  = MulVec2f(collision_info->normal, j_magnitude);
    
    j = AddVec2f(j, jn);
  }
  // --AlNov: Impulse along Tangent of collision (Friction)
  {
    f32   f                    = 0.3f;
    Vec2f tangent              = NormalToVec2f(collision_info->normal);
    f32   vrel_dot_tangent     = DotVec2f(vrel, tangent);
    f32   ra_cross_tangent_sqr = CrossVec2f(ra, tangent) * CrossVec2f(ra, tangent);
    f32   rb_cross_tangent_sqr = CrossVec2f(rb, tangent) * CrossVec2f(rb, tangent);
    f32   j_magnitude          = -(1.0f + f) * vrel_dot_tangent / ((shape_a->inv_mass + shape_b->inv_mass) + (ra_cross_tangent_sqr * shape_a->inv_moment_of_inertia) + (rb_cross_tangent_sqr * shape_b->inv_moment_of_inertia));
    Vec2f jf                   = MulVec2f(tangent, j_magnitude);

    j = AddVec2f(j, jf);
  }

  PH_ApplyImpulseToShape(shape_a, j, ra);
  PH_ApplyImpulseToShape(shape_b, MulVec2f(j, -1.0f), rb);
}

// -------------------------------------------------------------------
// --AlNov: Constrains Operations ------------------------------------
func void PH_PushConstrainList(PH_ConstrainList* list, PH_Constrain* constrain)
{
  if (list->count == 0)
  {
    list->first =  constrain;
    list->last  =  constrain;
    list->count += 1;

    constrain->next = 0;
  }
  else
  {
    constrain->next  =  0;
    list->last->next =  constrain;
    list->last       =  constrain;
    list->count      += 1;
  };
}

func PH_Constrain* PH_CreateDistanceConstrain(Arena* arena, PH_Shape* shape_a, PH_Shape* shape_b, Vec2f location)
{
  PH_Constrain* constrain  = (PH_Constrain*)PushArena(arena, sizeof(PH_Constrain));
  constrain->shape_a       = shape_a;
  constrain->shape_b       = shape_b;
  constrain->point_a_space = PH_LocalFromWorldSpace(shape_a, location);
  constrain->point_b_space = PH_LocalFromWorldSpace(shape_b, location);

  return constrain;
}

func void PH_PresolveConstrain(PH_Constrain* constrain, f32 dt)
{
  PH_Shape* shape_a = constrain->shape_a;
  PH_Shape* shape_b = constrain->shape_b;

  Vec2f ra                 = RotateVec2f(constrain->point_a_space, shape_a->angle);
  Vec2f rb                 = RotateVec2f(constrain->point_b_space, shape_b->angle);
  Vec2f n                  = NormalizeVec2f(SubVec2f(AddVec2f(shape_a->position, ra), AddVec2f(shape_b->position, rb)));
  f32   n_cross_ra_sqr     = CrossVec2f(n, ra) * CrossVec2f(n, ra);
  f32   n_cross_rb_sqr     = CrossVec2f(n, rb) * CrossVec2f(n, rb);
  f32   inv_effective_mass = shape_a->inv_mass + shape_b->inv_mass + shape_a->inv_moment_of_inertia*n_cross_ra_sqr + shape_b->inv_moment_of_inertia*n_cross_rb_sqr;
  f32   effective_mass     = inv_effective_mass != 0.0f ? 1.0f / inv_effective_mass : 0.0f;
  
  Vec2f j = MulVec2f(n, constrain->cached_impulse_magnitude);
  PH_ApplyLinearImpulseToShape(shape_a, MulVec2f(j, -1.0f));
  PH_ApplyAngularImpulseToShape(shape_a, -CrossVec2f(ra, j));
  PH_ApplyLinearImpulseToShape(shape_b, j);
  PH_ApplyAngularImpulseToShape(shape_b, CrossVec2f(rb, j));

  constrain->ra = ra;
  constrain->rb = rb;
  constrain->n = n;
  constrain->effective_mass = effective_mass;
}

func void PH_SolveConstrain(PH_Constrain* constrain)
{
  PH_Shape* shape_a = constrain->shape_a;
  PH_Shape* shape_b = constrain->shape_b;
  Vec2f     ra      = constrain->ra;
  Vec2f     rb      = constrain->rb;

  Vec2f va          = AddVec2f(shape_a->velocity, MulVec2f(MakeVec2f(-ra.y, ra.x), shape_a->angular_velocity));
  Vec2f vb          = AddVec2f(shape_b->velocity, MulVec2f(MakeVec2f(-rb.y, rb.x), shape_b->angular_velocity));
  f32   cdot        = DotVec2f(constrain->n, SubVec2f(vb, va));
  f32   j_magnitude = -constrain->effective_mass* cdot;
  Vec2f j           = MulVec2f(constrain->n, j_magnitude);

  PH_ApplyLinearImpulseToShape(shape_a, MulVec2f(j, -1.0f));
  PH_ApplyAngularImpulseToShape(shape_a, -CrossVec2f(ra, j));
  PH_ApplyLinearImpulseToShape(shape_b, j);
  PH_ApplyAngularImpulseToShape(shape_b, CrossVec2f(rb, j));

  constrain->cached_impulse_magnitude += j_magnitude;
}
