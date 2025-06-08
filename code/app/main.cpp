#include "base/base_core.h"
#include "base/base_include.h"
#include "base/base_math.h"
#include "os/os_gfx.h"
#include "os/os_include.h"
#include "render/r_core.h"
#include "render/r_include.h"
#include "assets/mesh.h"

#include "base/base_include.cpp"
#include "os/os_include.cpp"
#include "render/r_include.cpp"
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
  app_state.window = OS_CreateWindow("Vulkan Triangle", MakeVec2u(1270, 720));
  OS_ShowWindow(&app_state.window);
  OS_LockCursor(&app_state.window);

  R_Init(R_RENDERER_TYPE_VULKAN, &app_state.window);

  R_Pipeline pipeline = {};
  {
    R_VertexAttributeFormat attributes[] = {
      R_VERTEX_ATTRIBUTE_FORMAT_VEC3F,
      R_VERTEX_ATTRIBUTE_FORMAT_VEC3F,
      R_VERTEX_ATTRIBUTE_FORMAT_VEC2F
    };
    R_PipelineAssignAttributes(&pipeline, attributes, CountArrayElements(attributes));
  
    R_H_LoadShader(app_state.arena, "data/shaders/main.vs.glsl",
                   "main", R_SHADER_TYPE_VERTEX,
                   &pipeline.shaders[R_SHADER_TYPE_VERTEX]);
    R_H_LoadShader(app_state.arena, "data/shaders/main.fs.glsl",
                   "main", R_SHADER_TYPE_FRAGMENT,
                   &pipeline.shaders[R_SHADER_TYPE_FRAGMENT]);
  
    Renderer.CreatePipeline(&pipeline);
  }
  
  R_Pipeline red_pipeline = {};
  {
    R_VertexAttributeFormat attributes[] = {
      R_VERTEX_ATTRIBUTE_FORMAT_VEC3F,
      R_VERTEX_ATTRIBUTE_FORMAT_VEC3F
    };
    R_PipelineAssignAttributes(&red_pipeline, attributes, CountArrayElements(attributes));
  
    R_H_LoadShader(app_state.arena, "data/shaders/red.vs.glsl",
                   "main", R_SHADER_TYPE_VERTEX,
                   &red_pipeline.shaders[R_SHADER_TYPE_VERTEX]);
    R_H_LoadShader(app_state.arena, "data/shaders/red.fs.glsl",
                   "main", R_SHADER_TYPE_FRAGMENT,
                   &red_pipeline.shaders[R_SHADER_TYPE_FRAGMENT]);
  
    Renderer.CreatePipeline(&red_pipeline);
  }
  
  AST_StaticMesh static_mesh = AST_LoadStaticMeshFromGLTF(app_state.arena, Str8FromC("data/motocycle_gltf/motocycle.gltf"));
  AST_StaticMesh monkey_mesh = AST_LoadStaticMeshFromGLTF(app_state.arena, Str8FromC("data/monkey_gltf/monkey.gltf"));
  AST_StaticMesh sphere_mesh = AST_LoadStaticMeshFromGLTF(app_state.arena, Str8FromC("data/sphere_gltf/sphere.gltf"));
  AST_StaticMesh plane_mesh = AST_LoadStaticMeshFromGLTF(app_state.arena, Str8FromC("data/plane_gltf/plane.gltf"));
 
  F32 begin_time = OS_CurrentTimeSeconds();
  while (!app_state.is_window_closed)
  {
    HandleEvents(app_state.frame_arena, &app_state);

    Renderer.BeginFrame();
    {
      Renderer.BeginRenderPass(R_ATTACHMENT_LOAD_OPERATION_CLEAR, MakeVec4f(0.3f, 0.3f, 0.3f, 1.0f));
      {
        Renderer.BindPipeline(&pipeline);
        for (ListNodeAST_Geometry* geometry_node = sphere_mesh.geometry_list.first;
             geometry_node;
             geometry_node = geometry_node->next)
        {
          Renderer.PushGeometry(&geometry_node->data);
          
          struct UData
          {
            alignas(16) Mat4x4f projection;
            alignas(16) Mat4x4f view;
            alignas(16) Mat4x4f model;
            alignas(4)  F32 dt;
          };

          local_persist F32 angle = 0.0f;
          angle += 1.0f * app_state.delta_time;

          Mat4x4f transpose = Transpose4x4f(MakeVec3f(0.0f, 0.0f, 0.0f));
          Mat4x4f rotate = Rotate4x4f(MakeVec3f(0.0f, 1.0f, 0.0f), app_state.camera.yaw);
          // Mat4x4f rotate = Rotate4x4f(MakeVec3f(0.0f, 1.0f, 0.0f), angle);
          Mat4x4f model = rotate * transpose;
          
          F32 aspect_ration = (F32)app_state.window.size.x / (F32)app_state.window.size.y;
          UData u_data = {
            .projection = MakePerspective4x4f(45.0f, aspect_ration, 0.1f, 1000.0f),
            .view = MakeLookAt(app_state.camera.position, app_state.camera.position + app_state.camera.front, app_state.camera.up),
            .model = model,
            .dt = OS_CurrentTimeSeconds()
          };
          
          R_DrawGeometryInfo draw_info = {
            .pipeline = &pipeline,
            .uniform_data = (U8*)PushArena(app_state.frame_arena, sizeof(u_data)),
            .uniform_data_size = sizeof(u_data),
            .geometry = &geometry_node->data
          };
          memcpy(draw_info.uniform_data, &u_data, sizeof(u_data));
          Renderer.DrawGeometry(&draw_info);
        }
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
        state->window.size = event->window_size;
        // LOG_INFO("New window size: %d w %d h\n\n", state->window.size.x, state->window.size.y);
        Renderer.HandleResize(&state->window);
      } break;

      case OS_EVENT_TYPE_MOUSE_MOVE:
      {
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
