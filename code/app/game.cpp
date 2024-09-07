#pragma comment(lib, "Winmm.lib")

// --AlNov: .h -------------------------------------------------------
#include "../base/base_include.h"
#include "../os/os_include.h"
#include "../render/r_include.h"
#include "../physics/ph_core.h"

// --AlNov: .cpp -----------------------------------------------------
#include "../base/base_include.cpp"
#include "../os/os_include.cpp"
#include "../render/r_include.cpp"
#include "../physics/ph_core.cpp"

// --AlNov: @TODO List
// * PH Box and draw box have different sizes
// * After some time there is a crash (First idea - Vulkan memory) - Fixed by itself for now
// * Player brick goes out of window (Only left side)

enum EntetyType
{
  ENTETY_TYPE_NONE,
  ENTETY_TYPE_PLAYER,
  ENTETY_TYPE_BALL,
  ENTETY_TYPE_BRICK,
  ENTETY_TYPE_WALL,

  ENTETY_TYPE_COUNT
};

struct PhysicsComponent
{
  PH_Shape* collision_shape;
};

struct CircleComponent
{
  f32   radius;
  Vec3f color;
};

struct BoxComponent
{
  Vec2f size;
  Vec3f color;
};

#define INVALID_ID        U32_MAX
#define MAX_ENTETY_COUNT  256

struct ComponentArray
{
  void* array;
  u32   type_size;
  u32   count;

  u32 lookup_table[MAX_ENTETY_COUNT];
};

#define InitComponentArray(arena, type, component_array)                          \
{                                                                                 \
  component_array.array     = PushArena(arena, sizeof(type) * MAX_ENTETY_COUNT);  \
  component_array.type_size = sizeof(type);                                       \
  component_array.count     = 0;                                                  \
}                                                                                 \

#define GetComponentFromArray(type, component_array, entety_id) ((type*)component_array.array + component_array.lookup_table[entety_id])
global struct GameState
{
  Arena* arena;
  Arena* frame_arena;

  OS_Window window;

  R_Pipeline ball_pipeline;
  R_Pipeline block_pipeline;

  b8 quit_game;
  b8 throw_ball;

  u32                 enteties[MAX_ENTETY_COUNT];
  u32                 entety_count;
  ComponentArray      circle_components;
  ComponentArray      box_components;
  ComponentArray      physics_components;

  u32 player_id;
  u32 ball_id;

  u32 bricks_ids[MAX_ENTETY_COUNT];
} game_state;

func void InitGameState(GameState* game_state);
func void InitPipelines(GameState* game_state);
func void HandleEvents(GameState* game_state);
func void Update(GameState* game_state, f32 delta_time);
func void Draw(GameState* game_state, f32 delta_time);
func void DrawBox(GameState* game_state, Vec2f position, Vec2f size, Vec3f color);
func void DrawCircle(GameState* game_state, Vec2f position, f32 radius, Vec3f color);

func u32  CreateEntety(GameState* game_state);
func void CreateCircleComponent(GameState* game_state, u32 entety_id, f32 radius, Vec3f color);
func void CreateBoxComponent(GameState* game_state, u32 entety_id, Vec2f radius, Vec3f color);
func void CreatePhysicsComponent(GameState* game_state, u32 entety_id, PH_Shape* shape);

func void RemoveEntety(GameState* game_state, u32 entety_id);

int main()
{
  InitGameState(&game_state);

  f32 delta_time = 1.0f / 60.0f;
  while (!game_state.quit_game)
  {
    f32 begin_time = OS_CurrentTimeSeconds();

    HandleEvents(&game_state);
    Update(&game_state, delta_time);
    Draw(&game_state, delta_time);

    f32 end_time  = OS_CurrentTimeSeconds();
    f32 wait_time = delta_time - (end_time - begin_time);
    if (wait_time > 0) { OS_Wait(wait_time); }
  }
};

