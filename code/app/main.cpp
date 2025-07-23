#include "base/base_include.h"
#include "os/os_include.h"
#include "render/r_include.h"
#include "draw/d_include.h"
#include "assets/mesh.h"

#include "base/base_include.cpp"
#include "os/os_include.cpp"
#include "render/r_include.cpp"
#include "draw/d_include.cpp"
#include "assets/mesh.cpp"
#include "render/r_pipeline.h"

struct Camera
{
  Vec3f position;
  Vec3f front;
  Vec3f up;
  Vec3f right;
  F32 speed;
  F32 yaw;
  F32 pitch;
};

struct AppState
{
  Arena* arena;
  Arena* frame_arena;
  OS_Window window;

  Camera camera;
  F32 delta_time;

  B32 is_window_closed;

  Vec2f last_mouse_position;
} app_state;

func void HandleEvents(Arena* arena, AppState* state);

I32 main()
{
  // https://docs.vulkan.org/spec/latest/chapters/cmdbuffers.html#VUID-vkResetCommandBuffer-commandBuffer-00046
  app_state = {};
  app_state.arena = AllocateArena(Megabytes(64));
  app_state.frame_arena = AllocateArena(Megabytes(8));
  app_state.is_window_closed = false;
  app_state.camera = {};
  app_state.camera.position = MakeVec3f(0.0f, 0.0f, 4.0f),
  app_state.camera.front = NormalizeVec3f(MakeVec3f(0.0f, 0.0f, -1.0f));
  app_state.camera.up = NormalizeVec3f(MakeVec3f(0.0f, 1.0f, 0.0f));
  app_state.camera.right = NormalizeVec3f(CrossVec3f(app_state.camera.front, app_state.camera.up));
  app_state.camera.speed = 1.0f;

  F32 new_variable = 0;
 
  OS_Init(Megabytes(32));
  OS_CreateWindow("Vulkan Triangle", MakeVec2u(1270, 720), &app_state.window);
  OS_ShowWindow(&app_state.window);

  R_Init(R_RENDERER_TYPE_VULKAN, &app_state.window);
  D_Init(Megabytes(32));

  R_Pipeline bline_pipeline= {};
  {
    R_VertexAttributeFormat attributes[] = {
      R_VERTEX_ATTRIBUTE_FORMAT_VEC2F
    };
    R_PipelineAssignAttributes(&bline_pipeline, attributes, CountArrayElements(attributes));

    R_BindingInfo scene_bindings[] = {
      {R_BINDING_TYPE_UNIFORM_BUFFER, R_SHADER_TYPE_VERTEX},
    };
    R_PipelineAssignSceneBindingLayout(&bline_pipeline, scene_bindings, CountArrayElements(scene_bindings));

    R_H_LoadShader(app_state.arena, "data/shaders/square.vs.glsl",
                   "main", R_SHADER_TYPE_VERTEX,
                   &bline_pipeline.shaders[R_SHADER_TYPE_VERTEX]);
    R_H_LoadShader(app_state.arena, "data/shaders/bline.fs.glsl",
                   "main", R_SHADER_TYPE_FRAGMENT,
                   &bline_pipeline.shaders[R_SHADER_TYPE_FRAGMENT]);
  
    bline_pipeline.is_back_culing_enabled = false;
    bline_pipeline.is_depth_test_enabled = false;
    Renderer.CreatePipeline(&bline_pipeline);
  }
  
  F32 begin_time = OS_CurrentTimeSeconds();
  while (!app_state.is_window_closed)
  {
    HandleEvents(app_state.frame_arena, &app_state);

    Renderer.BeginFrame();
    {
      Renderer.BeginRenderPass(R_ATTACHMENT_LOAD_OPERATION_CLEAR, MakeVec4f(0.3f, 0.3f, 0.3f, 1.0f));
      {
          D_DrawBezierCubic(ZeroVec2I(), MakeVec2I(300, 400), MakeVec2I(100, 100), MakeVec2I(800, 400), MakeVec3f(1.0f, 0.0f, 0.0f));
          D_DrawBezierCubic(MakeVec2I(210, 95), MakeVec2I(300, 400), MakeVec2I(400, 400), MakeVec2I(1100, 550), MakeVec3f(1.0f, 1.0f, 0.0f));
      }
    }
    Renderer.EndFrame();
    ResetArena(app_state.frame_arena);

    F32 end_time = OS_CurrentTimeSeconds();
    app_state.delta_time = end_time - begin_time;
    begin_time = end_time;
  }

  R_Shutdown();
  
  return 0;
}

func void
HandleEvents(Arena* arena, AppState* state)
{
  ListOS_Event event_list = OS_GetEventList(arena, &app_state.window);
  
  for (ListNodeOS_Event *event_node = event_list.first; event_node; event_node = event_node->next)
  {
    OS_Event* event = &event_node->data;

    switch (event->type)
    {
      case OS_EVENT_TYPE_EXIT:
      {
        state->is_window_closed = true;
      } break;

      case OS_EVENT_TYPE_RESIZE:
      {
        // @TODO Window recreated multiple time.
        // I guess, resize event is triggered multiple time. It should be handled only once, after last resizing
        state->window.size = event->window_size;
        // LOG_INFO("New window size: %d w %d h\n\n", state->window.size.x, state->window.size.y);
        Renderer.HandleResize(&state->window);
      } break;

      case OS_EVENT_TYPE_MOUSE_MOVE:
      {
        LOG_INFO("MousePosition: %.3f, %.3f\n", event->mouse_position.x, event->mouse_position.y);
        LOG_INFO("Virtual Cursor: %.3f, %.3f\n", state->window.virtual_cursor_position.x, state->window.virtual_cursor_position.y);
        Vec2f d_position = state->window.virtual_cursor_position - state->last_mouse_position;
        Vec2f mouse_direction = NormalizeVec2f(d_position);

        state->camera.yaw += mouse_direction.x * 1.0f * state->delta_time;
        // state->camera.pitch += dy * 0.1f;

        Vec3f direction = {};
        direction.x = cos(state->camera.yaw) * cos(state->camera.pitch);
        direction.y = sin(state->camera.pitch);
        direction.z = sin(state->camera.yaw) * cos(state->camera.pitch);
        // state->camera.front = NormalizeVec3f(direction);
        // state->camera.right = NormalizeVec3f(CrossVec3f(state->camera.front, MakeVec3f(0.0f, 1.0f, 0.0f)));
        // state->camera.up = NormalizeVec3f(CrossVec3f(state->camera.right, state->camera.front));
        
        state->last_mouse_position = state->window.virtual_cursor_position;
      } break;

      case OS_EVENT_TYPE_KEYBOARD:
      {
        if (event->key == OS_KEY_ARROW_UP)
        {
          {
            state->camera.position = state->camera.position + state->camera.speed * state->camera.front;
          }
        }
        if (event->key == OS_KEY_ARROW_DOWN)
        {
          {
            state->camera.position = state->camera.position - state->camera.speed * state->camera.front;
          }
        }
        if (event->key == OS_KEY_ARROW_RIGHT)
        {
          {
            state->camera.position = state->camera.position + state->camera.speed * state->camera.right;
          }
        }
        if (event->key == OS_KEY_ARROW_LEFT)
        {
          {
            state->camera.position = state->camera.position - state->camera.speed * state->camera.right;
          }
        }
      }
      
      default: break;
    }
  }
}
