#include "base/base_include.h"
#include "os/os_include.h"
#include "render/r_include.h"

#include "base/base_include.c"
#include "os/os_include.c"
#include "render/r_include.c"

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"

  typedef struct Vertex Vertex;
  struct Vertex
  {
    Vec3 position;
    Vec2 uv;
  };

  typedef U32 EntityID;
#define EntityID_NIL 0

  typedef struct Entity Entity;
  struct Entity
  {
    EntityID id;
    Vec3 position;
    F32 rotation;

    I32 vertecies_count;
    Vertex* vertecies;
  };
  Entity EntityDefaultValue = {0};
  DefineArray(Entity, EntityArray, EntityDefaultValue) // -- AlNov: @TODO It can be better to set DefaultValue for Array through parameter

  func void DrawEntity(R_CommandBuffer command_buffer, R_Buffer buffer, Entity* entity);

  typedef struct Camera Camera;
  struct Camera
  {
    Vec3 position;
    Vec3 front;
    Vec3 right;
    Vec3 up;
    F32 yaw;
    F32 pitch;
  };

  typedef struct AppState AppState;
  struct AppState
  {
    Arena* arena;
    Arena* frame_arena;
    OS_Window window;
    F32 delta_time;
    B32 is_window_closed;
    B32 mouse_inside;
    Vec2F32 last_mouse_position;

    R_GraphicsPipeline grid_pipeline;
    R_GraphicsPipeline square_pipeline;
    R_GraphicsPipeline mesh_pipeline;

    R_TextureSampler texture_sampler;
    R_Texture mesh_texture;

    R_Texture depth_texture; // -AlNov: @TODO should it be created for R_VK_Swapchain?
    R_Texture test_texture;

    Camera camera;

    U32 hover_entity_id;
    
    F32 grid_scale;

    EntityArray entities;
    const Entity* selected_entity;
  } app_state;

  func void HandleEvents(Arena* arena, AppState* state);

  I32 main(void)
  {
    app_state.arena = AllocateArena(Megabytes(64));
    app_state.frame_arena = AllocateArena(Megabytes(8));
    app_state.is_window_closed = 0;
    app_state.grid_scale = 2000.0f;
    app_state.camera.position = MakeVec3(0.0f, 2.5f, 4.5f);
    app_state.camera.front = MakeVec3(1.0f, 0.0f, -1.0f);
    app_state.camera.right = MakeVec3(1.0f, 0.0f, 1.0f);
    app_state.camera.up = MakeVec3(0.0f, 1.0f, 0.0f);
    app_state.camera.yaw = -90.0f;
    app_state.camera.pitch = -30.0f;
    app_state.entities = EntityArrayAllocate(app_state.arena, 128);
    app_state.selected_entity = &EntityDefaultValue;

    F32 new_variable = 0;
   
    OS_Init(Megabytes(32));

    OS_CreateWindow(Str8C("Vulkan Triangle"), MakeVec2U32(1270, 720), &app_state.window);
    OS_ShowWindow(&app_state.window);

    R_Init(R_RENDERER_TYPE_VK, &app_state.window);

    R_CommandBuffer command_buffer = R_GetCommandBuffer();

    R_BufferUsageFlags triangle_buffer_usage_flags = R_BUFFER_USAGE_FLAG_VERTEX|R_BUFFER_USAGE_FLAG_INDEX|R_BUFFER_USAGE_FLAG_UNIFORM;
    R_Buffer data_buffer = R_CreateBuffer(Megabytes(64), triangle_buffer_usage_flags, R_BUFFER_PROPERTY_FLAG_HOST_COHERENT);
    R_Buffer transfer_buffer = R_CreateBuffer(Megabytes(64), R_BUFFER_USAGE_FLAG_TRANSFER, R_BUFFER_PROPERTY_FLAG_HOST_COHERENT);

    R_TextureSamplerCreateInfo sampler_info = {
      .mag_filter = R_FILTER_TYPE_LINEAR,
      .min_filter = R_FILTER_TYPE_LINEAR,
      .address_mode_u = R_SAMPLER_ADDRESS_MODE_REPEAT,
      .address_mode_v = R_SAMPLER_ADDRESS_MODE_REPEAT,
      .address_mode_w = R_SAMPLER_ADDRESS_MODE_REPEAT,
      .mipmap_mode = R_SAMPLER_MIPMAP_MODE_LINEAR,
    };
    app_state.texture_sampler = R_CreateTextureSampler(&sampler_info);

    R_TextureCreateInfo test_texture_info = {
      .type = R_TEXTURE_TYPE_2D,
      .format = R_TEXTURE_FORMAT_R16_UINT,
      .usage_flags = R_TEXTURE_USAGE_FLAG_COLOR_ATTACHMENT | R_TEXTURE_USAGE_FLAG_TRANSFER_SRC,
      .width = app_state.window.size.w,
      .height = app_state.window.size.h,
      .depth = 1,
      .num_levels = 1
    };
    app_state.test_texture = R_CreateTexture(&test_texture_info);

    R_TextureCreateInfo depth_texture_info = {
      .type = R_TEXTURE_TYPE_2D,
      .format = R_TEXTURE_FORMAT_D16_UNORM,
      .usage_flags = R_TEXTURE_USAGE_FLAG_DEPTH_STENCIL_ATTACHMENT,
      .width = app_state.window.size.w,
      .height = app_state.window.size.h,
      .depth = 1,
      .num_levels = 1
    };
    app_state.depth_texture = R_CreateTexture(&depth_texture_info);


    Str8 texture_path = Str8C("./data/uv_checker.png");
    I32 tex_width = 0;
    I32 tex_height = 0;
    I32 tex_channels = 0;
    U8* tex_pixels = stbi_load(CFromStr8(texture_path), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);

    if (!tex_pixels)
    {
      LOG_ERROR("Cannot load texture %s\n", CFromStr8(texture_path));
    }
    I32 texture_size = tex_width * tex_height * 4;

    R_TextureCreateInfo mesh_texture_info = {
      .type = R_TEXTURE_TYPE_2D,
      .format = R_TEXTURE_FORMAT_R8G8B8A8_SRGB,
      .usage_flags = R_TEXTURE_USAGE_FLAG_SAMPLED | R_TEXTURE_USAGE_FLAG_TRANSFER_DST,
      .width = tex_width,
      .height = tex_height,
      .depth = 1,
      .num_levels = 1,
    };
    app_state.mesh_texture = R_CreateTexture(&mesh_texture_info);
    U64 mesh_texture_offset = R_PushBuffer(data_buffer, tex_pixels, texture_size);
    R_CopyBufferToTexture(0, data_buffer, mesh_texture_offset, texture_size, app_state.mesh_texture);

    // --AlNov: Word Grid
    {
      R_ShaderCreateInfo grid_vertex_shader_info = {
        .file_name = Str8C("./data/shaders/grid.vs.glsl"),
        .type = R_SHADER_TYPE_VERTEX,
        .global_uniforms_count = 1,
        .instance_uniforms_count = 1,
      };
      R_Shader grid_vertex_shader = R_CreateShader(app_state.arena, &grid_vertex_shader_info);
      R_ShaderCreateInfo grid_fragment_shader_info = {
        .file_name = Str8C("./data/shaders/grid.fs.glsl"),
        .type = R_SHADER_TYPE_FRAGMENT,
        .global_uniforms_count = 1,
        .instance_uniforms_count = 0,
      };
      R_Shader grid_fragment_shader = R_CreateShader(app_state.arena, &grid_fragment_shader_info);

      R_GraphicsPipelineColorTargetInfo grid_pipeline_color_target_infos[] = {
        {
          .format = R_GetSwapchainTextureFormat(),
          .blend_enable = 1,
        },
      };
      R_GraphicsPipelineCreateInfo grid_pipeline_info = {
        .vertex_shader = grid_vertex_shader,
        .fragment_shader = grid_fragment_shader,
        .color_targets_count = CountArrayElements(grid_pipeline_color_target_infos),
        .color_target_infos = grid_pipeline_color_target_infos,
        .depth_stencil_state = {
          .depth_test_enable = 1,
          .depth_write_enable = 0,
          .depth_compare_operation = R_COMPARE_OPERATION_GREATER,
          .depth_target_format = R_GetTextureFormat(app_state.depth_texture),
        },
      };
      app_state.grid_pipeline = R_CreateGraphicsPipeline(&grid_pipeline_info);
    }

    // Square Pipeline
    {
      R_ShaderCreateInfo square_vertex_shader_info = {
        .file_name = Str8C("./data/shaders/sdf/sdf_vs.glsl"),
        .type = R_SHADER_TYPE_VERTEX,
        .global_uniforms_count = 1,
        .instance_uniforms_count = 0,
      };
      R_Shader square_vertex_shader = R_CreateShader(app_state.arena, &square_vertex_shader_info);
      R_ShaderCreateInfo square_fragment_shader_info = {
        .file_name = Str8C("./data/shaders/sdf/sdf_box_fs.glsl"),
        .type = R_SHADER_TYPE_FRAGMENT,
        .global_uniforms_count = 1,
        .instance_uniforms_count = 0,
      };
      R_Shader square_fragment_shader = R_CreateShader(app_state.arena, &square_fragment_shader_info);

      R_GraphicsPipelineColorTargetInfo square_pipeline_color_target_infos[] = {
        {
          .format = R_GetSwapchainTextureFormat(),
          .blend_enable = 1,
        },
      };

      R_GraphicsPipelineCreateInfo square_pipeline_info = {
        .vertex_shader = square_vertex_shader,
        .fragment_shader = square_fragment_shader,
        .color_targets_count = CountArrayElements(square_pipeline_color_target_infos),
        .color_target_infos = square_pipeline_color_target_infos,
        .depth_stencil_state = {
          .depth_test_enable = 0,
          .depth_write_enable = 0,
          .depth_compare_operation = R_COMPARE_OPERATION_GREATER,
          .depth_target_format = R_GetTextureFormat(app_state.depth_texture),
        },
      };
      app_state.square_pipeline = R_CreateGraphicsPipeline(&square_pipeline_info);
    }

    // --AlNov: UV coordinates writen in Bottom-Left coordinate system.
    // But Vulkan uses Top-Left coordinate system.
    Vertex cube_vertecies[] = {
      {.position = {-0.5f, -0.5f, -0.5f}, .uv = {0.0f, 0.0f}},
      {.position = { 0.5f, -0.5f, -0.5f}, .uv = {1.0f, 0.0f}},
      {.position = { 0.5f,  0.5f, -0.5f}, .uv = {1.0f, 1.0f}},
      {.position = { 0.5f,  0.5f, -0.5f}, .uv = {1.0f, 1.0f}},
      {.position = {-0.5f,  0.5f, -0.5f}, .uv = {0.0f, 1.0f}},
      {.position = {-0.5f, -0.5f, -0.5f}, .uv = {0.0f, 0.0f}},

      {.position = {-0.5f, -0.5f,  0.5f}, .uv = {0.0f, 0.0f}},
      {.position = { 0.5f, -0.5f,  0.5f}, .uv = {1.0f, 0.0f}},
      {.position = { 0.5f,  0.5f,  0.5f}, .uv = {1.0f, 1.0f}},
      {.position = { 0.5f,  0.5f,  0.5f}, .uv = {1.0f, 1.0f}},
      {.position = {-0.5f,  0.5f,  0.5f}, .uv = {0.0f, 1.0f}},
      {.position = {-0.5f, -0.5f,  0.5f}, .uv = {0.0f, 0.0f}},

      {.position = {-0.5f,  0.5f,  0.5f}, .uv = {1.0f, 0.0f}},
      {.position = {-0.5f,  0.5f, -0.5f}, .uv = {1.0f, 1.0f}},
      {.position = {-0.5f, -0.5f, -0.5f}, .uv = {0.0f, 1.0f}},
      {.position = {-0.5f, -0.5f, -0.5f}, .uv = {0.0f, 1.0f}},
      {.position = {-0.5f, -0.5f,  0.5f}, .uv = {0.0f, 0.0f}},
      {.position = {-0.5f,  0.5f,  0.5f}, .uv = {1.0f, 0.0f}},

      {.position = { 0.5f,  0.5f,  0.5f}, .uv = {1.0f, 0.0f}},
      {.position = { 0.5f,  0.5f, -0.5f}, .uv = {1.0f, 1.0f}},
      {.position = { 0.5f, -0.5f, -0.5f}, .uv = {0.0f, 1.0f}},
      {.position = { 0.5f, -0.5f, -0.5f}, .uv = {0.0f, 1.0f}},
      {.position = { 0.5f, -0.5f,  0.5f}, .uv = {0.0f, 0.0f}},
      {.position = { 0.5f,  0.5f,  0.5f}, .uv = {1.0f, 0.0f}},

      {.position = {-0.5f, -0.5f, -0.5f}, .uv = {0.0f, 1.0f}},
      {.position = { 0.5f, -0.5f, -0.5f}, .uv = {1.0f, 1.0f}},
      {.position = { 0.5f, -0.5f,  0.5f}, .uv = {1.0f, 0.0f}},
      {.position = { 0.5f, -0.5f,  0.5f}, .uv = {1.0f, 0.0f}},
      {.position = {-0.5f, -0.5f,  0.5f}, .uv = {0.0f, 0.0f}},
      {.position = {-0.5f, -0.5f, -0.5f}, .uv = {0.0f, 1.0f}},

      {.position = {-0.5f,  0.5f, -0.5f}, .uv = {0.0f, 1.0f}},
      {.position = { 0.5f,  0.5f, -0.5f}, .uv = {1.0f, 1.0f}},
      {.position = { 0.5f,  0.5f,  0.5f}, .uv = {1.0f, 0.0f}},
      {.position = { 0.5f,  0.5f,  0.5f}, .uv = {1.0f, 0.0f}},
      {.position = {-0.5f,  0.5f,  0.5f}, .uv = {0.0f, 0.0f}},
      {.position = {-0.5f,  0.5f, -0.5f}, .uv = {0.0f, 1.0f}},
    };

    Entity cube = {
      .id = 1,
      .position = MakeVec3(0.0f, 0.0f, 0.0f),
      .vertecies_count = CountArrayElements(cube_vertecies),
      .vertecies = cube_vertecies,
    };
    EntityArrayAdd(&app_state.entities, cube);
    Entity cube_1 = {
      .id = 2,
      .position = MakeVec3(1.0f, 2.0f, 0.0f),
      .vertecies_count = CountArrayElements(cube_vertecies),
      .vertecies = cube_vertecies,
    };
    EntityArrayAdd(&app_state.entities, cube_1);

    // Mesh Pipeline
    {
      R_ShaderCreateInfo mesh_vertex_shader_info = {
        .file_name = Str8C("./data/shaders/mesh.vs.glsl"),
        .type = R_SHADER_TYPE_VERTEX,
        .global_uniforms_count = 1,
        .instance_uniforms_count = 1,
      };
      R_Shader mesh_vertex_shader = R_CreateShader(app_state.arena, &mesh_vertex_shader_info);
      R_ShaderCreateInfo mesh_fragment_shader_info = {
        .file_name = Str8C("./data/shaders/mesh.fs.glsl"),
        .type = R_SHADER_TYPE_FRAGMENT,
        .global_uniforms_count = 0,
        .instance_uniforms_count = 1,
        .instance_samplers_count = 1,
      };
      R_Shader mesh_fragment_shader = R_CreateShader(app_state.arena, &mesh_fragment_shader_info);

      R_VertexAttribute mesh_vertex_attributes[] = {
        {
          .location = 0,
          .format = R_VERTEX_ATTRIBUTE_FORMAT_VEC3F32,
          .offset = offsetof(Vertex, position),
        },
        {
          .location = 1,
          .format = R_VERTEX_ATTRIBUTE_FORMAT_VEC2F32,
          .offset = offsetof(Vertex, uv),
        }
      };

      R_GraphicsPipelineColorTargetInfo mesh_pipeline_color_target_infos[] = {
        {
          .format = R_GetSwapchainTextureFormat(),
          .blend_enable = 1,
        },
        {
          .format = R_GetTextureFormat(app_state.test_texture),
          .blend_enable = 0,
        },
      };
      R_GraphicsPipelineCreateInfo mesh_pipeline_info = {
        .vertex_shader = mesh_vertex_shader,
        .fragment_shader = mesh_fragment_shader,
        .vertex_attributes_count = CountArrayElements(mesh_vertex_attributes),
        .vertex_attributes = mesh_vertex_attributes,
        .color_targets_count = CountArrayElements(mesh_pipeline_color_target_infos),
        .color_target_infos = mesh_pipeline_color_target_infos,
        .depth_stencil_state = {
          .depth_test_enable = 1,
          .depth_write_enable = 1,
          .depth_compare_operation = R_COMPARE_OPERATION_GREATER,
          .depth_target_format = R_GetTextureFormat(app_state.depth_texture),
        },
      };
      app_state.mesh_pipeline = R_CreateGraphicsPipeline(&mesh_pipeline_info);
    }

    // AlNov: AppLoop
    F32 begin_time = OS_CurrentTimeSeconds();
    U16 test_texture_values_offset = 0;
    while (!app_state.is_window_closed)
    {
      HandleEvents(app_state.frame_arena, &app_state);

      // Update World
      {
        if (app_state.hover_entity_id != 0)
        {
          Entity* hover_entity = EntityArrayGetPointer(&app_state.entities, app_state.hover_entity_id - 1);
          hover_entity->rotation += 25.0f*app_state.delta_time;
        }
      }

      R_ResetBuffer(data_buffer);
      struct
      {
        Mat4 projection;
      } square_global_vertex_data;
      U64 square_global_vertex_data_offset = R_PushBuffer(data_buffer, (U8*)&square_global_vertex_data, sizeof(square_global_vertex_data));

      struct
      {
        Vec3 color;
        F32 rotation;
        Vec2 position;
        Vec2 size;
      } square_global_fragment_data;
      square_global_fragment_data.color = MakeVec3(0.7f, 0.8f, 0.7f);
      if (app_state.hover_entity_id != 0)
      {
        square_global_fragment_data.color = MakeVec3(0.7f, 0.2f, 0.3f);
      }
      square_global_fragment_data.rotation = 0.0f;
      square_global_fragment_data.position = app_state.last_mouse_position;
      square_global_fragment_data.size = MakeVec2(15.0f, 15.0f);
      U64 square_global_fragment_data_offset = R_PushBuffer(data_buffer, (U8*)&square_global_fragment_data, sizeof(square_global_fragment_data));

      // Draw
      R_Texture swapchain_texture = R_AcquireSwapchainTexture(command_buffer);
      R_BeginCommandBuffer(command_buffer);
      {
        RectI32 viewport = {
          .x = 0,
          .y = 0,
          .w = app_state.window.size.w,
          .h = app_state.window.size.h,
        };
        RectI32 scissor = viewport;
        R_SetViewport(command_buffer, viewport);
        R_SetScissor(command_buffer, scissor);

        // Entity Pass
        R_ColorTarget color_targets[] = {
          {
            .texture = swapchain_texture,
            .load_operation = R_ATTACHMENT_LOAD_OPERATION_CLEAR,
            .store_operation = R_ATTACHMENT_STORE_OPERATION_STORE,
            .clear_color = RGBAFromHex(0x1A1D26FF),
          },
          {
            .texture = app_state.test_texture,
            .load_operation = R_ATTACHMENT_LOAD_OPERATION_CLEAR,
            .store_operation = R_ATTACHMENT_STORE_OPERATION_STORE,
            .clear_color = RGBAFromHex(0x00000000),
          },
        };
        R_DepthStencilTarget depth_target = {
          .texture = app_state.depth_texture,
          .depth_load_operation = R_ATTACHMENT_LOAD_OPERATION_CLEAR,
          .depth_store_operation = R_ATTACHMENT_STORE_OPERATION_STORE,
          .clear_depth = 0.0f,
        };
        R_BeginRenderPass(command_buffer, CountArrayElements(color_targets), color_targets, &depth_target);
        {
          for (I32 i = 0; i < app_state.entities.length; i += 1)
          {
            DrawEntity(command_buffer, data_buffer, EntityArrayGetPointer(&app_state.entities, i));
          }
        }
        R_EndRenderPass(command_buffer, 0);

        // Draw Grid (UI)
        R_ColorTarget grid_color_target = {
          .texture = swapchain_texture,
          .load_operation = R_ATTACHMENT_LOAD_OPERATION_LOAD,
          .store_operation = R_ATTACHMENT_STORE_OPERATION_STORE,
        };
        R_DepthStencilTarget grid_depth_target = {
          .texture = app_state.depth_texture,
          .depth_load_operation = R_ATTACHMENT_LOAD_OPERATION_LOAD,
          .depth_store_operation = R_ATTACHMENT_STORE_OPERATION_DONT_CARE,
        };
        R_BeginRenderPass(command_buffer, 1, &grid_color_target, &grid_depth_target);
        {
          struct
          {
            Mat4 view_matrix;
            Mat4 projection_matrix;
          } grid_global_vertex_data;
          grid_global_vertex_data.view_matrix = MakeLookAtMat4(
              app_state.camera.position,
              AddVec3(app_state.camera.position, app_state.camera.front),
              app_state.camera.up);
          grid_global_vertex_data.projection_matrix = MakePerspectiveMat4(
              45.0f, (F32)app_state.window.size.w/(F32)app_state.window.size.h,
              0.1f, 100.0f);
          struct
          {
            Vec3 position;
            F32 grid_scale;
          } grid_instance_vertex_data;
          grid_instance_vertex_data.position = MakeVec3(app_state.camera.position.x, 0.0f, app_state.camera.position.z);
          grid_instance_vertex_data.grid_scale = app_state.grid_scale;

          struct
          {
            Vec4 color;
          } grid_global_fragment_data;
          grid_global_fragment_data.color = RGBAFromHex(0x95B8D177);

          U64 grid_global_vertex_data_offset = R_PushBuffer(data_buffer, (U8*)&grid_global_vertex_data, sizeof(grid_global_vertex_data));
          U64 grid_instance_vertex_data_offset = R_PushBuffer(data_buffer, (U8*)&grid_instance_vertex_data, sizeof(grid_instance_vertex_data));
          U64 grid_global_fragment_data_offset = R_PushBuffer(data_buffer, (U8*)&grid_global_fragment_data, sizeof(grid_global_fragment_data));

          R_UniformBufferBindingInfo grid_vertex_shader_global_uniform = {
            .buffer = data_buffer,
            .offset = grid_global_vertex_data_offset,
            .size = sizeof(grid_global_vertex_data),
          };
          R_UniformBufferBindingInfo grid_vertex_shader_instance_uniform = {
            .buffer = data_buffer,
            .offset = grid_instance_vertex_data_offset,
            .size = sizeof(grid_instance_vertex_data),
          };
          R_UniformBufferBindingInfo grid_fragment_shader_global_uniform = {
            .buffer = data_buffer,
            .offset = grid_global_fragment_data_offset,
            .size = sizeof(grid_global_fragment_data),
          };

          R_BindGraphicsPipeline(command_buffer, app_state.grid_pipeline);
          R_BindGlobalVertexShaderData(command_buffer, 1, &grid_vertex_shader_global_uniform, 0, 0);
          R_BindInstanceVertexShaderData(command_buffer, 1, &grid_vertex_shader_instance_uniform, 0, 0);
          R_BindGlobalFragmentShaderData(command_buffer, 1, &grid_fragment_shader_global_uniform, 0, 0);
          R_DrawPrimitives(command_buffer, 6, 1, 0, 0);

          if (app_state.mouse_inside)
          {
            R_UniformBufferBindingInfo square_vertex_shader_global_uniform = {
              .buffer = data_buffer,
              .offset = square_global_vertex_data_offset,
              .size = sizeof(square_global_vertex_data),
            };
            R_UniformBufferBindingInfo square_fragment_shader_global_uniform = {
              .buffer = data_buffer,
              .offset = square_global_fragment_data_offset,
              .size = sizeof(square_global_fragment_data),
            };
            R_BindGraphicsPipeline(command_buffer,app_state.square_pipeline);
            R_BindGlobalVertexShaderData(command_buffer, 1, &square_vertex_shader_global_uniform, 0, 0);
            R_BindGlobalFragmentShaderData(command_buffer, 1, &square_fragment_shader_global_uniform, 0, 0);
            R_DrawPrimitives(command_buffer, 6, 1, 0, 0);
          }
        }
        R_EndRenderPass(command_buffer, 0);

        // R_CopyTexture(command_buffer, swapchain_texture, app_state.test_texture);
        test_texture_values_offset = R_CopyTextureToBuffer(command_buffer, app_state.test_texture, transfer_buffer);
      }
      R_SubmitCommandBuffer(command_buffer);

      R_PresentTexture(command_buffer, swapchain_texture);

      U64 pixel_offset = sizeof(U16)*(((I32)app_state.window.size.x*(I32)app_state.last_mouse_position.y) + (I32)app_state.last_mouse_position.x);
      R_VK_BufferGetData(transfer_buffer, test_texture_values_offset + pixel_offset, &app_state.hover_entity_id, sizeof(U16));
      // LOG_DEBUG("MOUSE: %.1fx, %.1fy\tOFFSET: %d, PIXEL VALUE: %d\n", app_state.last_mouse_position.x, app_state.last_mouse_position.y, pixel_offset, app_state.hover_entity_id);

      ResetArena(app_state.frame_arena);

      F32 end_time = OS_CurrentTimeSeconds();
      app_state.delta_time = end_time - begin_time;
      begin_time = end_time;
    }

    R_VK_DestroyBuffer(data_buffer);
    R_Shutdown(); // -AlNov: @BUG Driver Timeout (Vulkan Shutdown is not implemented)
    
    return 0;
  }

  func void
  DrawEntity(R_CommandBuffer command_buffer, R_Buffer buffer, Entity* entity)
  {
    struct
    {
      Mat4 view_matrix;
      Mat4 projection_matrix;
    } global_vertex_data;
    global_vertex_data.view_matrix = MakeLookAtMat4(
        app_state.camera.position,
        AddVec3(app_state.camera.position, app_state.camera.front),
        app_state.camera.up);
    global_vertex_data.projection_matrix = MakePerspectiveMat4(
        45.0f, (F32)app_state.window.size.w/(F32)app_state.window.size.h,
        0.1f, 100.0f);

    U64 global_vertex_data_offset = R_PushBuffer(buffer, (U8*)&global_vertex_data, sizeof(global_vertex_data));

    U64 mesh_vertex_data_offset = R_PushBuffer(buffer, (U8*)entity->vertecies, sizeof(entity->vertecies[0])*entity->vertecies_count);
    struct
    {
      Mat4 instance_matrix;
    } mesh_instance_vertex_data;
    // mesh_instance_vertex_data.instance_matrix = MakeTransposeMat4(entity->position);
    mesh_instance_vertex_data.instance_matrix = MakeMat4(1.0f);
    mesh_instance_vertex_data.instance_matrix = MulMat4(MakeRotationMat4(MakeVec3(0.0f, 1.0f, 0.0), RadiansFromDegrees(entity->rotation)), mesh_instance_vertex_data.instance_matrix);
    mesh_instance_vertex_data.instance_matrix = MulMat4(MakeTransposeMat4(entity->position), mesh_instance_vertex_data.instance_matrix);
    if (app_state.hover_entity_id == entity->id)
    {
    }
    U64 mesh_instance_vertex_data_offset = R_PushBuffer(buffer, (U8*)&mesh_instance_vertex_data, sizeof(mesh_instance_vertex_data));

    struct
    {
      F32 entity_id;
    } mesh_instance_fragment_data;
    mesh_instance_fragment_data.entity_id = entity->id;
    U64 mesh_instance_fragment_data_offset = R_PushBuffer(buffer, (U8*)&mesh_instance_fragment_data, sizeof(mesh_instance_fragment_data));

    R_UniformBufferBindingInfo mesh_vertex_shader_global_uniform = {
      .buffer = buffer,
      .offset = global_vertex_data_offset,
      .size = sizeof(global_vertex_data),
    };
    R_UniformBufferBindingInfo mesh_vertex_shader_instance_uniform = {
      .buffer = buffer,
      .offset = mesh_instance_vertex_data_offset,
      .size = sizeof(mesh_instance_vertex_data),
    };
    R_UniformBufferBindingInfo mesh_fragment_shader_instance_uniform = {
      .buffer = buffer,
      .offset = mesh_instance_fragment_data_offset,
      .size = sizeof(mesh_instance_fragment_data),
    };
    R_SamplerBindingInfo mesh_fragment_shader_instance_sampler = {
      .sampler = app_state.texture_sampler,
      .texture = app_state.mesh_texture,
    };

    R_BindGraphicsPipeline(command_buffer, app_state.mesh_pipeline);
    R_BindGlobalVertexShaderData(command_buffer, 1, &mesh_vertex_shader_global_uniform, 0, 0);
    R_BindInstanceVertexShaderData(command_buffer, 1, &mesh_vertex_shader_instance_uniform, 0, 0);
    R_BindInstanceFragmentShaderData(command_buffer, 1, &mesh_fragment_shader_instance_uniform, 1, &mesh_fragment_shader_instance_sampler);
    R_BindVertexBuffer(command_buffer, buffer, mesh_vertex_data_offset);
    R_DrawPrimitives(command_buffer, entity->vertecies_count, 1, 0, 0);
  }

  func void
  HandleEvents(Arena* arena, AppState* state)
  {
    OS_EventList event_list = OS_GetEventList(arena, &state->window);

    if (OS_IsKeyPressed(OS_KEY_ESC))
    {
      state->is_window_closed = 1;
    }

    Vec3 direction = MakeVec3(0.0f, 0.0f, 0.0f);
    F32 speed = 2.0f;
    if (OS_IsKeyDown(OS_KEY_W))
    {
      direction = AddVec3(direction, state->camera.front);
    }
    if (OS_IsKeyDown(OS_KEY_S))
    {
      direction = SubVec3(direction, state->camera.front);
    }
    if (OS_IsKeyDown(OS_KEY_D))
    {
      direction = AddVec3(direction, state->camera.right);
    }
    if (OS_IsKeyDown(OS_KEY_A))
    {
      direction = SubVec3(direction, state->camera.right);
    }
    state->camera.position = AddVec3(state->camera.position, ScaleVec3(NormalizeVec3(direction), speed*state->delta_time));

    if (OS_IsKeyDown(OS_KEY_ARROW_LEFT))
    {
      state->camera.yaw -= 25.0f*state->delta_time;
    }
    if (OS_IsKeyDown(OS_KEY_ARROW_RIGHT))
    {
      state->camera.yaw += 25.0f*state->delta_time;
    }
    if (OS_IsKeyDown(OS_KEY_ARROW_UP))
    {
      state->camera.pitch += 25.0f*state->delta_time;
    }
    if (OS_IsKeyDown(OS_KEY_ARROW_DOWN))
    {
      state->camera.pitch -= 25.0f*state->delta_time;
    }
    Vec3 rotation = {0};
    rotation.x = cos(RadiansFromDegrees(state->camera.yaw))*cos(RadiansFromDegrees(state->camera.pitch));
    rotation.y = sin(RadiansFromDegrees(state->camera.pitch));
    rotation.z = sin(RadiansFromDegrees(state->camera.yaw))*cos(RadiansFromDegrees(state->camera.pitch));
    state->camera.front = rotation;

    state->camera.right = NormalizeVec3(CrossVec3(state->camera.front, MakeVec3(0.0f, 1.0f, 0.0f)));
    state->camera.up = CrossVec3(state->camera.right, state->camera.front);
    
    for (OS_EventListNode *event_node = event_list.first; event_node; event_node = event_node->next)
    {
      OS_Event* event = &event_node->data;

      switch (event->type)
      {
        case OS_EVENT_TYPE_EXIT:
        {
          state->is_window_closed = 1;
        } break;

        case OS_EVENT_TYPE_RESIZE:
        {
          // @TODO Window recreated multiple time.
          // I guess, resize event is triggered multiple time. It should be handled only once, after last resizing
          if ((state->window.size.w != event->window_size.w) || (state->window.size.h != event->window_size.h))
          {
            LOG_DEBUG("Old Window Size: %d\t%d\n", state->window.size.w, state->window.size.h);
            state->window.size = event->window_size;
            LOG_DEBUG("New Window Size: %d\t%d\n", state->window.size.w, state->window.size.h);
            R_VK_HandleResize(&state->window);

            R_VK_DestroyTexture(app_state.depth_texture);
            R_TextureCreateInfo depth_texture_info = {
              .type = R_TEXTURE_TYPE_2D,
              .format = R_TEXTURE_FORMAT_D16_UNORM,
              .usage_flags = R_TEXTURE_USAGE_FLAG_DEPTH_STENCIL_ATTACHMENT,
              .width = app_state.window.size.w,
              .height = app_state.window.size.h,
              .depth = 1,
              .num_levels = 1
            };
            app_state.depth_texture = R_CreateTexture(&depth_texture_info);

            R_VK_DestroyTexture(app_state.test_texture);
            R_TextureCreateInfo test_texture_info = {
              .type = R_TEXTURE_TYPE_2D,
              .format = R_TEXTURE_FORMAT_R16_UINT,
              .usage_flags = R_TEXTURE_USAGE_FLAG_COLOR_ATTACHMENT | R_TEXTURE_USAGE_FLAG_TRANSFER_SRC | R_TEXTURE_USAGE_FLAG_TRANSFER_DST,
              .width = app_state.window.size.w,
              .height = app_state.window.size.h,
              .depth = 1,
              .num_levels = 1
            };
            app_state.test_texture = R_CreateTexture(&test_texture_info);

          }
          // LOG_INFO("New window size: %d w %d h\n\n", state->window.size.x, state->window.size.y);
          // Renderer.HandleResize(&state->window);
        } break;

        case OS_EVENT_TYPE_MOUSE_ENTER:
        {
          app_state.mouse_inside = 1;
        } break;

        case OS_EVENT_TYPE_MOUSE_LEAVE:
        {
          app_state.mouse_inside = 0;
        } break;

        case OS_EVENT_TYPE_MOUSE_MOVE:
        {
          // LOG_DEBUG("MousePosition: %.3f, %.3f\n", event->mouse_position.x, event->mouse_position.y);
          // LOG_DEBUG("Virtual Cursor: %.3f, %.3f\n", state->window.virtual_cursor_position.x, state->window.virtual_cursor_position.y);
          Vec2F32 d_position = SubVec2F32(state->window.virtual_cursor_position, state->last_mouse_position);
          Vec2F32 mouse_direction = NormalizeVec2F32(d_position);

          state->last_mouse_position = event->mouse_position;
        } break;
        
        default: break;
      }
    }
  }