func void InitGameState(GameState* game_state)
{
  *game_state = {};

  game_state->window = OS_CreateWindow("Game", MakeVec2u(1280, 720));
  R_Init(&game_state->window);
  OS_ShowWindow(&game_state->window);

  game_state->arena       = AllocateArena(Megabytes(128));
  game_state->frame_arena = AllocateArena(Megabytes(128));

  InitComponentArray(game_state->arena, CircleComponent, game_state->circle_components);
  InitComponentArray(game_state->arena, BoxComponent, game_state->box_components);
  InitComponentArray(game_state->arena, PhysicsComponent, game_state->physics_components);

  InitPipelines(game_state);

  // --AlNov: Add Player
  {
    Vec2f size     = MakeVec2f(150.0f, 15.0f);
    Vec2f position = MakeVec2f(game_state->window.width / 2.0f, game_state->window.height - 20.0f);
    Vec3f color    = MakeVec3f(1.0f, 1.0f, 1.0f);

    PH_Shape* ph_shape = PH_CreateBoxShape(
      game_state->arena,
      position,
      size.y,
      size.x,
      10000.0f
    );
    u32 entety_id = CreateEntety(game_state);
    CreateBoxComponent(game_state, entety_id, size, color);
    CreatePhysicsComponent(game_state, entety_id, ph_shape);

    game_state->player_id = entety_id;
  }

  // --AlNov: Add ball
  {
    f32   radius   = 15.0f;
    Vec2f position = AddVec2f(GetComponentFromArray(PhysicsComponent, game_state->physics_components, game_state->player_id)->collision_shape->position, MakeVec2f(0.0f, -20.0f));
    Vec3f color    = MakeVec3f(1.0f, 1.0f, 1.0f);

    PH_Shape* ph_shape = PH_CreateCircleShape(
      game_state->arena,
      position,
      radius,
      1.0f
    );
    u32 entety_id = CreateEntety(game_state);
    CreateCircleComponent(game_state, entety_id, radius, color);
    CreatePhysicsComponent(game_state, entety_id, ph_shape);

    game_state->ball_id = entety_id;
    game_state->throw_ball = true;
    PH_ApplyLinearImpulseToShape(ph_shape, MakeVec2f(0.0f, -1500.0f));
  }

  // --AlNov: Left Wall
  {
    Vec2f size     = MakeVec2f(15.0f, game_state->window.height * 2.0f);
    Vec2f position = MakeVec2f(0.0f, game_state->window.height / 2.0f);
    Vec3f color    = MakeVec3f(1.0f, 1.0f, 1.0f);

    PH_Shape* ph_shape = PH_CreateBoxShape(
      game_state->arena,
      position,
      size.y,
      size.x,
      0.0f
    );
    u32 entety_id = CreateEntety(game_state);
    CreateBoxComponent(game_state, entety_id, size, color);
    CreatePhysicsComponent(game_state, entety_id, ph_shape);
  }

  // --AlNov: Right Wall
  {
    Vec2f size     = MakeVec2f(15.0f, game_state->window.height * 2.0f);
    Vec2f position = MakeVec2f(game_state->window.width - 15.0f, game_state->window.height / 2.0f);
    Vec3f color    = MakeVec3f(1.0f, 1.0f, 1.0f);

    PH_Shape* ph_shape = PH_CreateBoxShape(
      game_state->arena,
      position,
      size.y,
      size.x,
      0.0f
    );
    u32 entety_id = CreateEntety(game_state);
    CreateBoxComponent(game_state, entety_id, size, color);
    CreatePhysicsComponent(game_state, entety_id, ph_shape);
  }

  // --AlNov: Top Wall
  {
    Vec2f size     = MakeVec2f(game_state->window.width * 2.0f, 15.0f);
    Vec2f position = MakeVec2f(game_state->window.width / 2.0f, 15.0f);
    Vec3f color    = MakeVec3f(1.0f, 1.0f, 1.0f);

    PH_Shape* ph_shape = PH_CreateBoxShape(
      game_state->arena,
      position,
      size.y,
      size.x,
      0.0f
    );
    u32 entety_id = CreateEntety(game_state);
    CreateBoxComponent(game_state, entety_id, size, color);
    CreatePhysicsComponent(game_state, entety_id, ph_shape);
  }

  for (i32 i = 0; i < MAX_ENTETY_COUNT; i += 1)
  {
    game_state->bricks_ids[i] = INVALID_ID;
  }

  // --AlNov: Add Bricks
  // for (i32 i = 0; i < 4; i += 1)
  // {
  //   Vec2f size     = MakeVec2f(75.0f, 15.0f);
  //   Vec2f position = MakeVec2f(150.0f + i * (size.x + 10.0f), 100.0f);
  //   Vec3f color    = MakeVec3f(1.0f, 1.0f, 1.0f);

  //   PH_Shape* ph_shape = PH_CreateBoxShape(
  //     game_state->arena,
  //     position,
  //     size.y,
  //     size.x,
  //     0.0f
  //   );
  //   u32 entety_id = CreateEntety(game_state);
  //   CreateBoxComponent(game_state, entety_id, size, color);
  //   CreatePhysicsComponent(game_state, entety_id, ph_shape);

  //   game_state->bricks_ids[entety_id] = entety_id;
  // }
}

