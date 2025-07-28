#include "base/base_include.h"
#include "base/base_math.h"
#include "os/os_include.h"
#include "render/r_include.h"
#include "draw/d_include.h"
#include "assets/mesh.h"
#include "assets/ast_font.h"

#include "base/base_include.cpp"
#include "os/os_include.cpp"
#include "render/r_include.cpp"
#include "draw/d_include.cpp"
#include "assets/mesh.cpp"
#include "assets/ast_font.cpp"

struct Camera
{
  Vec3f position;
  Vec3f front;
  Vec3f up;
  Vec3f right;
  F32 speed;
  F32 yaw;
  F32 pitch;
};

struct AppState
{
  Arena* arena;
  Arena* frame_arena;
  OS_Window window;

  Camera camera;
  F32 delta_time;

  B32 is_window_closed;

  Vec2f last_mouse_position;
} app_state;

func void HandleEvents(Arena* arena, AppState* state);
func void DrawGlyph(GlyphData glyph, I32 size, Vec2I position);

I32 main()
{
  // https://docs.vulkan.org/spec/latest/chapters/cmdbuffers.html#VUID-vkResetCommandBuffer-commandBuffer-00046
  app_state = {};
  app_state.arena = AllocateArena(Megabytes(64));
  app_state.frame_arena = AllocateArena(Megabytes(8));
  app_state.is_window_closed = false;
  app_state.camera = {};
  app_state.camera.position = MakeVec3f(0.0f, 0.0f, 4.0f),
  app_state.camera.front = NormalizeVec3f(MakeVec3f(0.0f, 0.0f, -1.0f));
  app_state.camera.up = NormalizeVec3f(MakeVec3f(0.0f, 1.0f, 0.0f));
  app_state.camera.right = NormalizeVec3f(CrossVec3f(app_state.camera.front, app_state.camera.up));
  app_state.camera.speed = 1.0f;

  F32 new_variable = 0;
 
  OS_Init(Megabytes(32));
  OS_CreateWindow("Vulkan Triangle", MakeVec2u(1270, 720), &app_state.window);
  OS_ShowWindow(&app_state.window);

  R_Init(R_RENDERER_TYPE_VULKAN, &app_state.window);
  D_Init(Megabytes(32));

  R_Pipeline bline_pipeline= {};
  {
    R_VertexAttributeFormat attributes[] = {
      R_VERTEX_ATTRIBUTE_FORMAT_VEC2F
    };
    R_PipelineAssignAttributes(&bline_pipeline, attributes, CountArrayElements(attributes));

    R_BindingInfo scene_bindings[] = {
      {R_BINDING_TYPE_UNIFORM_BUFFER, R_SHADER_TYPE_VERTEX},
    };
    R_PipelineAssignSceneBindingLayout(&bline_pipeline, scene_bindings, CountArrayElements(scene_bindings));

    R_H_LoadShader(app_state.arena, "data/shaders/square.vs.glsl",
                   "main", R_SHADER_TYPE_VERTEX,
                   &bline_pipeline.shaders[R_SHADER_TYPE_VERTEX]);
    R_H_LoadShader(app_state.arena, "data/shaders/bline.fs.glsl",
                   "main", R_SHADER_TYPE_FRAGMENT,
                   &bline_pipeline.shaders[R_SHADER_TYPE_FRAGMENT]);
  
    bline_pipeline.is_back_culing_enabled = false;
    bline_pipeline.is_depth_test_enabled = false;
    Renderer.CreatePipeline(&bline_pipeline);
  }

  // TTFData ttf_data = AST_GetTTFData(app_state.arena, Str8FromC("data/fonts/RobotoMono-Regular.ttf"));
  TTFData ttf_data = AST_GetTTFData(app_state.arena, Str8FromC("data/fonts/Delius-Regular.ttf"));
  LOG_INFO("GetGlyphIndex: %i", _AST_TTFGetGlyphIndex(0x0041, ttf_data.format));

  // --AlNov: @TODO Not Working with ttf other than Envy Code R
  GlyphData glyph_A = AST_GetGlyphDataFromTTF(app_state.arena, ttf_data, 0x0041);
  GlyphData glyph_B = AST_GetGlyphDataFromTTF(app_state.arena, ttf_data, 0x0042);
  GlyphData glyph_C = AST_GetGlyphDataFromTTF(app_state.arena, ttf_data, 0x0043);
  GlyphData glyph_V = AST_GetGlyphDataFromTTF(app_state.arena, ttf_data, 0x0056);
  GlyphData glyph_test = AST_GetGlyphDataFromTTF(app_state.arena, ttf_data, 0x0041);

  // AlNov: AppLoop
  F32 begin_time = OS_CurrentTimeSeconds();
  while (!app_state.is_window_closed)
  {
    HandleEvents(app_state.frame_arena, &app_state);

    Renderer.BeginFrame();
    {
      Renderer.BeginRenderPass(R_ATTACHMENT_LOAD_OPERATION_CLEAR, MakeVec4f(0.03f, 0.03f, 0.03f, 1.0f));
      {
          // D_DrawCircle(MakeVec2I(100, 100), 2, ZeroVec2I()); // --AlNov: @BUG @TODO Not Working. Because I not using SceneUniformData (sdf_vs espects it).
          I32 padding = 100;
          I32 size = 100;
          DrawGlyph(glyph_A, size, MakeVec2I(50 + padding*0, 100));
          DrawGlyph(glyph_B, size, MakeVec2I(50 + padding*1, 100));
          DrawGlyph(glyph_C, size, MakeVec2I(50 + padding*2, 100));
          DrawGlyph(glyph_V, size, MakeVec2I(50 + padding*3, 100));
          DrawGlyph(glyph_test, size, MakeVec2I(50 + padding*4, 100));
      }
    }
    Renderer.EndFrame();
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

        state->camera.yaw += mouse_direction.x * 1.0f * state->delta_time;
        // state->camera.pitch += dy * 0.1f;

        Vec3f direction = {};
        direction.x = cos(state->camera.yaw) * cos(state->camera.pitch);
        direction.y = sin(state->camera.pitch);
        direction.z = sin(state->camera.yaw) * cos(state->camera.pitch);
        // state->camera.front = NormalizeVec3f(direction);
        // state->camera.right = NormalizeVec3f(CrossVec3f(state->camera.front, MakeVec3f(0.0f, 1.0f, 0.0f)));
        // state->camera.up = NormalizeVec3f(CrossVec3f(state->camera.right, state->camera.front));
        
        state->last_mouse_position = state->window.virtual_cursor_position;
      } break;

      case OS_EVENT_TYPE_KEYBOARD:
      {
        if (event->key == OS_KEY_ARROW_UP)
        {
          {
            state->camera.position = state->camera.position + state->camera.speed * state->camera.front;
          }
        }
        if (event->key == OS_KEY_ARROW_DOWN)
        {
          {
            state->camera.position = state->camera.position - state->camera.speed * state->camera.front;
          }
        }
        if (event->key == OS_KEY_ARROW_RIGHT)
        {
          {
            state->camera.position = state->camera.position + state->camera.speed * state->camera.right;
          }
        }
        if (event->key == OS_KEY_ARROW_LEFT)
        {
          {
            state->camera.position = state->camera.position - state->camera.speed * state->camera.right;
          }
        }
      }
      
      default: break;
    }
  }
}

