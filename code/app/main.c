#include "base/base_include.h"
#include "os/os_include.h"
#include "render/r_include.h"

#include "base/base_include.c"
#include "os/os_include.c"
#include "render/r_include.c"

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
	Vec2F32 last_mouse_position;

  R_Texture* depth_texture;

	Camera camera;
	
	F32 grid_scale;
} app_state;

func void HandleEvents(Arena* arena, AppState* state);

I32 main(void)
{
  app_state.arena = AllocateArena(Megabytes(64));
  app_state.frame_arena = AllocateArena(Megabytes(8));
  app_state.is_window_closed = false;
	app_state.grid_scale = 1000.0f;
	app_state.camera.position = MakeVec3(0.0f, 1.0f, 6.0f);
	app_state.camera.front = MakeVec3(1.0f, 0.0f, -1.0f);
	app_state.camera.right = MakeVec3(1.0f, 0.0f, 1.0f);
	app_state.camera.up = MakeVec3(0.0f, 1.0f, 0.0f);
  app_state.camera.yaw = -90.0f;
  app_state.camera.pitch = -30.0f;

  F32 new_variable = 0;
 
  OS_Init(Megabytes(32));

  OS_CreateWindow(Str8C("Vulkan Triangle"), MakeVec2U32(1270, 720), &app_state.window);
  OS_ShowWindow(&app_state.window);

  R_Init(R_RENDERER_TYPE_VK, &app_state.window);

	R_BufferUsageFlags triangle_buffer_usage_flags = R_BUFFER_USAGE_FLAG_VERTEX|R_BUFFER_USAGE_FLAG_INDEX|R_BUFFER_USAGE_FLAG_UNIFORM;
	R_Buffer* data_buffer = R_CreateBuffer(Megabytes(4), triangle_buffer_usage_flags, R_BUFFER_PROPERTY_FLAG_HOST_COHERENT);

	// --AlNov: Word Grid
	R_Shader grid_vertex_shader = R_CreateShader(app_state.arena, Str8C("./data/shaders/grid.vs.glsl"), R_SHADER_TYPE_VERTEX, 1);
	R_Shader grid_fragment_shader = R_CreateShader(app_state.arena, Str8C("./data/shaders/grid.fs.glsl"), R_SHADER_TYPE_FRAGMENT, 1);

  R_TextureFormat grid_pipeline_color_target_format = R_GetSwapchainTextureFormat();
	R_GraphicsPipelineCreateInfo grid_pipeline_info = {
		.vertex_shader = grid_vertex_shader,
		.fragment_shader = grid_fragment_shader,
    .depth_stencil_state = {
      .depth_test_enable = true,
      .depth_write_enable = false,
      .depth_compare_operation = R_COMPARE_OPERATION_GREATER,
    },
    .target_info = {
      .color_targets_count = 1,
      .color_targets_formats = &grid_pipeline_color_target_format,
      .depth_target_format = R_TEXTURE_FORMAT_D16_UNORM,
    },
	};
	R_GraphicsPipeline* grid_pipeline = R_CreateGraphicsPipeline(&grid_pipeline_info);

  typedef struct Vertex Vertex;
  struct Vertex
  {
    Vec3 position;
    Vec2 uv;
  };

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

  R_Shader mesh_vertex_shader = R_CreateShader(app_state.arena, Str8C("./data/shaders/mesh.vs.glsl"), R_SHADER_TYPE_VERTEX, 1);
  R_Shader mesh_fragment_shader = R_CreateShader(app_state.arena, Str8C("./data/shaders/mesh.fs.glsl"), R_SHADER_TYPE_FRAGMENT, 0);

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

  R_TextureFormat mesh_pipeline_color_target_format = R_GetSwapchainTextureFormat();
  R_GraphicsPipelineCreateInfo mesh_pipeline_info = {
    .vertex_shader = mesh_vertex_shader,
    .fragment_shader = mesh_fragment_shader,
    .vertex_attributes_count = CountArrayElements(mesh_vertex_attributes),
    .vertex_attributes = mesh_vertex_attributes,
    .depth_stencil_state = {
      .depth_test_enable = true,
      .depth_write_enable = true,
      .depth_compare_operation = R_COMPARE_OPERATION_GREATER,
    },
    .target_info = {
      .color_targets_count = 1,
      .color_targets_formats = &mesh_pipeline_color_target_format,
      .depth_target_format = R_TEXTURE_FORMAT_D16_UNORM,
    },
  };
  R_GraphicsPipeline* mesh_pipeline = R_CreateGraphicsPipeline(&mesh_pipeline_info);

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
  
	R_CommandBuffer* command_buffer = R_GetCommandBuffer();

  // AlNov: AppLoop
  F32 begin_time = OS_CurrentTimeSeconds();
  while (!app_state.is_window_closed)
  {
    HandleEvents(app_state.frame_arena, &app_state);

    R_ResetBuffer(data_buffer);

		struct
		{
			Mat4 view_matrix;
			Mat4 projection_matrix;
			Vec3 position;
			F32 grid_scale;
		} grid_global_vertex_data;
		grid_global_vertex_data.view_matrix = MakeLookAtMat4(
				app_state.camera.position,
				AddVec3(app_state.camera.position, app_state.camera.front),
				app_state.camera.up);
		grid_global_vertex_data.projection_matrix = MakePerspectiveMat4(
				45.0f, (F32)app_state.window.size.w/(F32)app_state.window.size.h,
				0.1f, 100.0f);
		grid_global_vertex_data.grid_scale = app_state.grid_scale;
		grid_global_vertex_data.position = MakeVec3(app_state.camera.position.x, 0.0f, app_state.camera.position.z);

		struct
		{
			Vec4 color;
		} grid_global_fragment_data;
		grid_global_fragment_data.color = MakeVec4(0.5f, 0.5f, 0.5f, 0.8f);

		U64 grid_global_vertex_data_offset = R_PushBuffer(data_buffer, (U8*)&grid_global_vertex_data, sizeof(grid_global_vertex_data));
		U64 grid_global_fragment_data_offset = R_PushBuffer(data_buffer, (U8*)&grid_global_fragment_data, sizeof(grid_global_fragment_data));

    U64 mesh_vertex_data_offset = R_PushBuffer(data_buffer, (U8*)cube_vertecies, sizeof(cube_vertecies[0])*CountArrayElements(cube_vertecies));

		// Draw
		R_Texture* swapchain_texture = R_AcquireSwapchainTexture(command_buffer);
		R_BeginCommandBuffer(command_buffer);
		{
			R_ColorTarget color_target = {
				.texture = swapchain_texture,
				.load_operation = R_ATTACHMENT_LOAD_OPERATION_CLEAR,
				.store_operation = R_ATTACHMENT_STORE_OPERATION_STORE,
				.clear_color = MakeVec4(0.1f, 0.1f, 0.1f, 1.0f),
			};
      R_DepthStencilTarget depth_target = {
        .texture = app_state.depth_texture,
        .depth_load_operation = R_ATTACHMENT_LOAD_OPERATION_CLEAR,
        .depth_store_operation = R_ATTACHMENT_STORE_OPERATION_STORE,
        .clear_depth = 0.0f,
      };
			R_BeginRenderPass(command_buffer, 1, &color_target, &depth_target);
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

        R_BindGraphicsPipeline(command_buffer, mesh_pipeline);
				R_BindGlobalVertexUniformData(command_buffer, data_buffer, grid_global_vertex_data_offset, sizeof(grid_global_vertex_data));
        R_BindVertexBuffer(command_buffer, data_buffer, mesh_vertex_data_offset);
        R_DrawPrimitives(command_buffer, CountArrayElements(cube_vertecies), 1, 0, 0);

				R_BindGraphicsPipeline(command_buffer, grid_pipeline);
				R_BindGlobalFragmentUniformData(command_buffer, data_buffer, grid_global_fragment_data_offset, sizeof(grid_global_fragment_data));
				R_DrawPrimitives(command_buffer, 6, 1, 0, 0);
			}
			R_EndRenderPass(command_buffer, 0);
		}
		R_SubmitCommandBuffer(command_buffer);

    ResetArena(app_state.frame_arena);

    F32 end_time = OS_CurrentTimeSeconds();
    app_state.delta_time = end_time - begin_time;
    begin_time = end_time;
  }

  // R_Shutdown(); // -AlNov: @BUG Driver Timeout (Vulkan Shutdown is not implemented)
  
  return 0;
}

func void
HandleEvents(Arena* arena, AppState* state)
{
  ListOS_Event event_list = OS_GetEventList(arena, &state->window);

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
				}
        // LOG_INFO("New window size: %d w %d h\n\n", state->window.size.x, state->window.size.y);
        // Renderer.HandleResize(&state->window);
      } break;

      case OS_EVENT_TYPE_MOUSE_MOVE:
      {
        // LOG_DEBUG("MousePosition: %.3f, %.3f\n", event->mouse_position.x, event->mouse_position.y);
        // LOG_DEBUG("Virtual Cursor: %.3f, %.3f\n", state->window.virtual_cursor_position.x, state->window.virtual_cursor_position.y);
        Vec2F32 d_position = SubVec2F32(state->window.virtual_cursor_position, state->last_mouse_position);
        Vec2F32 mouse_direction = NormalizeVec2F32(d_position);

        state->last_mouse_position = state->window.virtual_cursor_position;
      } break;
      
      default: break;
    }
  }
}
