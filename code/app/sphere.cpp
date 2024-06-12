#pragma comment(lib, "Winmm.lib")

// --AlNov: .h -------------------------------------------------------
#include "../base/base_include.h"
#include "../os/os_include.h"
#include "../render/vulkan/r_init_vk.h"

// --AlNov: .cpp -----------------------------------------------------
#include "../base/base_include.cpp"
#include "../os/os_include.cpp"
#include "../render/vulkan/r_init_vk.cpp"

#define PI 3.141592654f

func R_Mesh* GenerateUVSphere(Arena* arena, Vec3f position, f32 radius, u32 phi_count, f32 theta_count)
{
  const f32 phi_step     = 2 * PI / phi_count;
  const f32 theta_step   = PI / theta_count;
  const u32 vertex_count = 2 + phi_count * (theta_count - 1);
  const u32 index_count  = 2 * 3 * phi_count + 2 * 3 * phi_count * (theta_count - 2);

  R_Mesh* mesh              = (R_Mesh*)PushArena(arena, sizeof(R_Mesh));
  mesh->mvp.color           = MakeVec3f(1.0f, 0.0f, 0.0f);
  mesh->mvp.center_position = position;
  mesh->mvp.view            = Make4x4f(1.0f);
  mesh->vertex_count        = vertex_count;
  mesh->vertecies           = (R_MeshVertex*)PushArena(arena, sizeof(R_MeshVertex) * vertex_count);
  mesh->index_count         = index_count;
  mesh->indecies            = (u32*)PushArena(arena, sizeof(u32) * index_count);

  // --AlNov: Generate UVSphere vertecies
  {
    u32 current_index = 0;
    mesh->vertecies[current_index].position = MakeVec3f(0.0f, radius, 0.0f);
    mesh->vertecies[current_index].normals  = NormalizeVec3f(mesh->vertecies[current_index].position);
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

        mesh->vertecies[current_index].position = position;
        mesh->vertecies[current_index].normals  = NormalizeVec3f(position);
        current_index += 1;
      }
    }

    mesh->vertecies[current_index].position = MakeVec3f(0.0f, -radius, 0.0f);
    mesh->vertecies[current_index].normals  = NormalizeVec3f(mesh->vertecies[current_index].position);
    current_index += 1;
  }

  // --AlNov: Generate UVSphere indecies
  {
    u32 current_index = 0;
    for (u32 i = 0; i < phi_count - 1; i += 1)
    {
      mesh->indecies[current_index] = 0;
      current_index += 1;
      mesh->indecies[current_index] = i + 1;
      current_index += 1;
      mesh->indecies[current_index] = i + 2;
      current_index += 1;
    }

    mesh->indecies[current_index] = 0;
    current_index += 1;
    mesh->indecies[current_index] = phi_count;
    current_index += 1;
    mesh->indecies[current_index] = 1;
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

        mesh->indecies[current_index] = QuadIndecies[0];
        current_index += 1;
        mesh->indecies[current_index] = QuadIndecies[1];
        current_index += 1;
        mesh->indecies[current_index] = QuadIndecies[2];
        current_index += 1;
        mesh->indecies[current_index] = QuadIndecies[0];
        current_index += 1;
        mesh->indecies[current_index] = QuadIndecies[2];
        current_index += 1;
        mesh->indecies[current_index] = QuadIndecies[3];
        current_index += 1;
      }

      const u32 QuadIndecies[4] = {
        phi_count + i * phi_count,
        phi_count + (i + 1) * phi_count,
        1 + (i + 1) * phi_count,
        1 + i * phi_count,
      };

      mesh->indecies[current_index] = QuadIndecies[0];
      current_index += 1;
      mesh->indecies[current_index] = QuadIndecies[1];
      current_index += 1;
      mesh->indecies[current_index] = QuadIndecies[2];
      current_index += 1;
      mesh->indecies[current_index] = QuadIndecies[0];
      current_index += 1;
      mesh->indecies[current_index] = QuadIndecies[2];
      current_index += 1;
      mesh->indecies[current_index] = QuadIndecies[3];
      current_index += 1;
    }

    const u32 south_index = vertex_count - 1;
    for (u32 i = 0; i < phi_count - 1; i += 1)
    {
      mesh->indecies[current_index] = south_index;
      current_index += 1;
      mesh->indecies[current_index] = south_index - phi_count + i + 1;
      current_index += 1;
      mesh->indecies[current_index] = south_index - phi_count + i;
      current_index += 1;
    }

    mesh->indecies[current_index] = south_index;
    current_index += 1;
    mesh->indecies[current_index] = south_index - phi_count;
    current_index += 1;
    mesh->indecies[current_index] = south_index - 1;
    current_index += 1;
  }

  return mesh;
}

i32 main()
{
  OS_Window window = OS_CreateWindow("Sphere", MakeVec2u(1280, 720));
  R_VK_Init(&window);
  Arena* arena = AllocateArena(Megabytes(128));

  OS_ShowWindow(&window);

  R_Mesh* uv_sphere = GenerateUVSphere(arena, MakeVec3f(0.5f, 0.5f, -1.0f), 0.25f, 20, 20);

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

        default: break;
      }

      event = event->next;
    }

    R_AddMeshToDrawList(uv_sphere);

    R_DrawFrame();

    R_EndFrame();
    ResetArena(arena);
  }

  return 0;
}
