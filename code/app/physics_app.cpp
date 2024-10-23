#include "base/base_include.h"
#include "os/os_include.h"
#include "render/r_include.h"

#include "base/base_include.cpp"
#include "os/os_include.cpp"
#include "render/r_include.cpp"

global R_Pipeline box_pipeline = {};
global R_VertexBuffer box_vertex_buffer = {};
global R_IndexBuffer box_index_buffer = {};
func R_Pipeline CreateBox2DPipeline(Arena* arena);

func void DrawBox(Vec2f position, Vec2f size, Vec3f color);

I32 main()
{
  OS_Window window = OS_CreateWindow("PhysicsApp", MakeVec2u(1280, 720));
  R_Init(&window);

  OS_ShowWindow(&window);

  Arena* arena = AllocateArena(Megabytes(64));

  box_pipeline = CreateBox2DPipeline(arena);

  // --AlNov: Vertex and Index Buffers for sdf box
  Vec2f vertecies[] = {
    MakeVec2f(-1.0f, -1.0f),
    MakeVec2f(+1.0f, -1.0f),
    MakeVec2f(+1.0f, +1.0f),
    MakeVec2f(-1.0f, +1.0f),
  };
  box_vertex_buffer = R_CreateVertexBuffer(
    vertecies,
    sizeof(vertecies[0]),
    CountArrayElements(vertecies)
  );
  U32 indecies[] = {0, 1, 2, 2, 3, 0};
  box_index_buffer = R_CreateIndexBuffer(
    indecies,
    sizeof(indecies[0]),
    CountArrayElements(indecies)
  );

  B32 is_window_closed = false;
  while(!is_window_closed)
  {
    OS_EventList event_list = OS_GetEventList(arena);
    
    for (OS_Event* event = event_list.first;
         event;
         event = event->next)
    {
      switch(event->type)
      {
        case OS_EVENT_TYPE_EXIT:
        {
          is_window_closed = true;
        } break;

        default: break;
      }
    }

    R_FrameInfo frame_info = {};
    Renderer.BeginFrame();
    {
      Vec4f clear_color = MakeVec4f(0.2f, 0.2f, 0.2f, 1.0f);
      F32 depth_clear = 1.0f;
      F32 stencil_clear = 0.0f;
      Renderer.BeginRenderPass(clear_color, depth_clear, stencil_clear);
      {
        for (I32 i = 0; i < 2; i += 1)
        {
          DrawBox(
            MakeVec2f(0.0f + 300.0f*i, 0.0 + 300.0f*i),
            MakeVec2f(25.0f, 25.0f),
            MakeVec3f(1.0f, 0.5f, 0.0f)
          );
        }
      }
      Renderer.EndRenderPass();
    }
    Renderer.EndFrame();
  }
}

func R_Pipeline
CreateBox2DPipeline(Arena* arena)
{
  R_Pipeline result = {};

  R_VertexAttributeFormat vertex_attributes[] = {
    R_VERTEX_ATTRIBUTE_FORMAT_VEC2F
  };
  R_PipelineAssignAttributes(
    &result,
    vertex_attributes,
    CountArrayElements(vertex_attributes));

  R_BindingInfo bindings[] = 
  {
    {R_BINDING_TYPE_UNIFORM_BUFFER, R_SHADER_TYPE_VERTEX}
  };
  R_PipelineAssignBindingLayout(&result, bindings, CountArrayElements(bindings));

  R_H_LoadShader(
    arena, "data/shaders/sdf_vs.glsl",
    "main", R_SHADER_TYPE_VERTEX,
    &result.shaders[R_SHADER_TYPE_VERTEX]
  );
  R_H_LoadShader(
    arena, "data/shaders/sdf_box_fs.glsl",
    "main", R_SHADER_TYPE_FRAGMENT,
    &result.shaders[R_SHADER_TYPE_FRAGMENT]
  );

  result.is_depth_test_enabled = false;

  Renderer.CreatePipeline(&result);

  return result;
}

func void
DrawBox(Vec2f position, Vec2f size, Vec3f color)
{
  struct 
  {
    alignas(16) Mat4x4f projection;
    alignas(8)  Vec2f position;
    alignas(8)  Vec2f size;
    alignas(16) Vec3f color;
  } uniform_data;
  uniform_data.projection = MakeOrthographic4x4f(
    0.0f, 1280.0f, 0.0f, 720.0f, 0.0f, 1.0f
  );
  uniform_data.position = position;
  uniform_data.size = size;
  uniform_data.color = color;

  struct
  {
    alignas(16) Mat4x4f projection;
  } scene_data;
  scene_data.projection = MakeOrthographic4x4f(
    0.0f, 1280.0f, 0.0f, 720.0f, 0.0f, 1.0f
  );

  struct
  {
    alignas(8)  Vec2f translate;
    alignas(8)  Vec2f size;
    alignas(16) Vec3f color;
  } draw_vs_data;
  draw_vs_data.translate = position;
  draw_vs_data.size = size;
  draw_vs_data.color = color;

  struct
  {
    alignas(16) Vec3f color;
  } draw_fs_data;
  draw_fs_data.color = color;

  R_DrawInfo draw_info = {};
  draw_info.pipeline = &box_pipeline;
  draw_info.vertex_buffer = &box_vertex_buffer;
  draw_info.index_buffer = &box_index_buffer;
  draw_info.uniform_data = &uniform_data;
  draw_info.uniform_data_size = sizeof(uniform_data);
  draw_info.scene_data = &scene_data;
  draw_info.scene_data_size = sizeof(scene_data);
  draw_info.draw_vs_data = &draw_vs_data;
  draw_info.draw_vs_data_size = sizeof(draw_vs_data);
  draw_info.draw_fs_data = &draw_fs_data;
  draw_info.draw_fs_data_size = sizeof(draw_fs_data);

  Renderer.Draw(&draw_info);
}
