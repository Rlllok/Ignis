#pragma comment(lib, "Winmm.lib")

// --AlNov: .h -------------------------------------------------------
#include "../base/base_include.h"
#include "../os/os_include.h"
#include "../render/r_include.h"

// --AlNov: .cpp -----------------------------------------------------
#include "../base/base_include.cpp"
#include "../os/os_include.cpp"
#include "../render/r_include.cpp"


func R_SceneObject* GenerateUVSphere(Arena* arena, Vec3f center_position, f32 radius, u32 phi_count, f32 theta_count)
{
  const f32 phi_step     = 2 * PI / phi_count;
  const f32 theta_step   = PI / theta_count;
  const u32 vertex_count = 2 + phi_count * (theta_count - 1);
  const u32 index_count  = 2 * 3 * phi_count + 2 * 3 * phi_count * (theta_count - 2);

  R_SceneObject* object   = (R_SceneObject*)PushArena(arena, sizeof(R_SceneObject));
  object->vertex_count    = vertex_count;
  object->vertecies       = (R_SceneObject::Vertex*)PushArena(arena, sizeof(R_SceneObject::Vertex) * vertex_count);
  object->index_count     = index_count;
  object->indecies        = (u32*)PushArena(arena, sizeof(u32) * index_count);

  // --AlNov: Generate UVSphere vertecies
  {
    u32 current_index = 0;
    object->vertecies[current_index].position = MakeVec3f(0.0f, radius, 0.0f);
    object->vertecies[current_index].normal   = NormalizeVec3f(object->vertecies[current_index].position);
    object->vertecies[current_index].uv       = MakeVec2f(0.5f + atan2f(object->vertecies[current_index].position.z, object->vertecies[current_index].position.x) / (2*PI), 0.5f + asinf(object->vertecies[current_index].position.y) / PI);
    current_index += 1;

    for (u32 i = 1; i < theta_count - 0; i += 1)
    {
      const f32 theta = i * theta_step;
      for (u32 j = 0; j < phi_count; j += 1)
      {
        const f32 phi = j * phi_step;

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
    u32 current_index = 0;
    for (u32 i = 0; i < phi_count - 1; i += 1)
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

    for (u32 i = 0; i < (theta_count - 2); i += 1)
    {
      for (u32 j = 0; j < (phi_count - 1); j += 1)
      {
        const u32 QuadIndecies[4] = {
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

      const u32 QuadIndecies[4] = {
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

    const u32 south_index = vertex_count - 1;
    for (u32 i = 0; i < phi_count - 1; i += 1)
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

i32 main()
{
  OS_Window window = OS_CreateWindow("Sphere", MakeVec2u(1280, 720));
  R_Init(&window);

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
    R_CreatePipeline(&default_pipeline);
  }

  R_Pipeline pink_pipeline = {};
  {
    R_VertexAttributeFormat attributes[] = {
      R_VERTEX_ATTRIBUTE_FORMAT_VEC3F,
      R_VERTEX_ATTRIBUTE_FORMAT_VEC3F,
      R_VERTEX_ATTRIBUTE_FORMAT_VEC2F
    };
    R_PipelineAssignAttributes(&pink_pipeline, attributes, CountArrayElements(attributes));

    R_BindingInfo bindings[] =
    {
      {R_BINDING_TYPE_UNIFORM_BUFFER, R_SHADER_TYPE_VERTEX}
    };
    R_PipelineAssignBindingLayout(&pink_pipeline, bindings, CountArrayElements(bindings));

    R_H_LoadShader(arena, "data/shaders/pink3D.vert", "main", R_SHADER_TYPE_VERTEX, &pink_pipeline.shaders[R_SHADER_TYPE_VERTEX]);
    R_H_LoadShader(arena, "data/shaders/pink3D.frag", "main", R_SHADER_TYPE_FRAGMENT, &pink_pipeline.shaders[R_SHADER_TYPE_FRAGMENT]);
    
    R_CreatePipeline(&pink_pipeline);
  }

  OS_ShowWindow(&window);
  LOG_INFO("Window showed.\n");

  LARGE_INTEGER win32_freq;
  QueryPerformanceFrequency(&win32_freq);
  u64 frequency = win32_freq.QuadPart;

  LARGE_INTEGER win32_cycles;
  QueryPerformanceCounter(&win32_cycles);
  u64 start_cycles = win32_cycles.QuadPart;

  f32 time_sec = 0.0f;

  Vec3f sphere_position = MakeVec3f(0.0f, 0.0f, 5.0f);
  f32   sphere_speed    = 5.0f;

  bool is_window_closed = false;
  while(!is_window_closed)
  {
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

        case OS_EVENT_TYPE_KEYBOARD:
        {
          switch (event->key)
          {
            case OS_KEY_ARROW_UP:
            {
              if (event->is_down)
              {
                sphere_position.z += sphere_speed * time_sec;
              }
            } break; 
            case OS_KEY_ARROW_DOWN:
            {
              if (event->is_down)
              {
                sphere_position.z -= sphere_speed * time_sec;
              }
            } break;

            default: break;
          }
        }

        default: break;
      }

      event = event->next;
    }

    R_SceneObject* uv_sphere = GenerateUVSphere(arena, sphere_position, 1.0f, 30, 30);

    R_FrameInfo frame_info = {};
    frame_info.delta_time = time_sec;

    R_BeginFrame();
    {
      R_BeginRenderPass(MakeVec4f(1.0f, 1.0f, 1.0f, 1.0f), 1.0f, 0.0f);
      {
        struct MVP
        {
          alignas(16) Vec3f   color       = MakeVec3f(1.0f, 1.0f, 0.0f);
          alignas(16) Mat4x4f view        = MakePerspective4x4f(45.0f, 1.0f, 0.1f, 1000.0f);
          alignas(16) Mat4x4f translation = Transpose4x4f(MakeVec3f(0.0f, 0.0f, 8.0f));
        };

        {
          MVP mvp;
          mvp.color       = MakeVec3f(0.87f, 0.57f, 0.81f);
          mvp.translation = Transpose4x4f(MakeVec3f(1.5f, 1.0f, 8.0f));

          R_DrawInfo draw_info = {};
          draw_info.pipeline          = &pink_pipeline;
          draw_info.vertecies         = uv_sphere->vertecies;
          draw_info.vertex_size       = sizeof(R_SceneObject::Vertex);
          draw_info.vertex_count      = uv_sphere->vertex_count;
          draw_info.indecies          = uv_sphere->indecies;
          draw_info.index_size        = sizeof(u32);
          draw_info.index_count       = uv_sphere->index_count;
          draw_info.uniform_data      = &mvp;
          draw_info.uniform_data_size = sizeof(mvp);

          R_DrawSceneObject(&draw_info);
        }

        {
          MVP mvp;

          mvp.color       = MakeVec3f(1.0f, 1.0f, 0.0f);
          mvp.translation = Transpose4x4f(MakeVec3f(-1.0f, 0.0f, 8.0f));

          R_DrawInfo draw_info = {};
          draw_info.pipeline          = &default_pipeline;
          draw_info.vertecies         = uv_sphere->vertecies;
          draw_info.vertex_size       = sizeof(R_SceneObject::Vertex);
          draw_info.vertex_count      = uv_sphere->vertex_count;
          draw_info.indecies          = uv_sphere->indecies;
          draw_info.index_size        = sizeof(u32);
          draw_info.index_count       = uv_sphere->index_count;
          draw_info.uniform_data      = &mvp;
          draw_info.uniform_data_size = sizeof(mvp);

          R_DrawSceneObject(&draw_info);
        }
      }
      R_EndRenderPass();
    }
    R_EndFrame();

    ResetArena(arena);

    QueryPerformanceCounter(&win32_cycles);
    u64 end_cycles    = win32_cycles.QuadPart;
    u64 cycles_delta  = end_cycles - start_cycles;
    start_cycles      = end_cycles;
    time_sec          = (f32)cycles_delta / (f32)frequency;
  }

  return 0;
}
