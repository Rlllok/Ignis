#include "base/base_include.h"
#include "os/os_include.h"
#include "render/r_include.h"

#include "base/base_include.c"
#include "os/os_include.c"
#include "render/r_include.c"

typedef struct AppState AppState;
struct AppState
{
	Arena* arena;
	Arena* frame_arena;
	OS_Window window;
	F32 delta_time;
	B32 is_window_closed;
	Vec2F32 last_mouse_position;
} app_state;

func void HandleEvents(Arena* arena, AppState* state);

I32 main(void)
{
  app_state.arena = AllocateArena(Megabytes(64));
  app_state.frame_arena = AllocateArena(Megabytes(8));
  app_state.is_window_closed = false;

  F32 new_variable = 0;
 
  OS_Init(Megabytes(32));

  OS_CreateWindow(Str8C("Vulkan Triangle"), MakeVec2U32(1270, 720), &app_state.window);
  OS_ShowWindow(&app_state.window);

  R_Init(R_RENDERER_TYPE_VK, &app_state.window);

	typedef struct Vertex Vertex;
	struct Vertex
	{
		Vec3 position;
		Vec4 color;
	};

	Vertex vertecies[] = {
		{.position = MakeVec3(-1.0f, 1.0f, 0.0f), .color = MakeVec4(1.0f, 0.0f, 0.0f, 1.0f)},
		{.position = MakeVec3(1.0f, 1.0f, 0.0f), .color = MakeVec4(0.0f, 1.0f, 0.0f, 1.0f)},
		{.position = MakeVec3(-1.0f, -1.0f, 0.0f), .color = MakeVec4(0.0f, 0.0f, 1.0f, 1.0f)},
		{.position = MakeVec3(1.0f, -1.0f, 0.0f), .color = MakeVec4(0.0f, 0.0f, 1.0f, 1.0f)},
	};
	// @NOTE @TODO RenderDoc doesnt accept second vertex attribute.
	// It can see data, but not name. Maybe, because there is no alignment
	R_VertexAttribute triangle_vertex_attributes[] = {
		{
			.location = 0,
			.format = R_VERTEX_ATTRIBUTE_FORMAT_VEC3F32,
			.offset = offsetof(Vertex, position),
		},
		{
			.location = 1,
			.format = R_VERTEX_ATTRIBUTE_FORMAT_VEC4F32,
			.offset = offsetof(Vertex, color),
		},
	};

	U16 indecies[] = {
		0, 1, 2,
		2, 1, 3
	};

	R_BufferUsageFlags triangle_buffer_usage_flags = R_BUFFER_USAGE_FLAG_VERTEX|R_BUFFER_USAGE_FLAG_INDEX|R_BUFFER_USAGE_FLAG_UNIFORM;
	R_Buffer* triangle_buffer = R_CreateBuffer(Megabytes(4), triangle_buffer_usage_flags, R_BUFFER_PROPERTY_FLAG_HOST_COHERENT);

	R_Shader vertex_shader = R_CreateShader(app_state.arena, Str8C("./data/shaders/triangle.vs.glsl"), R_SHADER_TYPE_VERTEX, 1);
	R_Shader fragment_shader = R_CreateShader(app_state.arena, Str8C("./data/shaders/triangle.fs.glsl"), R_SHADER_TYPE_FRAGMENT, 1);

	R_GraphicsPipelineCreateInfo triangle_pipeline_info = {
		.vertex_shader = vertex_shader,
		.fragment_shader = fragment_shader,
		.vertex_attributes_count = CountArrayElements(triangle_vertex_attributes),
		.vertex_attributes = triangle_vertex_attributes,
	};
	R_GraphicsPipeline* triangle_pipeline = R_CreateGraphicsPipeline(&triangle_pipeline_info);

	R_CommandBuffer* command_buffer = R_GetCommandBuffer();

  // AlNov: AppLoop
  F32 begin_time = OS_CurrentTimeSeconds();
  while (!app_state.is_window_closed)
  {
    HandleEvents(app_state.frame_arena, &app_state);

		typedef struct GlobalData GlobalData;
		struct GlobalData
		{
			Mat4 view_matrix;
			Mat4 scale_matrix;
			Mat4 transpose_matrix;
			Mat4 rotation_matrix;
		};
		typedef struct FragmentGlobalData FragmentGlobalData;
		struct FragmentGlobalData
		{
			Vec3 color;
			U8 padding[4];
		};

#define TRIANGLE_COUNT 1
		GlobalData triangles_data[TRIANGLE_COUNT];
		for (I32 i = 0; i < TRIANGLE_COUNT; i += 1)
		{
			F32 aspect_ratio = (F32)app_state.window.size.w/(F32)app_state.window.size.h;
#if 0
			triangles_data[i].view_matrix = MakePerspectiveMat4(45.0f, 1280.0f/720.0f, 0.001, 100.0f);
			triangles_data[i].scale_matrix = MakeMat4(0.1f);
			triangles_data[i].transpose_matrix = MakeTransposeMat4(ScaleVec3(MakeVec3(1.0f, 1.0f, 1.0f), 0.1f*i));
			triangles_data[i].rotation_matrix = MakeRotationMat4(MakeVec3(0.1f, 1.0f, 0.3f), 0.5f*begin_time);
#endif
			triangles_data[i].view_matrix = MakePerspectiveMat4(80.0f, aspect_ratio, 0.001, 1000.0f);
			triangles_data[i].scale_matrix = MakeMat4(1.0f);
			triangles_data[i].transpose_matrix = MakeTransposeMat4(MakeVec3(.0f, .0f, -5.0f));
			triangles_data[i].rotation_matrix = MakeMat4(1.0f);
		}
		FragmentGlobalData fragment_triangles_data[TRIANGLE_COUNT];
		for (I32 i = 0; i < TRIANGLE_COUNT; i += 1)
		{
			fragment_triangles_data[i].color = MakeVec3(fabs(sin(begin_time*(i+5))), fabs(1.0f - sin(begin_time / (i+1))), fabs(cos(begin_time)));
		}

		// Draw
		R_TextureTest* swapchain_texture = R_AcquireSwapchainTexture(command_buffer);
		R_BeginCommandBuffer(command_buffer);
		{
		R_ResetBuffer(triangle_buffer);
		U64 triangle_vertex_data_offset = R_PushBuffer(triangle_buffer, (U8*)vertecies, sizeof(vertecies[0])*CountArrayElements(vertecies));
		U64 triangle_index_data_offset = R_PushBuffer(triangle_buffer, (U8*)indecies, sizeof(indecies[0])*CountArrayElements(indecies));

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
				R_BindGraphicsPipeline(command_buffer, triangle_pipeline);
				R_BindIndexBuffer(command_buffer, triangle_buffer, triangle_index_data_offset, R_INDEX_SIZE_U16);
				R_BindVertexBuffer(command_buffer, triangle_buffer, triangle_vertex_data_offset);

				for (I32 i = 0; i < TRIANGLE_COUNT; i += 1)
				{
					U64 triangle_global_data_offset = R_PushBuffer(triangle_buffer, (U8*)(triangles_data + i), sizeof(triangles_data[i]));
					U64 triangle_fragment_global_data_offset = R_PushBuffer(triangle_buffer, (U8*)(fragment_triangles_data + i), sizeof(fragment_triangles_data[i]));
					R_BindGlobalVertexUniformData(command_buffer, triangle_buffer, triangle_global_data_offset, sizeof(triangles_data[0]));
					R_BindGlobalFragmentUniformData(command_buffer, triangle_buffer, triangle_fragment_global_data_offset, sizeof(fragment_triangles_data[i]));
					R_DrawIndexedPrimitives(command_buffer, 6, 1, 0, 0, 0);
				}
			}
			R_EndRenderPass(command_buffer, 0);
					//LOG_DEBUG("Old Window Size: %d\t%d\n", app_state.window.size.w, app_state.window.size.h);
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
          }
        }
        if (event->key == OS_KEY_ARROW_DOWN)
        {
          {
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
