#include "base/base_core.h"
#include "base/base_include.h"
#include "os/os_include.h"
#include "render/r_include.h"
#include "assets/mesh.h"

#include "base/base_include.cpp"
#include "os/os_include.cpp"
#include "render/r_include.cpp"
#include "assets/mesh.cpp"
#include "render/r_pipeline.h"

struct AppState
{
  Arena* arena;
  OS_Window window;

  B32 is_window_closed;
} app_state;

func void HandleEvents(AppState* state);

I32 main()
{
  app_state = {};
  app_state.arena = AllocateArena(Megabytes(64));
  app_state.is_window_closed = false;
 
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
  
    R_H_LoadShader(app_state.arena, "data/shaders/triangle.vs.glsl",
                   "main", R_SHADER_TYPE_VERTEX,
                   &pipeline.shaders[R_SHADER_TYPE_VERTEX]);
    R_H_LoadShader(app_state.arena, "data/shaders/triangle.fs.glsl",
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
  for (ListNodeAST_Geometry* geometry_node = static_mesh.geometry_list.first;
       geometry_node;
       geometry_node = geometry_node->next)
  {
    Renderer.PushGeometry(&geometry_node->data);
  }

  AST_StaticMesh monkey_mesh = AST_LoadStaticMeshFromGLTF(app_state.arena, Str8FromC("data/monkey_gltf/monkey.gltf"));
  for (ListNodeAST_Geometry* geometry_node = monkey_mesh.geometry_list.first;
       geometry_node;
       geometry_node = geometry_node->next)
  {
    Renderer.PushGeometry(&geometry_node->data);
  }

  AST_StaticMesh sphere_mesh = AST_LoadStaticMeshFromGLTF(app_state.arena, Str8FromC("data/sphere_gltf/sphere.gltf"));
  for (ListNodeAST_Geometry* geometry_node = sphere_mesh.geometry_list.first;
       geometry_node;
       geometry_node = geometry_node->next)
  {
    Renderer.PushGeometry(&geometry_node->data);
  }
  
  while (!app_state.is_window_closed)
  {
    HandleEvents(&app_state);

    Renderer.BeginFrame();
    {
      Renderer.BeginRenderPass(R_ATTACHMENT_LOAD_OPERATION_CLEAR, MakeVec4f(1.0f, 1.0f, 1.0f, 1.0f));
      {
        Renderer.BindPipeline(&pipeline);
        for (ListNodeAST_Geometry* geometry_node = sphere_mesh.geometry_list.first;
             geometry_node;
             geometry_node = geometry_node->next)
        {
          Renderer.DrawGeometry(&geometry_node->data);
        }
      }
      # if 0
      Renderer.EndRenderPass();
      Renderer.BeginRenderPass(R_ATTACHMENT_LOAD_OPERATION_LOAD, {});
      {
        Renderer.BindPipeline(&red_pipeline);
        for (ListNodeAST_Geometry* geometry_node = monkey_mesh.geometry_list.first;
             geometry_node;
             geometry_node = geometry_node->next)
        {
          Renderer.DrawGeometry(&geometry_node->data);
        }
      }
      #endif
      // @NOTE Vulkan inserts VkCmdEndRendering() at the end by itself.
      // Renderer.EndRenderPass();
    }
    Renderer.EndFrame();
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
      
      default: break;
    }
  }
}
