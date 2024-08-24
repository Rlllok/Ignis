#pragma comment(lib, "Winmm.lib")

// --AlNov: .h -------------------------------------------------------
#include "../base/base_include.h"
#include "../os/os_include.h"
#include "../render/r_include.h"

// --AlNov: .cpp -----------------------------------------------------
#include "../base/base_include.cpp"
#include "../os/os_include.cpp"
#include "../render/r_include.cpp"

func R_SceneObject* GenerateQuad(Arena* arena);

i32 main()
{
  OS_Window window = OS_CreateWindow("FullQuadShader", MakeVec2u(1280, 720));
  R_Init(&window);

  f32 delta_time = 1.0f / 60.0f;

  Arena* arena = AllocateArena(Megabytes(256));

  R_Pipeline fullquad_pipeline = {};
  {
    // --AlNov: @TODO
    // R_VertexAttributeFormat attributes[] = { R_VERTEX_ATTRIBUTE_FORMAT_VEC3F };
    R_VertexAttributeFormat attributes[] = {
      R_VERTEX_ATTRIBUTE_FORMAT_VEC3F,
      R_VERTEX_ATTRIBUTE_FORMAT_VEC3F,
      R_VERTEX_ATTRIBUTE_FORMAT_VEC2F
    };

    R_PipelineAssignAttributes(&fullquad_pipeline, attributes, CountArrayElements(attributes));

    R_BindingInfo bindings[] =
    {
      {R_BINDING_TYPE_UNIFORM_BUFFER, R_SHADER_TYPE_VERTEX},
    };
    R_PipelineAssignBindingLayout(&fullquad_pipeline, bindings, CountArrayElements(bindings));

    R_H_LoadShader(arena, "data/shaders/fullquad.vert", "main", R_SHADER_TYPE_VERTEX, &fullquad_pipeline.shaders[R_SHADER_TYPE_VERTEX]);
    R_H_LoadShader(arena, "data/shaders/fullquad.frag", "main", R_SHADER_TYPE_FRAGMENT, &fullquad_pipeline.shaders[R_SHADER_TYPE_FRAGMENT]);
    R_CreatePipeline(&fullquad_pipeline);
  }

  OS_ShowWindow(&window);
  LOG_INFO("Window showed.\n");

  R_SceneObject* quad = GenerateQuad(arena);

  bool is_window_closed = false;
  while(!is_window_closed)
  {
    f32 begin_time = OS_CurrentTimeSeconds();

    OS_EventList event_list = OS_GetEventList(arena);

    OS_Event* event = event_list.first;
    while (event)
    {
      switch(event->type)
      {
        case OS_EVENT_TYPE_EXIT:
        {
          is_window_closed = true;
        } break;

        default: break;
      }

      event = event->next;
    }

    R_FrameInfo frame_info = {};
    frame_info.delta_time = delta_time;

    R_BeginFrame();
    {
      R_BeginRenderPass(MakeVec4f(1.0f, 1.0f, 1.0f, 1.0f), 1.0f, 0.0f);
      {
        struct UBO
        {
          alignas(4)  f32     time;
          alignas(8)  Vec2f   resolution;
          alignas(16) Mat4x4f ortho;
          alignas(8)  Vec2f   mouse_position;
        };

        UBO ubo = {};
        ubo.time            = begin_time;
        ubo.resolution      = MakeVec2f(window.width, window.height);
        ubo.ortho           = MakeOrthographic4x4f(0.0f, window.width, window.height, 0.0f, -1.0f, 2.0f);
        ubo.mouse_position  = OS_MousePosition(window);

        R_DrawInfo draw_info = {};
        draw_info.pipeline          = &fullquad_pipeline;
        draw_info.vertecies         = quad->vertecies;
        draw_info.vertex_size       = sizeof(quad->vertecies[0]);
        draw_info.vertex_count      = quad->vertex_count;
        draw_info.indecies          = quad->indecies;
        draw_info.index_size        = sizeof(u32);
        draw_info.index_count       = quad->index_count;
        draw_info.uniform_data      = &ubo;
        draw_info.uniform_data_size = sizeof(ubo);

        R_DrawSceneObject(&draw_info);
      }
      R_EndRenderPass();
    }
    R_EndFrame();

    ResetArena(arena);

    f32 end_time = OS_CurrentTimeSeconds();
    f32 wait_time = delta_time - (end_time - begin_time);
    if (wait_time > 0) { OS_Wait(wait_time); };
  }

  return 0;
}

func R_SceneObject* GenerateQuad(Arena* arena)
{
  R_SceneObject* quad = (R_SceneObject*)PushArena(arena, sizeof(R_SceneObject));
  quad->vertex_count          = 4;
  quad->vertecies             = (R_SceneObject::Vertex*)PushArena(arena, sizeof(R_SceneObject::Vertex) * quad->vertex_count);
  quad->vertecies[0].position = MakeVec3f(-1.0f, -1.0f, 0.0f);
  quad->vertecies[1].position = MakeVec3f(1.0f, -1.0f, 0.0f);
  quad->vertecies[2].position = MakeVec3f(1.0f, 1.0f, 0.0f);
  quad->vertecies[3].position = MakeVec3f(-1.0f, 1.0f, 0.0f);
  quad->index_count           = 6;
  quad->indecies              = (u32*)PushArena(arena, sizeof(u32) * quad->index_count);
  quad->indecies[0]           = 0;
  quad->indecies[1]           = 1;
  quad->indecies[2]           = 2;
  quad->indecies[3]           = 2;
  quad->indecies[4]           = 3;
  quad->indecies[5]           = 0;

  return quad;
}