func void InitPipelines(GameState* game_state)
{
  {
    R_VertexAttributeFormat attributes[] = {
      R_VERTEX_ATTRIBUTE_FORMAT_VEC3F,
      R_VERTEX_ATTRIBUTE_FORMAT_VEC3F,
      R_VERTEX_ATTRIBUTE_FORMAT_VEC2F
    };

    R_PipelineAssignAttributes(&game_state->ball_pipeline, attributes, CountArrayElements(attributes));

    R_BindingInfo bindings[] =
    {
      {R_BINDING_TYPE_UNIFORM_BUFFER, R_SHADER_TYPE_VERTEX},
    };
    R_PipelineAssignBindingLayout(&game_state->ball_pipeline, bindings, CountArrayElements(bindings));

    R_H_LoadShader(game_state->arena, "data/shaders/sdf/sdf_circle.vert", "main", R_SHADER_TYPE_VERTEX, &game_state->ball_pipeline.shaders[R_SHADER_TYPE_VERTEX]);
    R_H_LoadShader(game_state->arena, "data/shaders/sdf/sdf_circle.frag", "main", R_SHADER_TYPE_FRAGMENT, &game_state->ball_pipeline.shaders[R_SHADER_TYPE_FRAGMENT]);

    game_state->ball_pipeline.is_depth_test_enabled = false;

    R_CreatePipeline(&game_state->ball_pipeline);
  }

  // --AlNov: Pipeline to draw SDF Box
  {
    R_VertexAttributeFormat attributes[] = {
      R_VERTEX_ATTRIBUTE_FORMAT_VEC3F,
      R_VERTEX_ATTRIBUTE_FORMAT_VEC3F,
      R_VERTEX_ATTRIBUTE_FORMAT_VEC2F
    };

    R_PipelineAssignAttributes(&game_state->block_pipeline, attributes, CountArrayElements(attributes));

    R_BindingInfo bindings[] =
    {
      {R_BINDING_TYPE_UNIFORM_BUFFER, R_SHADER_TYPE_VERTEX},
    };
    R_PipelineAssignBindingLayout(&game_state->block_pipeline, bindings, CountArrayElements(bindings));

    R_H_LoadShader(game_state->arena, "data/shaders/sdf/sdf_box.vert", "main", R_SHADER_TYPE_VERTEX, &game_state->block_pipeline.shaders[R_SHADER_TYPE_VERTEX]);
    R_H_LoadShader(game_state->arena, "data/shaders/sdf/sdf_box.frag", "main", R_SHADER_TYPE_FRAGMENT, &game_state->block_pipeline.shaders[R_SHADER_TYPE_FRAGMENT]);

    game_state->block_pipeline.is_depth_test_enabled = false;

    R_CreatePipeline(&game_state->block_pipeline);
  }
}

