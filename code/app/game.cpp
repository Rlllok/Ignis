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
// * Key event only on release

struct Player
{
  PH_Shape* collision_box;
};

struct Ball
{
  PH_Shape* collision_ball;
};

struct Wall
{
  PH_Shape* collision_box;
};

struct Brick
{
  u32   id;
  Vec2f position;
  Vec2f size;
};

global struct GameState
{
  Arena* arena;
  Arena* frame_arena;

  OS_Window window;

  R_Pipeline ball_pipeline;
  R_Pipeline block_pipeline;

  b8 quit_game;
  b8 throw_ball;

  Player  player;
  Ball    ball;
  Wall    walls[3];
  Brick   bricks[256];
  u32 block_count;

  PH_ShapeList collision_shapes;
} game_state;

func void InitGameState(GameState* game_state);
func void InitPipelines(GameState* game_state);
func void HandleEvents(GameState* game_state);
func void Update(GameState* game_state, f32 delta_time);
func void Draw(GameState* game_state, f32 delta_time);
func void DrawBox(GameState* game_state, Vec2f position, Vec2f size);

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

  game_state->arena       = AllocateArena(Megabytes(64));
  game_state->frame_arena = AllocateArena(Megabytes(128));

  InitPipelines(game_state);

  game_state->player.collision_box = PH_CreateBoxShape(
    game_state->arena,
    MakeVec2f(game_state->window.width / 2.0f, game_state->window.height - 20.0f),
    15.0f,
    150.f,
    10000.0f
  );
  PH_PushShapeList(&game_state->collision_shapes, game_state->player.collision_box);

  game_state->ball.collision_ball = PH_CreateCircleShape(
    game_state->arena,
    AddVec2f(game_state->player.collision_box->position, MakeVec2f(0.0f, -20.0f)),
    15.0f,
    1.0f
  );
  PH_PushShapeList(&game_state->collision_shapes, game_state->ball.collision_ball);
  game_state->throw_ball = true;
  PH_ApplyLinearImpulseToShape(game_state->ball.collision_ball, MakeVec2f(-1500.0f, -500.0f));

  // --AlNov: Left Wall
  game_state->walls[0].collision_box = PH_CreateBoxShape(
    game_state->arena,
    MakeVec2f(0.0f, game_state->window.height / 2.0f),
    game_state->window.height * 2.0f,
    15.0f,
    0.0f
  );
  PH_PushShapeList(&game_state->collision_shapes, game_state->walls[0].collision_box);

  // --AlNov: Right Wall
  game_state->walls[1].collision_box = PH_CreateBoxShape(
    game_state->arena,
    MakeVec2f(game_state->window.width - 15.0f, game_state->window.height / 2.0f),
    game_state->window.height * 2.0f,
    15.0f,
    0.0f
  );
  PH_PushShapeList(&game_state->collision_shapes, game_state->walls[1].collision_box);

  // --AlNov: Top Wall
  game_state->walls[2].collision_box = PH_CreateBoxShape(
    game_state->arena,
    MakeVec2f(game_state->window.width, 15.0f),
    15.0f,
    game_state->window.width * 2.0f,
    0.0f
  );
  PH_PushShapeList(&game_state->collision_shapes, game_state->walls[2].collision_box);
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

  game_state->player.collision_box->position.x = mouse_position.x;
  if (mouse_position.x < 0.0f)
  {
    game_state->player.collision_box->position.x = 0.0f;
  }
  if (mouse_position.x > game_state->window.width)
  {
    game_state->player.collision_box->position.x = game_state->window.width;
  }

  for (PH_Shape* shape_a = game_state->collision_shapes.first; shape_a; shape_a = shape_a->next)
  {
    for (PH_Shape* shape_b = shape_a->next; shape_b; shape_b = shape_b->next)
    {
      PH_CollisionInfo collision_info = {};
      if (PH_CheckCollision(&collision_info, shape_a, shape_b))
      {
        PH_ResolveCollisionImpulse(&collision_info);
      }
    }
  }

  // --AlNov: Update Ball
  {
    PH_Shape* ball = game_state->ball.collision_ball;
    if (game_state->throw_ball)
    {
      ball->position.x = game_state->player.collision_box->position.x;
      ball->position.y = game_state->player.collision_box->position.y - ball->circle.radius * 2.0f;
    }
    else
    {
      PH_IntegrateShape(ball, delta_time);
    }
  }
}

func void Draw(GameState* game_state, f32 delta_time)
{
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

  Vec4f clear_color   = MakeVec4f(0.3f, 0.3f, 0.3f, 1.0f);
  f32   clear_depth   = 1.0f;
  f32   clear_stencil = 0.0f;

  R_BeginFrame();
  {
    R_BeginRenderPass(clear_color, clear_depth, clear_stencil);
    {
      Vec2f player_size = MakeVec2f(game_state->player.collision_box->box.width / 2.0f, game_state->player.collision_box->box.height / 2.0f);
      DrawBox(game_state, game_state->player.collision_box->position, player_size);
      for (i32 i = 0; i < CountArrayElements(game_state->walls); i += 1)
      {
        Vec2f wall_size = MakeVec2f(game_state->walls[i].collision_box->box.width, game_state->walls[i].collision_box->box.height);
        DrawBox(game_state, game_state->walls[i].collision_box->position, wall_size);
      }

      // --AlNov: Draw ball
      {
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
        ubo.position   = game_state->ball.collision_ball->position;
        ubo.radius     = game_state->ball.collision_ball->circle.radius;
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
    }
    R_EndRenderPass();
  }
  R_EndFrame();
}

func void DrawBox(GameState* game_state, Vec2f position, Vec2f size)
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
  ubo.color      = MakeVec3f(0.65f, 0.23f, 0.12f);

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