func void
DrawGlyph(GlyphData glyph, I32 size, Vec2I position)
{
  F32 scale_factor = size;
  I32 contur_start = 0;
  for (I32 i = 0; i < glyph.num_conturs; i += 1)
  {
    for (I32 j = contur_start; j <= glyph.contur_end_indecies[i] - 2; j += 2)
    {
      Vec2I p0 = Vec2IFromVec(glyph.points[j]*scale_factor);
      p0.y *= -1;
      p0 = p0 + position;
      Vec2I cp = Vec2IFromVec(glyph.points[j+1]*scale_factor);
      cp.y *= -1;
      cp = cp + position;
      Vec2I p1 = Vec2IFromVec(glyph.points[j+2]*scale_factor);
      p1.y *= -1;
      p1 = p1 + position;

      D_DrawBezier(p0, p1, cp, MakeVec3f(1.0f, 1.0f, 1.0f));
    }
    contur_start = glyph.contur_end_indecies[i] + 1;
  }

  #if 0
  contur_start = 0;
  for (I32 i = 0; i < glyph.num_conturs; i += 1)
  {
    for (I32 j = contur_start; j <= glyph.contur_end_indecies[i] - 2; j += 2)
    {
      Vec2I p0 = Vec2IFromVec(glyph.points[j]);
      p0.y *= -1;
      p0 = p0*scale_factor;
      p0 = p0 + position;
      Vec2I cp = Vec2IFromVec(glyph.points[j+1]);
      cp.y *= -1;
      cp = cp*scale_factor;
      cp = cp + position;
      Vec2I p1 = Vec2IFromVec(glyph.points[j+2]);
      p1.y *= -1;
      p1 = p1*scale_factor;
      p1 = p1 + position;

      RectI rectangle = {};
      rectangle.position = p0;
      rectangle.size = {10, 10};
      D_DrawRectangle(&app_state.window, rectangle, MakeVec3f(0.0f, 0.0f, 1.0f), 0);
      rectangle.position = cp;
      rectangle.size = {10, 10};
      D_DrawRectangle(&app_state.window, rectangle, MakeVec3f(1.0f, 0.0f, 0.0f), 0);
      rectangle.position = p1;
      rectangle.size = {10, 10};
      D_DrawRectangle(&app_state.window, rectangle, MakeVec3f(0.0f, 0.0f, 1.0f), 0);
    }
    contur_start = glyph.contur_end_indecies[i] + 1;
  }
  #endif
}