func void HandleEvents(GameState* game_state)
{
  OS_EventList event_list = OS_GetEventList(game_state->frame_arena);

  OS_Event* event = event_list.first;
  while (event)
  {
    switch (event->type)
    {
      case OS_EVENT_TYPE_EXIT:
      {
        game_state->quit_game = true;
      } break;

      case OS_EVENT_TYPE_KEYBOARD:
      {
        switch (event->key)
        {
          case OS_KEY_ARROW_DOWN:
          {
            if (event->was_down)
            {
              game_state->throw_ball = !game_state->throw_ball;
            }
          }

          default: break;
        }
      } break;

      default: break;
    }

    event = event->next;
  }
}

func void Update(GameState* game_state, f32 delta_time)
{
  Vec2f mouse_position = OS_MousePosition(game_state->window);

  u32       player_id       = game_state->player_id;
  PH_Shape* player_ph_shape = GetComponentFromArray(PhysicsComponent, game_state->physics_components, player_id)->collision_shape;
  player_ph_shape->position.x = mouse_position.x;
  if (mouse_position.x < 0.0f)
  {
    player_ph_shape->position.x = 0.0f;
  }
  if (mouse_position.x > game_state->window.width)
  {
    player_ph_shape->position.x = game_state->window.width;
  }

  for (i32 i = 0; i < game_state->physics_components.count; i += 1)
  {
    for (i32 j = i; j < game_state->physics_components.count; j += 1)
    {
      PH_CollisionInfo collision_info = {};
      PH_Shape*        shape_a        = GetComponentFromArray(PhysicsComponent, game_state->physics_components, i)->collision_shape;
      PH_Shape*        shape_b        = GetComponentFromArray(PhysicsComponent, game_state->physics_components, j)->collision_shape;

      if (PH_CheckCollision(&collision_info, shape_a, shape_b))
      {
        PH_ResolveCollisionImpulse(&collision_info);

        bool some_is_brick = ((game_state->bricks_ids[i] != INVALID_ID) || 
                              (game_state->bricks_ids[j] != INVALID_ID));
        bool some_is_ball  = (game_state->ball_id == i || game_state->ball_id == j);
        if (some_is_ball && some_is_brick)
        {
          LOG_INFO("BALL HIT BRICK\n");
          u32 brick_entety_id = (game_state->bricks_ids[i] != INVALID_ID) ? i : j;
          // RemoveEntety(game_state, brick_entety_id);
        }
      }
    }
  }

  // --AlNov: Update Ball
  {

    PH_Shape* ball   = GetComponentFromArray(PhysicsComponent, game_state->physics_components, game_state->ball_id)->collision_shape;
    PH_Shape* player = GetComponentFromArray(PhysicsComponent, game_state->physics_components, game_state->player_id)->collision_shape;
    if (game_state->throw_ball)
    {
      ball->position.x = player->position.x;
      ball->position.y = player->position.y - ball->circle.radius * 2.0f;
    }
    else
    {
      PH_IntegrateShape(ball, delta_time);
    }
  }
}

func void Draw(GameState* game_state, f32 delta_time)
{
  Vec4f clear_color   = MakeVec4f(0.3f, 0.3f, 0.3f, 1.0f);
  f32   clear_depth   = 1.0f;
  f32   clear_stencil = 0.0f;

  // --AlNov: @NOTE Cannot understand how render using loop over box/circle components
  //                (that represent rendering component). The proble is that position
  //                is needed to render. But cannot be accessed as it is in a different
  //                component - PhysicsComponent in this case. So use loop over enteties
  //                for now.
  R_BeginFrame();
  {
    R_BeginRenderPass(clear_color, clear_depth, clear_stencil);
    {
      for (i32 i = 0; i < game_state->entety_count; i += 1)
      {
        PhysicsComponent* physics_component = GetComponentFromArray(PhysicsComponent, game_state->physics_components, i);
        Vec2f position = physics_component->collision_shape->position;

        BoxComponent* box_component = GetComponentFromArray(BoxComponent, game_state->box_components, i);
        Vec2f size     = MulVec2f(box_component->size, 0.5f);
        Vec3f color    = box_component->color;

        DrawBox(game_state, position, size, color);
      }

      for (i32 i = 0; i < game_state->entety_count; i += 1)
      {
        PhysicsComponent* physics_component = GetComponentFromArray(PhysicsComponent, game_state->physics_components, i);
        Vec2f position = physics_component->collision_shape->position;

        CircleComponent* circle_component = GetComponentFromArray(CircleComponent, game_state->circle_components, i);
        f32   radius   = circle_component->radius;
        Vec3f color    = circle_component->color;

        DrawCircle(game_state, position, radius, color);
      }
    }
    R_EndRenderPass();
  }
  R_EndFrame();
}

