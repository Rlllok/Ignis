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

	Camera camera;
	
	F32 grid_scale;
} app_state;

func void HandleEvents(Arena* arena, AppState* state);

I32 main(void)
{
  app_state.arena = AllocateArena(Megabytes(64));
  app_state.frame_arena = AllocateArena(Megabytes(8));
  app_state.is_window_closed = false;
	app_state.grid_scale = 1.0f;

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

	R_GraphicsPipelineCreateInfo grid_pipeline_info = {
		.vertex_shader = grid_vertex_shader,
		.fragment_shader = grid_fragment_shader,
	};
	R_GraphicsPipeline* grid_pipeline = R_CreateGraphicsPipeline(&grid_pipeline_info);

	R_CommandBuffer* command_buffer = R_GetCommandBuffer();

  // AlNov: AppLoop
  F32 begin_time = OS_CurrentTimeSeconds();
  while (!app_state.is_window_closed)
  {
    HandleEvents(app_state.frame_arena, &app_state);

		struct
		{
			Mat4 view_matrix;
			Mat4 projection_matrix;
			F32 grid_scale;
		} global_data;
		global_data.view_matrix = MakeLookAtMat4(MakeVec3(0.0f, 1.0f, 1.0f), MakeVec3(0.0f, 0.0f, 0.0f), MakeVec3(0.0f, 1.0f, 0.0f));
		global_data.projection_matrix = MakePerspectiveMat4(
				45.0f, (F32)app_state.window.size.w/(F32)app_state.window.size.h,
				0.0001f, 1000.0f);
		global_data.grid_scale = app_state.grid_scale;

		struct
		{
			Vec4 color;
		} grid_global_fragment_data;
		grid_global_fragment_data.color = MakeVec4(0.4f, 0.4f, 0.4f, 0.8f);

		U64 grid_global_data_offset = R_PushBuffer(data_buffer, (U8*)&global_data, sizeof(global_data));
		U64 grid_global_fragment_data_offset = R_PushBuffer(data_buffer, (U8*)&grid_global_fragment_data, sizeof(grid_global_fragment_data));

		// Draw
		R_TextureTest* swapchain_texture = R_AcquireSwapchainTexture(command_buffer);
		R_BeginCommandBuffer(command_buffer);
		{
			R_ResetBuffer(data_buffer);

			R_ColorAttachment color_attachment = {
				.texture = swapchain_texture,
				.load_operation = R_ATTACHMENT_LOAD_OPERATION_CLEAR,
				.store_operation = R_ATTACHMENT_STORE_OPERATION_STORE,
				.clear_color = MakeVec4(0.0f, 0.07f, 0.12f, 1.0f),
			};
			R_BeginRenderPass(command_buffer, &color_attachment);
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

				R_BindGraphicsPipeline(command_buffer, grid_pipeline);
				R_BindGlobalVertexUniformData(command_buffer, data_buffer, grid_global_data_offset, sizeof(global_data));
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
        // @TODO Window recreated multiple time.
        // I guess, resize event is triggered multiple time. It should be handled only once, after last resizing
				if ((state->window.size.w != event->window_size.w) || (state->window.size.h != event->window_size.h))
				{
					LOG_DEBUG("Old Window Size: %d\t%d\n", state->window.size.w, state->window.size.h);
					state->window.size = event->window_size;
					LOG_DEBUG("New Window Size: %d\t%d\n", state->window.size.w, state->window.size.h);
					R_VK_HandleResize(&state->window);
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

      case OS_EVENT_TYPE_KEYBOARD:
      {
        if (event->key == OS_KEY_ARROW_UP)
        {
          {
						app_state.grid_scale += 1.0f;
          }
        }
        if (event->key == OS_KEY_ARROW_DOWN)
        {
          {
						app_state.grid_scale -= 1.0f;
          }
        }
        if (event->key == OS_KEY_ARROW_RIGHT)
        {
          {
          }
        }
        if (event->key == OS_KEY_ARROW_LEFT)
        {
          {
          }
        }
      }
      
      default: break;
    }
  }
}
