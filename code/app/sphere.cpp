#pragma comment(lib, "Winmm.lib")

// --AlNov: .h -------------------------------------------------------
#include "base/base_include.h"
#include "os/os_include.h"
#include "render/r_include.h"

// --AlNov: .cpp -----------------------------------------------------
#include "../base/base_include.cpp"
#include "../os/os_include.cpp"
#include "../render/r_include.cpp"

func R_SceneObject* 
GenerateUVSphere(Arena* arena, Vec3f center_position, F32 radius, U32 phi_count, F32 theta_count)
{
  const F32 phi_step     = 2 * PI / phi_count;
  const F32 theta_step   = PI / theta_count;
  const U32 vertex_count = 2 + phi_count * (theta_count - 1);
  const U32 index_count  = 2 * 3 * phi_count + 2 * 3 * phi_count * (theta_count - 2);

  R_SceneObject* object   = (R_SceneObject*)PushArena(arena, sizeof(R_SceneObject));
  object->vertex_count    = vertex_count;
  object->vertecies       = (R_SceneObject::Vertex*)PushArena(arena, sizeof(R_SceneObject::Vertex) * vertex_count);
  object->index_count     = index_count;
  object->indecies        = (U32*)PushArena(arena, sizeof(U32) * index_count);

  // --AlNov: Generate UVSphere vertecies
  {
    U32 current_index = 0;
    object->vertecies[current_index].position = MakeVec3f(0.0f, radius, 0.0f);
    object->vertecies[current_index].normal   = NormalizeVec3f(object->vertecies[current_index].position);
    object->vertecies[current_index].uv       = MakeVec2f(0.5f + atan2f(object->vertecies[current_index].position.z, object->vertecies[current_index].position.x) / (2*PI), 0.5f + asinf(object->vertecies[current_index].position.y) / PI);
    current_index += 1;

    for (U32 i = 1; i < theta_count - 0; i += 1)
    {
      const F32 theta = i * theta_step;
      for (U32 j = 0; j < phi_count; j += 1)
      {
        const F32 phi = j * phi_step;

        Vec3f position = {};
        position.x     = sinf(theta) * cosf(phi);
        position.y     = cosf(theta);
        position.z     = sinf(theta) * sinf(phi);
        position       = MulVec3f(position, radius);

        object->vertecies[current_index].position = position;
        object->vertecies[current_index].normal   = NormalizeVec3f(position);
        object->vertecies[current_index].uv       = MakeVec2f(0.5f + atan2f(position.z, position.x) / (2*PI), 0.5f + asinf(position.y) / PI);
        current_index += 1;
      }
    }

    object->vertecies[current_index].position = MakeVec3f(0.0f, -radius, 0.0f);
    object->vertecies[current_index].normal   = NormalizeVec3f(object->vertecies[current_index].position);
    object->vertecies[current_index].uv       = MakeVec2f(0.5f + atan2f(object->vertecies[current_index].position.z, object->vertecies[current_index].position.x) / (2*PI), 0.5f + asinf(object->vertecies[current_index].position.y) / PI);
    current_index += 1;
  }

  // --AlNov: Generate UVSphere indecies
  {
    U32 current_index = 0;
    for (U32 i = 0; i < phi_count - 1; i += 1)
    {
      object->indecies[current_index] = 0;
      current_index += 1;
      object->indecies[current_index] = i + 1;
      current_index += 1;
      object->indecies[current_index] = i + 2;
      current_index += 1;
    }

    object->indecies[current_index] = 0;
    current_index += 1;
    object->indecies[current_index] = phi_count;
    current_index += 1;
    object->indecies[current_index] = 1;
    current_index += 1;

    for (U32 i = 0; i < (theta_count - 2); i += 1)
    {
      for (U32 j = 0; j < (phi_count - 1); j += 1)
      {
        const U32 QuadIndecies[4] = {
          1 + j + i * phi_count,
          1 + j + (i + 1) * phi_count,
          1 + (j + 1) + (i + 1) * phi_count,
          1 + (j + 1) + i * phi_count,
        };

        object->indecies[current_index] = QuadIndecies[0];
        current_index += 1;
        object->indecies[current_index] = QuadIndecies[1];
        current_index += 1;
        object->indecies[current_index] = QuadIndecies[2];
        current_index += 1;
        object->indecies[current_index] = QuadIndecies[0];
        current_index += 1;
        object->indecies[current_index] = QuadIndecies[2];
        current_index += 1;
        object->indecies[current_index] = QuadIndecies[3];
        current_index += 1;
      }

      const U32 QuadIndecies[4] = {
        phi_count + i * phi_count,
        phi_count + (i + 1) * phi_count,
        1 + (i + 1) * phi_count,
        1 + i * phi_count,
      };

      object->indecies[current_index] = QuadIndecies[0];
      current_index += 1;
      object->indecies[current_index] = QuadIndecies[1];
      current_index += 1;
      object->indecies[current_index] = QuadIndecies[2];
      current_index += 1;
      object->indecies[current_index] = QuadIndecies[0];
      current_index += 1;
      object->indecies[current_index] = QuadIndecies[2];
      current_index += 1;
      object->indecies[current_index] = QuadIndecies[3];
      current_index += 1;
    }

    const U32 south_index = vertex_count - 1;
    for (U32 i = 0; i < phi_count - 1; i += 1)
    {
      object->indecies[current_index] = south_index;
      current_index += 1;
      object->indecies[current_index] = south_index - phi_count + i + 1;
      current_index += 1;
      object->indecies[current_index] = south_index - phi_count + i;
      current_index += 1;
    }

    object->indecies[current_index] = south_index;
    current_index += 1;
    object->indecies[current_index] = south_index - phi_count;
    current_index += 1;
    object->indecies[current_index] = south_index - 1;
    current_index += 1;
  }

  return object;
}