func void
DrawBox(GameState* game_state, Vec2f position, Vec2f size, Vec3f color)
{
  // --AlNov: @TODO should be created once
  static R_SceneObject* quad_object = (R_SceneObject*)PushArena(game_state->arena, sizeof(R_SceneObject));
  quad_object->vertex_count          = 4;
  quad_object->vertecies             = (R_SceneObject::Vertex*)PushArena(game_state->arena, sizeof(R_SceneObject::Vertex) * quad_object->vertex_count);
  quad_object->vertecies[0].position = MakeVec3f(-1.0f, -1.0f, 0.0f);
  quad_object->vertecies[1].position = MakeVec3f(1.0f, -1.0f, 0.0f);
  quad_object->vertecies[2].position = MakeVec3f(1.0f, 1.0f, 0.0f);
  quad_object->vertecies[3].position = MakeVec3f(-1.0f, 1.0f, 0.0f);
  quad_object->index_count           = 6;
  quad_object->indecies              = (u32*)PushArena(game_state->arena, sizeof(u32) * quad_object->index_count);
  quad_object->indecies[0]           = 0;
  quad_object->indecies[1]           = 1;
  quad_object->indecies[2]           = 2;
  quad_object->indecies[3]           = 2;
  quad_object->indecies[4]           = 3;
  quad_object->indecies[5]           = 0;

  struct UBO
  {
    alignas(16) Mat4x4f projection;
    alignas(8)  Vec2f   position;
    alignas(8)  Vec2f   size;
    alignas(16) Vec3f   color;
  };

  Vec2f resolution = MakeVec2f(1280.0f, 720.0f);

  UBO ubo = {};
  ubo.projection = MakeOrthographic4x4f(0.0f, resolution.x, 0.0f, resolution.y, 0.0f, 1.0f);
  ubo.position   = position;
  ubo.size       = size;
  ubo.color      = color;

  R_DrawInfo draw_info = {};
  draw_info.pipeline          = &game_state->block_pipeline;
  draw_info.vertecies         = quad_object->vertecies;
  draw_info.vertex_size       = sizeof(quad_object->vertecies[0]);
  draw_info.vertex_count      = quad_object->vertex_count;
  draw_info.indecies          = quad_object->indecies;
  draw_info.index_size        = sizeof(quad_object->indecies[0]);
  draw_info.index_count       = quad_object->index_count;
  draw_info.uniform_data      = &ubo;
  draw_info.uniform_data_size = sizeof(ubo);

  R_DrawSceneObject(&draw_info);
}

