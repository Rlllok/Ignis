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

struct Entety
{
  u32   id;
  Vec2f position;
  Vec2f size;
  f32   radius;
};

global struct GameState
{
  Arena* arena;
  Arena* frame_arena;

  OS_Window window;

  R_Pipeline ball_pipeline;
  R_Pipeline block_pipeline;

  b8 quit_game;

  Entety player;
  Entety ball;
  Entety block[256];
  u32 block_count;
} game_state;

func void InitGameState(GameState* game_state);
func void InitPipelines(GameState* game_state);
func void HandleEvents(GameState* game_state);
func void Update(GameState* game_state, f32 delta_time);
func void Draw(GameState* game_state, f32 delta_time);

int main()
{
  InitGameState(&game_state);

  while (!game_state.quit_game)
  {
    HandleEvents(&game_state);
    Update(&game_state, 0.0f);
    Draw(&game_state, 0.0f);
  }
};

func void InitGameState(GameState* game_state)
{
  *game_state = {};

  game_state->window = OS_CreateWindow("Game", MakeVec2u(1280, 720));
  R_Init(&game_state->window);
  OS_ShowWindow(&game_state->window);

  game_state->arena = AllocateArena(Megabytes(64));
  game_state->frame_arena = AllocateArena(Megabytes(128));

  InitPipelines(game_state);

  game_state->player.id       = 0;
  game_state->player.position = MakeVec2f(200.0f, 200.0f);
  game_state->player.size     = MakeVec2f(150.0f, 15.0f);
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

      default: break;
    }

    event = event->next;
  }
}

func void Update(GameState* game_state, f32 delta_time)
{
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
      ubo.position   = game_state->player.position;
      ubo.size       = game_state->player.size;
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
    R_EndRenderPass();
  }
  R_EndFrame();
}