I32 main()
{
  OS_Window window = OS_CreateWindow("Sphere", MakeVec2u(1280, 720));
  R_Init(&window);

  Renderer.CreateBuffer(
      64, 
      BUFFER_USAGE_FLAG_VERTEX,
      BUFFER_PROPERTY_HOST_COHERENT);

  F32 delta_time = 1.0f / 60.0f;

  Arena* arena = AllocateArena(Megabytes(256));

  R_Pipeline default_pipeline = {};
  {
    R_VertexAttributeFormat attributes[] = {
      R_VERTEX_ATTRIBUTE_FORMAT_VEC3F,
      R_VERTEX_ATTRIBUTE_FORMAT_VEC3F,
      R_VERTEX_ATTRIBUTE_FORMAT_VEC2F
    };
    R_PipelineAssignAttributes(&default_pipeline, attributes, CountArrayElements(attributes));

    R_BindingInfo bindings[] =
    {
      {R_BINDING_TYPE_UNIFORM_BUFFER, R_SHADER_TYPE_VERTEX},
      {R_BINDING_TYPE_TEXTURE_2D, R_SHADER_TYPE_FRAGMENT}
    };
    R_PipelineAssignBindingLayout(&default_pipeline, bindings, CountArrayElements(bindings));

    R_H_LoadShader(arena, "data/shaders/default3D.vert", "main", R_SHADER_TYPE_VERTEX, &default_pipeline.shaders[R_SHADER_TYPE_VERTEX]);
    R_H_LoadShader(arena, "data/shaders/default3D.frag", "main", R_SHADER_TYPE_FRAGMENT, &default_pipeline.shaders[R_SHADER_TYPE_FRAGMENT]);

    default_pipeline.is_depth_test_enabled = true;
    
    Renderer.CreatePipeline(&default_pipeline);
  }

  OS_ShowWindow(&window);
  LOG_INFO("Window showed.\n");

  Vec3f sphere_position = MakeVec3f(-1.0f, 0.0f, 5.0f);

  R_SceneObject* uv_sphere = GenerateUVSphere(arena, sphere_position, 1.0f, 30, 30);
  // --AlNov: @TODO Vertex buffer is empty because vertex data is not copied
  R_VertexBuffer sphere_vertex_buffer = R_CreateVertexBuffer(
      uv_sphere->vertecies,
      sizeof(R_SceneObject::Vertex),
      uv_sphere->vertex_count);
  R_IndexBuffer sphere_index_buffer = R_CreateIndexBuffer(
      uv_sphere->indecies,
      sizeof(U32),
      uv_sphere->index_count);

  bool is_window_closed = false;
  while(!is_window_closed)
  {
    F32 begin_time = OS_CurrentTimeSeconds();

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

    // --AlNov: Draw Sphere
    Renderer.BeginFrame();
    {
      Renderer.BeginRenderPass(MakeVec4f(1.0f, 1.0f, 1.0f, 1.0f), 1.0f, 0.0f);
      {
        struct MVP
        {
          alignas(16) Vec3f   color       = MakeVec3f(1.0f, 1.0f, 0.0f);
          alignas(16) Mat4x4f view        = MakePerspective4x4f(45.0f, 1.0f, 0.1f, 1000.0f);
          alignas(16) Mat4x4f translation = Transpose4x4f(MakeVec3f(0.0f, 0.0f, 8.0f));
        };

        {
          MVP mvp;

          R_DrawInfo draw_info = {};
          draw_info.pipeline          = &default_pipeline;
          draw_info.vertex_buffer = &sphere_vertex_buffer;
          draw_info.index_buffer = &sphere_index_buffer;
          draw_info.uniform_data      = &mvp;
          draw_info.uniform_data_size = sizeof(mvp);

          Renderer.Draw(&draw_info);
        }
      }
      Renderer.EndRenderPass();
    }
    Renderer.EndFrame();

    ResetArena(arena);

    F32 end_time = OS_CurrentTimeSeconds();
    F32 wait_time = delta_time - (end_time - begin_time);
    if (wait_time > 0) { OS_Wait(wait_time); };
    // LOG_INFO("Wait seconds: %f\n", wait_time);
  }

  return 0;
}