func void
DrawCircle(GameState* game_state, Vec2f position, f32 radius, Vec3f color)
{
  // --AlNov: @TODO should be created once
  static R_SceneObject* quad_object = (R_SceneObject*)PushArena(game_state->arena, sizeof(R_SceneObject));
  quad_object->vertex_count          = 4;
  quad_object->vertecies             = (R_SceneObject::Vertex*)PushArena(game_state->arena, sizeof(R_SceneObject::Vertex) * quad_object->vertex_count);
  quad_object->vertecies[0].position = MakeVec3f(-1.0f, -1.0f, 0.0f);
  quad_object->vertecies[1].position = MakeVec3f(1.0f, -1.0f, 0.0f);
  quad_object->vertecies[2].position = MakeVec3f(1.0f, 1.0f, 0.0f);
  quad_object->vertecies[3].position = MakeVec3f(-1.0f, 1.0f, 0.0f);
  quad_object->index_count           = 6;
  quad_object->indecies              = (u32*)PushArena(game_state->arena, sizeof(u32) * quad_object->index_count);
  quad_object->indecies[0]           = 0;
  quad_object->indecies[1]           = 1;
  quad_object->indecies[2]           = 2;
  quad_object->indecies[3]           = 2;
  quad_object->indecies[4]           = 3;
  quad_object->indecies[5]           = 0;

  struct UBO
  {
    alignas(16) Mat4x4f projection;
    alignas(8)  Vec2f   position;
    alignas(4)  f32     radius;
    alignas(16) Vec3f   color;
  };

  Vec2f resolution = MakeVec2f(1280.0f, 720.0f);

  UBO ubo = {};
  ubo.projection = MakeOrthographic4x4f(0.0f, resolution.x, 0.0f, resolution.y, 0.0f, 1.0f);
  ubo.position   = position;
  ubo.radius     = radius;
  ubo.color      = MakeVec3f(0.65f, 0.23f, 0.12f);

  R_DrawInfo draw_info = {};
  draw_info.pipeline          = &game_state->ball_pipeline;
  draw_info.vertecies         = quad_object->vertecies;
  draw_info.vertex_size       = sizeof(quad_object->vertecies[0]);
  draw_info.vertex_count      = quad_object->vertex_count;
  draw_info.indecies          = quad_object->indecies;
  draw_info.index_size        = sizeof(quad_object->indecies[0]);
  draw_info.index_count       = quad_object->index_count;
  draw_info.uniform_data      = &ubo;
  draw_info.uniform_data_size = sizeof(ubo);

  R_DrawSceneObject(&draw_info);
}

func u32
CreateEntety(GameState* game_state)
{
  assert(game_state->entety_count < MAX_ENTETY_COUNT);

  game_state->enteties[game_state->entety_count]  = game_state->entety_count;
  game_state->entety_count                       += 1;

  return game_state->entety_count - 1;
}

func void
CreateCircleComponent(GameState* game_state, u32 entety_id, f32 radius, Vec3f color)
{
  assert(entety_id < game_state->entety_count);

  u32              new_circle_id    = game_state->circle_components.count;
  CircleComponent* circle_component = (CircleComponent*)game_state->circle_components.array + new_circle_id;

  circle_component->radius = radius;
  circle_component->color  = color;
  
  game_state->circle_components.lookup_table[entety_id] = new_circle_id;
  game_state->circle_components.count += 1;
}

func void
CreateBoxComponent(GameState* game_state, u32 entety_id, Vec2f size, Vec3f color)
{
  assert(entety_id < game_state->entety_count);

  u32           new_box_id    = game_state->box_components.count;
  BoxComponent* box_component = (BoxComponent*)game_state->box_components.array + new_box_id;

  box_component->size  = size;
  box_component->color = color;

  game_state->box_components.lookup_table[entety_id] = new_box_id;
  game_state->box_components.count += 1;
}

func void
CreatePhysicsComponent(GameState* game_state, u32 entety_id, PH_Shape* shape)
{
  assert(entety_id < game_state->entety_count);

  u32               new_shape_id      = game_state->physics_components.count;
  PhysicsComponent* physics_component = (PhysicsComponent*)game_state->physics_components.array + new_shape_id;

  physics_component->collision_shape = shape;

  game_state->physics_components.lookup_table[entety_id] = new_shape_id;
  game_state->physics_components.count += 1;
}

func void
RemoveEntety(GameState* game_state, u32 entety_id)
{
  game_state->enteties[entety_id]                     = game_state->enteties[game_state->entety_count - 1];
  game_state->enteties[game_state->entety_count - 1]  = INVALID_ID;
  game_state->entety_count                           -= 1;
}
