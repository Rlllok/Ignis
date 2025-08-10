#include "base/base_include.h"
#include "base/base_math.h"
#include "os/os_include.h"
#include "render/r_include.h"
#include "assets/mesh.h"
#include "assets/ast_font.h"

#include "base/base_include.cpp"
#include "os/os_include.cpp"
#include "render/r_include.cpp"
#include "assets/mesh.cpp"
#include "assets/ast_font.cpp"

struct AppState
{
  Arena* arena;
  Arena* frame_arena;
  OS_Window window;

  F32 delta_time;

  B32 is_window_closed;

  Vec2f last_mouse_position;
} app_state;

func void HandleEvents(Arena* arena, AppState* state);

I32 main()
{
  // https://docs.vulkan.org/spec/latest/chapters/cmdbuffers.html#VUID-vkResetCommandBuffer-commandBuffer-00046
  app_state = {};
  app_state.arena = AllocateArena(Megabytes(64));
  app_state.frame_arena = AllocateArena(Megabytes(8));
  app_state.is_window_closed = false;

  F32 new_variable = 0;
 
  OS_Init(Megabytes(32));
  OS_CreateWindow("Vulkan Triangle", MakeVec2u(1270, 720), &app_state.window);
  OS_ShowWindow(&app_state.window);

  R_Init(R_RENDERER_TYPE_VK, &app_state.window);

	struct Vertex
	{
		Vec3f position;
	};

	Vertex vertecies[3] = {
		{.position = MakeVec3f(0.0f, 0.5f, 0.0f)},
		{.position = MakeVec3f(0.5f, 0.05f, 0.0f)},
		{.position = MakeVec3f(-0.5f, 0.5f, 0.0f)},
	};

	R_Buffer* vertex_buffer = R_CreateBuffer(Kilobytes(4), R_BUFFER_USAGE_FLAG_VERTEX, R_BUFFER_PROPERTY_FLAG_HOST_COHERENT);
	U64 triangle_vertex_data_offset = R_PushBuffer(vertex_buffer, (U8*)vertecies, sizeof(vertecies[0])*3);

	R_Shader vertex_shader = R_CreateShader(app_state.arena, Str8FromC("data/shaders/triangle.vs.glsl"), R_SHADER_TYPE_VERTEX);
	R_Shader fragment_shader = R_CreateShader(app_state.arena, Str8FromC("data/shaders/triangle.fs.glsl"), R_SHADER_TYPE_FRAGMENT);

	R_VertexAttribute triangle_vertex_attribute_position = {
		.location = 0,
		.format = R_VERTEX_ATTRIBUTE_FORMAT_VEC3F,
		.offset = offsetof(Vertex, position),
	};

	R_GraphicsPipelineCreateInfo triangle_pipeline_info = {
		.vertex_shader = vertex_shader,
		.fragment_shader = fragment_shader,
		.vertex_attributes_count = 1,
		.vertex_attributes = &triangle_vertex_attribute_position,
	};
	R_GraphicsPipeline* triangle_pipeline = R_CreateGraphicsPipeline(&triangle_pipeline_info);

	R_CommandBuffer* command_buffer = R_GetCommandBuffer();

  // AlNov: AppLoop
  F32 begin_time = OS_CurrentTimeSeconds();
  while (!app_state.is_window_closed)
  {
    HandleEvents(app_state.frame_arena, &app_state);

		R_TextureTest* swapchain_texture = R_AcquireSwapchainTexture(command_buffer);
		R_BeginCommandBuffer(command_buffer);
		{
			R_ColorAttachment color_attachment = {
				.texture = swapchain_texture,
				.load_operation = R_ATTACHMENT_LOAD_OPERATION_CLEAR,
				.store_operation = R_ATTACHMENT_STORE_OPERATION_STORE,
				.clear_color = MakeVec4f(1.0f, 0.0f, 0.0f, 1.0f),
			};
			R_BeginRenderPass(command_buffer, &color_attachment);
			{
				R_BindGraphicsPipeline(command_buffer, triangle_pipeline);
				R_BindVertexBuffer(command_buffer, vertex_buffer, triangle_vertex_data_offset);
				R_DrawPrimitives(command_buffer, 3, 1, 0, 0);
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
        state->window.size = event->window_size;
        // LOG_INFO("New window size: %d w %d h\n\n", state->window.size.x, state->window.size.y);
        // Renderer.HandleResize(&state->window);
      } break;

      case OS_EVENT_TYPE_MOUSE_MOVE:
      {
        LOG_INFO("MousePosition: %.3f, %.3f\n", event->mouse_position.x, event->mouse_position.y);
        LOG_INFO("Virtual Cursor: %.3f, %.3f\n", state->window.virtual_cursor_position.x, state->window.virtual_cursor_position.y);
        Vec2f d_position = state->window.virtual_cursor_position - state->last_mouse_position;
        Vec2f mouse_direction = NormalizeVec2f(d_position);

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
