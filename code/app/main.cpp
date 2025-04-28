#include "base/base_core.h"
#include "base/base_include.h"
#include "base/base_math.h"
#include "os/gfx/os_gfx.h"
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
};

struct AppState
{
  Arena* arena;
  Arena* frame_arena;
  OS_Window window;

  Camera camera;
  F32 delta_time;

  B32 is_window_closed;
} app_state;

func void HandleEvents(AppState* state);

I32 main()
{
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
 
  app_state.window = OS_CreateWindow("Vulkan Triangle", MakeVec2u(1270, 720));
  OS_ShowWindow(&app_state.window);

  R_Init(R_RENDERER_TYPE_VULKAN, &app_state.window);

  R_Pipeline pipeline = {};
  {
    R_VertexAttributeFormat attributes[] = {
      R_VERTEX_ATTRIBUTE_FORMAT_VEC3F,
      R_VERTEX_ATTRIBUTE_FORMAT_VEC3F
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
 
  F32 begin_time = OS_CurrentTimeSeconds();
  while (!app_state.is_window_closed)
  {
    HandleEvents(&app_state);

    Renderer.BeginFrame();
    {
      Renderer.BeginRenderPass(R_ATTACHMENT_LOAD_OPERATION_CLEAR, MakeVec4f(0.3f, 0.3f, 0.3f, 1.0f));
      {
        Renderer.BindPipeline(&pipeline);
        for (ListNodeAST_Geometry* geometry_node = static_mesh.geometry_list.first;
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
          angle += 0.01f;

          Mat4x4f transpose = Transpose4x4f(MakeVec3f(0.0f, 0.0f, 0.0f));
          Mat4x4f rotate = Rotate4x4f(MakeVec3f(0.0f, 1.0f, 0.0f), angle);
          Mat4x4f model = transpose;
          
          UData u_data = {
            .projection = MakePerspective4x4f(45.0f, 1280.0f/720.0f, 0.1f, 1000.0f),
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
      #if 0
      Renderer.EndRenderPass();
      Renderer.BeginRenderPass(R_ATTACHMENT_LOAD_OPERATION_LOAD, {});
      {
        Renderer.BindPipeline(&red_pipeline);
        for (ListNodeAST_Geometry* geometry_node = monkey_mesh.geometry_list.first;
             geometry_node;
             geometry_node = geometry_node->next)
        {
          Renderer.PushGeometry(&geometry_node->data);
          
          struct UData
          {
            alignas(16) Mat4x4f projection;
            alignas(16) Mat4x4f view;
            alignas(16) Mat4x4f model;
          };

          local_persist F32 angle = 0.0f;
          angle += 0.01f;

          Mat4x4f transpose = Transpose4x4f(MakeVec3f(0.0f, 0.0f, sinf(angle) * 5.0f));
          Mat4x4f model = transpose;
          
          UData u_data = {
            .projection = MakePerspective4x4f(45.0f, 1280.0f/720.0f, 0.1f, 10.0f),
            .view = MakeLookAt(app_state.camera.position, app_state.camera.position + app_state.camera.front, app_state.camera.up),
            .model = model
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
      // @NOTE Vulkan inserts VkCmdEndRendering() at the end by itself.
      //Renderer.EndRenderPass();
      #endif
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
HandleEvents(AppState* state)
{
  OS_EventList event_list = OS_GetEventList(app_state.arena);
  
  for (OS_Event *event = event_list.first; event; event = event->next)
  {
    switch (event->type)
    {
      case OS_EVENT_TYPE_EXIT:
      {
        state->is_window_closed = true;
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
