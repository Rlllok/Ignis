#include "base/base_include.h"
#include "os/os_include.h"
#include "render/r_include.h"
#include "assets/animation.h"
#include "assets/mesh.h"
#include "ui/ui_include.h"

#include "base/base_include.c"
#include "os/os_include.c"
#include "render/r_include.c"
#include "assets/animation.c"
#include "assets/mesh.c"
#include "ui/ui_include.c"

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"

func void DrawRect(R_CommandBuffer command_buffer, R_Buffer buffer, RectF32 rect, Vec4 border_radius, Vec4 color);
func void IGN_DrawText(R_CommandBuffer command_buffer, R_Buffer buffer, FontBitmap font, Str8 text, U32 font_size, Vec2F32 position, Vec4F32 color);

func void UpdateSkeletonGlobalTransform(Skeleton* skeleton);
func void DrawSkeleton(R_CommandBuffer command_buffer, R_Buffer buffer, Skeleton* skeleton, Vec4F32 color);

// -------------------------------------------------------------------
// Main
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
  Str8 name;

  Transform transform;

  F32 smoothness;

  AST_StaticMesh mesh;
  R_Texture color_texture;
};
Entity EntityDefaultValue = {0};
DefineArray(Entity, EntityArray, EntityDefaultValue) // -- AlNov: @TODO It can be better to set DefaultValue for Array through parameter

func void
CreateEntity(EntityArray* array, Entity entity)
{
  entity.id = array->length + 1;
  EntityArrayAdd(array, entity);
}

func void DrawLine3D(R_CommandBuffer command_buffer, R_Buffer buffer, Vec3F32 start, Vec3F32 end, Vec4F32 color, F32 width);
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

// Command Palette ----------------------------------------------------
typedef struct Command Command;
struct Command
{
  Str8 name;
  void (*callback)(void* data);
  void* data;
};
Command _command_nil = {0};
DefineArray(Command, CommandArray, _command_nil)

typedef struct CommandPalette CommandPalette;
struct CommandPalette
{
  Str8 input;
  CommandArray commands;

  RectF32 rectangle;
  Vec4F32 background_color;
  Vec4F32 border_radius;
  B32 activated;
};

func void ToggleCommandPalette(CommandPalette* command_palette) {command_palette->activated = !command_palette->activated;}
func void
ToggleCommandPaletteCommandCallback(void* command_palette_ptr)
{
  ToggleCommandPalette((CommandPalette*)(command_palette_ptr));
}

func void DrawCommandPalette(R_CommandBuffer command_buffer, R_Buffer buffer, CommandPalette* command_palette, U64 current_timestamp);

// App ---------------------------------------------------------------
typedef struct AppState AppState;
struct AppState
{
  Arena* arena;
  Arena* frame_arena;
  OS_Window window;
  F32 delta_time_sec;
  B32 is_window_closed;
  B32 mouse_inside;
  Vec2F32 last_mouse_position;

  R_GraphicsPipeline grid_pipeline;
  R_GraphicsPipeline line_3d_pipeline;
  R_GraphicsPipeline square_pipeline;
  R_GraphicsPipeline mesh_pipeline;
  R_GraphicsPipeline font_pipeline;
  R_GraphicsPipeline joint_pipeline;

  R_TextureSampler texture_sampler;
  R_Texture default_color_texture;
  R_Texture mesh_color_texture;
  R_Texture mesh_normal_texture;

  FontBitmap font;

  R_Texture depth_texture; // -AlNov: @TODO should it be created for R_VK_Swapchain?
  R_Texture test_texture;

  Camera camera;

  U32 hover_entity_id;

  F32 grid_scale;

  EntityArray entities;
  Entity* selected_entity;

  B32 to_render;
  B32 draw_ui;

  CommandPalette command_palette;

  // --AlNov. 12 December 2025: @TODO @TEST
  I32 current_animation_index;
} app_state;

func void HandleEvents(Arena* arena, AppState* state);

func R_Texture CreateLoadTexture(R_Buffer buffer, Str8 path, R_TextureFormat format)
{
  R_Texture result = {0};

  I32 tex_width = 0;
  I32 tex_height = 0;
  I32 tex_channels = 0;
  U8* tex_pixels = stbi_load(CFromStr8(path), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);

  if (!tex_pixels)
  {
    LOG_ERROR("Cannot load texture %s\n", CFromStr8(path));
  }
  I32 texture_size = tex_width * tex_height * 4;

   result = R_CreateTexture(
    &(R_TextureCreateInfo){
      .type = R_TEXTURE_TYPE_2D,
      .format = format,
      .usage_flags = R_TEXTURE_USAGE_FLAG_SAMPLED | R_TEXTURE_USAGE_FLAG_TRANSFER_DST,
      .width = tex_width,
      .height = tex_height,
      .depth = 1,
      .num_levels = 1,
    }
  );

  U64 texture_offset = R_PushBuffer(buffer, tex_pixels, texture_size);
  R_CopyBufferToTexture(0, buffer, texture_offset, texture_size, result);

  return result;
}

I32 main(void)
{
  app_state.arena = AllocateArena(Megabytes(64));
  app_state.frame_arena = AllocateArena(Megabytes(8));
  app_state.is_window_closed = 0;
  app_state.grid_scale = 2000.0f;
  app_state.camera.position = MakeVec3(1.0f, 5.0f, 10.0f);
  app_state.camera.front = MakeVec3(1.0f, 0.0f, -1.0f);
  app_state.camera.right = MakeVec3(1.0f, 0.0f, 1.0f);
  app_state.camera.up = MakeVec3(0.0f, 1.0f, 0.0f);
  app_state.camera.yaw = -90.0f;
  app_state.camera.pitch = -30.0f;
  app_state.entities = EntityArrayAllocate(app_state.arena, 128);
  app_state.selected_entity = &EntityDefaultValue;
  app_state.draw_ui = 1;
  app_state.to_render = 1;
  app_state.command_palette = (CommandPalette){
    .input = Str8C("Input string test"),
    .rectangle = (RectF32){
      .position = MakeVec2F32(0.0f, 0.0f),
      .size = MakeVec2F32(400.0f, 400.0f),
    },
    .background_color = MakeVec4F32(0.0f, 0.0f, 0.0f, 0.8f),
    .activated = 0,
  };
  app_state.command_palette.commands = CommandArrayAllocate(app_state.arena, 3);
  CommandArrayAdd(
    &app_state.command_palette.commands,
    (Command){
      .name = Str8C("Toggle Command Palette"),
      .callback = ToggleCommandPaletteCommandCallback,
      .data = &app_state.command_palette,
    }
  );

  app_state.current_animation_index = 0;

  ui_context.elements = UI_ElementArrayAllocate(app_state.arena, 1024);
  ui_context.draw_commands = UI_DrawCommandArrayAllocate(app_state.arena, 1024);

  OS_Init(Megabytes(32));

  OS_CreateWindow(Str8C("Ignis"), MakeVec2U32(1270, 720), &app_state.window);
  OS_ShowWindow(&app_state.window);

  R_Init(R_RENDERER_TYPE_VK, &app_state.window);

  R_CommandBuffer command_buffer = R_GetCommandBuffer();

  R_BufferUsageFlags triangle_buffer_usage_flags = R_BUFFER_USAGE_FLAG_VERTEX|R_BUFFER_USAGE_FLAG_INDEX|R_BUFFER_USAGE_FLAG_UNIFORM;
  R_Buffer data_buffer = R_CreateBuffer(Megabytes(64), triangle_buffer_usage_flags, R_BUFFER_PROPERTY_FLAG_HOST_COHERENT);
  R_Buffer transfer_buffer = R_CreateBuffer(Megabytes(128), R_BUFFER_USAGE_FLAG_TRANSFER, R_BUFFER_PROPERTY_FLAG_HOST_COHERENT);

  app_state.texture_sampler = R_CreateTextureSampler(
    &(R_TextureSamplerCreateInfo){
      .mag_filter = R_FILTER_TYPE_LINEAR,
      .min_filter = R_FILTER_TYPE_LINEAR,
      .address_mode_u = R_SAMPLER_ADDRESS_MODE_REPEAT,
      .address_mode_v = R_SAMPLER_ADDRESS_MODE_REPEAT,
      .address_mode_w = R_SAMPLER_ADDRESS_MODE_REPEAT,
      .mipmap_mode = R_SAMPLER_MIPMAP_MODE_LINEAR,
    }
  );

  app_state.test_texture = R_CreateTexture(
    &(R_TextureCreateInfo){
      .type = R_TEXTURE_TYPE_2D,
      .format = R_TEXTURE_FORMAT_R16_UINT,
      .usage_flags = R_TEXTURE_USAGE_FLAG_COLOR_ATTACHMENT | R_TEXTURE_USAGE_FLAG_TRANSFER_SRC,
      .width = app_state.window.size.w,
      .height = app_state.window.size.h,
      .depth = 1,
      .num_levels = 1
    }
  );

  app_state.depth_texture = R_CreateTexture(
    &(R_TextureCreateInfo){
      .type = R_TEXTURE_TYPE_2D,
      .format = R_TEXTURE_FORMAT_D16_UNORM,
      .usage_flags = R_TEXTURE_USAGE_FLAG_DEPTH_STENCIL_ATTACHMENT,
      .width = app_state.window.size.w,
      .height = app_state.window.size.h,
      .depth = 1,
      .num_levels = 1,
    }
  );

  // Mesh Texture
  {
    app_state.default_color_texture = CreateLoadTexture(transfer_buffer, Str8C("./data/uv_checker.png"), R_TEXTURE_FORMAT_R8G8B8A8_SRGB);
    app_state.mesh_color_texture = CreateLoadTexture(transfer_buffer, Str8C("./data/sphere_gltf/RockyColor.png"), R_TEXTURE_FORMAT_R8G8B8A8_SRGB);
    app_state.mesh_normal_texture = CreateLoadTexture(transfer_buffer, Str8C("./data/sphere_gltf/RockyNormal.png"), R_TEXTURE_FORMAT_R8G8B8A8_UNORM);
  }

  // Font Texture
  {
    Str8 texture_path = Str8C("./data/fonts/RobotoMonoBitmap.png");
    I32 tex_width = 0;
    I32 tex_height = 0;
    I32 tex_channels = 0;
    U8* tex_pixels = stbi_load(CFromStr8(texture_path), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);

    if (!tex_pixels)
    {
      LOG_ERROR("Cannot load texture %s\n", CFromStr8(texture_path));
    }
    I32 texture_size = tex_width * tex_height * 4;

    app_state.font.bitmap = R_CreateTexture(
      &(R_TextureCreateInfo){
        .type = R_TEXTURE_TYPE_2D,
        .format = R_TEXTURE_FORMAT_R8G8B8A8_SRGB,
        .usage_flags = R_TEXTURE_USAGE_FLAG_SAMPLED | R_TEXTURE_USAGE_FLAG_TRANSFER_DST,
        .width = tex_width,
        .height = tex_height,
        .depth = 1,
        .num_levels = 1,
      }
    );
    app_state.font.bitmap_size = MakeVec2U32(tex_width, tex_height);
    app_state.font.glyph_size = MakeVec2U32(30, 30);
    app_state.font.glyphs_per_row = 19;
    U64 font_texture_offset = R_PushBuffer(data_buffer, tex_pixels, texture_size);
    R_CopyBufferToTexture(0, data_buffer, font_texture_offset, texture_size, app_state.font.bitmap);
  }

  // --AlNov: Word Grid
  {
    R_Shader grid_vertex_shader = R_CreateShader(
      app_state.arena,
      &(R_ShaderCreateInfo){
        .file_name = Str8C("./data/shaders/grid.vs.glsl"),
        .type = R_SHADER_TYPE_VERTEX,
        .global_uniforms_count = 1,
        .instance_uniforms_count = 1,
      }
    );

    R_Shader grid_fragment_shader = R_CreateShader(
      app_state.arena,
      &(R_ShaderCreateInfo){
        .file_name = Str8C("./data/shaders/grid.fs.glsl"),
        .type = R_SHADER_TYPE_FRAGMENT,
        .global_uniforms_count = 1,
        .instance_uniforms_count = 0,
      }
    );

    app_state.grid_pipeline = R_CreateGraphicsPipeline(
      &(R_GraphicsPipelineCreateInfo){
        .vertex_shader = grid_vertex_shader,
        .fragment_shader = grid_fragment_shader,
        .color_targets_count = 1,
        .color_target_infos = &(R_GraphicsPipelineColorTargetInfo){
          .format = R_GetSwapchainTextureFormat(),
          .blend_enable = 1,
        },
        .depth_stencil_state = {
          .depth_test_enable = 1,
          .depth_write_enable = 0,
          .depth_compare_operation = R_COMPARE_OPERATION_GREATER,
          .depth_target_format = R_GetTextureFormat(app_state.depth_texture),
        },
      }
    );
  }

  // 3D Line Pipeline
  {
    R_Shader line_vertex_shader = R_CreateShader(
      app_state.arena,
      &(R_ShaderCreateInfo){
        .file_name = Str8C("./data/shaders/line3d.vs.glsl"),
        .type = R_SHADER_TYPE_VERTEX,
        .global_uniforms_count = 1,
        .instance_uniforms_count = 1,
      }
    );

    R_Shader line_fragment_shader = R_CreateShader(
      app_state.arena,
      &(R_ShaderCreateInfo){
        .file_name = Str8C("./data/shaders/line3d.fs.glsl"),
        .type = R_SHADER_TYPE_FRAGMENT,
      }
    );

    app_state.line_3d_pipeline = R_CreateGraphicsPipeline(
      &(R_GraphicsPipelineCreateInfo){
        .vertex_shader = line_vertex_shader,
        .fragment_shader = line_fragment_shader,
        .color_targets_count = 1,
        .color_target_infos = &(R_GraphicsPipelineColorTargetInfo){
          .format = R_GetSwapchainTextureFormat(),
        },
        .depth_stencil_state = {
          .depth_test_enable = 0,
          .depth_write_enable = 0,
          .depth_compare_operation = R_COMPARE_OPERATION_GREATER,
          .depth_target_format = R_GetTextureFormat(app_state.depth_texture),
        },
      }
    );
  }

  // Font Pipeline
  {
    R_Shader font_vertex_shader = R_CreateShader(
      app_state.arena,
      &(R_ShaderCreateInfo){
        .file_name = Str8C("./data/shaders/font.vs.glsl"),
        .type = R_SHADER_TYPE_VERTEX,
        .global_uniforms_count = 1,
      }
    );

    R_Shader font_fragment_shader = R_CreateShader(
      app_state.arena,
      &(R_ShaderCreateInfo){
        .file_name = Str8C("./data/shaders/font.fs.glsl"),
        .type = R_SHADER_TYPE_FRAGMENT,
        .global_samplers_count = 1,
      }
    );

    R_VertexAttribute font_vertex_attributes[] = {
      {
        .location = 0,
        .format = R_VERTEX_ATTRIBUTE_FORMAT_VEC2F32,
        .offset = offsetof(TextVertex, position),
      },
      {
        .location = 1,
        .format = R_VERTEX_ATTRIBUTE_FORMAT_VEC2F32,
        .offset = offsetof(TextVertex, uv),
      },
    };
    R_GraphicsPipelineColorTargetInfo font_pipeline_color_target = {
      .format = R_GetSwapchainTextureFormat(),
      .blend_enable = 1,
    };
    R_GraphicsPipelineCreateInfo font_pipeline_info = {
      .vertex_shader = font_vertex_shader,
      .fragment_shader = font_fragment_shader,
      .vertex_attributes_count = CountArrayElements(font_vertex_attributes),
      .vertex_attributes = font_vertex_attributes,
      .color_targets_count = 1,
      .color_target_infos = &font_pipeline_color_target,
      .depth_stencil_state = {
        .depth_test_enable = 0,
      },
    };
    app_state.font_pipeline = R_CreateGraphicsPipeline(&font_pipeline_info);
  }

  // Square Pipeline
  {
    R_Shader square_vertex_shader = R_CreateShader(
      app_state.arena,
      &(R_ShaderCreateInfo){
        .file_name = Str8C("./data/shaders/square.vs.glsl"),
        .type = R_SHADER_TYPE_VERTEX,
        .global_uniforms_count = 1,
        .instance_uniforms_count = 1,
      }
    );
    R_Shader square_fragment_shader = R_CreateShader(
      app_state.arena,
      &(R_ShaderCreateInfo){
        .file_name = Str8C("./data/shaders/square.fs.glsl"),
        .type = R_SHADER_TYPE_FRAGMENT,
        .global_uniforms_count = 0,
        .instance_uniforms_count = 1,
      }
    );

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


  // AST_StaticMesh dummy_mesh = AST_LoadStaticMeshFromGLTF(app_state.arena, Str8C("data/gltf_test/SimpleAnimation/SimpleAnimation.gltf"));
  // AST_StaticMesh dummy_mesh = AST_LoadStaticMeshFromGLTF(app_state.arena, Str8C("data/Dummy/Dummy.gltf"));
  AST_StaticMesh dummy_mesh = AST_LoadStaticMeshFromGLTF(app_state.arena, Str8C("data/gltf_test/SimpleBoneAnimation/SimpleBoneAnimation.gltf"));
  UpdateSkeletonGlobalTransform(&dummy_mesh.skeleton);
  CreateEntity(
    &app_state.entities,
    (Entity){
      .name = Str8C("Dummy"),
      .transform = (Transform){
        .translation = MakeVec3(0.0f, 0.0f, 0.0f),
        .rotation = IdentityQuaternion(),
        .scale = MakeVec3F32(1.0f, 1.0f, 1.0f),
      },
      .mesh = dummy_mesh,
      .color_texture = app_state.default_color_texture,
    }
  );

  // Mesh Pipeline
  {
    R_Shader mesh_vertex_shader = R_CreateShader(
      app_state.arena,
      &(R_ShaderCreateInfo){
        .file_name = Str8C("./data/shaders/mesh.vs.glsl"),
        .type = R_SHADER_TYPE_VERTEX,
        .global_uniforms_count = 1,
        .instance_uniforms_count = 1,
      }
    );
    R_Shader mesh_fragment_shader = R_CreateShader(
      app_state.arena,
      &(R_ShaderCreateInfo){
        .file_name = Str8C("./data/shaders/mesh.fs.glsl"),
        .type = R_SHADER_TYPE_FRAGMENT,
        .global_uniforms_count = 0,
        .instance_uniforms_count = 1,
        .instance_samplers_count = 2,
      }
    );

    R_VertexAttribute mesh_vertex_attributes[] = {
      {
        .location = 0,
        .format = R_VERTEX_ATTRIBUTE_FORMAT_VEC3F32,
        .offset = offsetof(AST_Vertex, position),
      },
      {
        .location = 1,
        .format = R_VERTEX_ATTRIBUTE_FORMAT_VEC3F32,
        .offset = offsetof(AST_Vertex, normal),
      },
      {
        .location = 2,
        .format = R_VERTEX_ATTRIBUTE_FORMAT_VEC3F32,
        .offset = offsetof(AST_Vertex, tangent),
      },
      {
        .location = 3,
        .format = R_VERTEX_ATTRIBUTE_FORMAT_VEC2F32,
        .offset = offsetof(AST_Vertex, uv),
      },
      {
        .location = 4,
        .format = R_VERTEX_ATTRIBUTE_FORMAT_VEC4I32,
        .offset = offsetof(AST_Vertex, joint_ids),
      },
      {
        .location = 5,
        .format = R_VERTEX_ATTRIBUTE_FORMAT_VEC4F32,
        .offset = offsetof(AST_Vertex, joint_weights),
      },
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

  // AlNov: -- AppLoop
  U64 begin_time_ms = OS_GetTimeTicks();
  U16 test_texture_values_offset = 0;
  while (!app_state.is_window_closed)
  {
    HandleEvents(app_state.frame_arena, &app_state);

    if (OS_IsMousePressed(OS_MouseButton_Left))
    {
      LOG_INFO("Mouse Pressed: %.3fx, %.3fy\n", app_state.last_mouse_position.x, app_state.last_mouse_position.y);
    }

    // Update World
    {
    }

    // UI
    if (app_state.draw_ui)
    {
      DeferBlock(UI_BeginFrame(app_state.last_mouse_position), UI_EndFrame())
      {
        Vec4F32 background_color = RGBAFromHex(0x1D1A26FF);

        UI_RectangleDescription default_rectangle = {
          .color = RGBAFromHex(0x1D1A26FF),
          .radius = {0.0f, 0.0f, 0.0f, 0.0f},
        };

        UI_TextDescription default_text = {
          .str = Str8C("DefaultText"),
          .font = app_state.font,
          .color = RGBAFromHex(0xFFFFFFFF),
          .font_size = 24.0f,
        };

        UI_ElementDescription button_description = {
          .flags = UI_ElementFlag_DrawBackground | UI_ElementFlag_Hover | UI_ElementFlag_Clickable,
          .layout = {
            .width = UI_ParentPercentSize(1.0f),
            .height = UI_FixedSize(40.0f),
          },
          .rectangle = {
            .color = RGBAFromHex(0xBBAA22FF),
            .radius = {4.0f, 4.0f, 4.0f, 4.0f},
          },
        };

        UI_OpenElement({
          .flags = UI_ElementFlag_DrawBackground,
          .layout = {
            .width = UI_FixedSize(250.0f),
            .height = UI_FixedSize(app_state.window.size.y),
            .direction = UI_LayoutDirection_TopToBottom,
           },
          .rectangle = default_rectangle,
        })
        {
          Entity* entity = EntityArrayGetPointer(&app_state.entities, 0);
          Skeleton* skeleton = &entity->mesh.skeleton;

          for (I32 i = 0; i < entity->mesh.skeletal_animations.length; i += 1)
          {
            UI_OpenElement(button_description)
            {
              SkeletalAnimation* skeletal_animation = SkeletalAnimationArrayGetPointer(&entity->mesh.skeletal_animations, i);
              UI_Text(skeletal_animation->name, default_text);
              if (UI_IsClicked())
              {
                for (I32 j = 0; j < skeleton->joints.length; j += 1)
                {
                  Animation* animation = AnimationArrayGetPointer(&skeletal_animation->bone_animations, j);
                  if (animation !=  &_animation_nil)
                  {
                    BeginAnimation(animation, OS_GetTimeTicks());
                  }
                }
                app_state.current_animation_index = i;
              }
            }
          }
        }
      }
    }

    if (app_state.to_render)
    {
      R_ResetBuffer(data_buffer);
      R_ResetBuffer(transfer_buffer);

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
            Entity* entity = EntityArrayGetPointer(&app_state.entities, i);
            DrawEntity(command_buffer, data_buffer, entity);
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
          for (I32 i = 0; i < app_state.entities.length; i += 1)
          {
            Entity* entity = EntityArrayGetPointer(&app_state.entities, i);
            Skeleton* skeleton = &entity->mesh.skeleton;

            for (I32 j = 0; j < skeleton->joints.length; j += 1)
            {
              Joint* joint = JointArrayGetPointer(&skeleton->joints, j);
              SkeletalAnimation* skeletal_animation = SkeletalAnimationArrayGetPointer(&entity->mesh.skeletal_animations, app_state.current_animation_index);
              Animation* animation = AnimationArrayGetPointer(&skeletal_animation->bone_animations, j);
              if (animation !=  &_animation_nil)
              {
                joint->local_transform = AnimateTransform(*animation, OS_GetTimeTicks());
              }
              else
              {
                LOG_DEBUG("Enitity %s. No animation for bone number (%i/%i)\n", CFromStr8(entity->name), j, skeleton->joints.length);
              }
            }

            UpdateSkeletonGlobalTransform(skeleton);
            DrawSkeleton(command_buffer, data_buffer, skeleton, MakeVec4F32(0.9f, 0.7f, 0.5f, 1.0f));
          }

          struct
          {
            Mat4 view_matrix;
            Mat4 projection_matrix;
          } grid_global_vertex_data;
          grid_global_vertex_data.view_matrix = MakeLookAtMat4(
            app_state.camera.position,
            AddVec3(app_state.camera.position, app_state.camera.front),
            app_state.camera.up
          );
          grid_global_vertex_data.projection_matrix = MakePerspectiveMat4(
            45.0f, (F32)app_state.window.size.w/(F32)app_state.window.size.h,
            0.1f, 100.0f
          );
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
          grid_global_fragment_data.color = RGBAFromHex(0x95B8D1AA);

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
        }
        R_EndRenderPass(command_buffer, 0);

        R_ColorTarget font_pass_color_target = {
          .texture = swapchain_texture,
          .load_operation = R_ATTACHMENT_LOAD_OPERATION_LOAD,
          .store_operation = R_ATTACHMENT_STORE_OPERATION_STORE,
        };
        R_BeginRenderPass(command_buffer, 1, &font_pass_color_target, 0);
        {
          char frame_time_cstring[128] = {0};
          sprintf(frame_time_cstring, "%.3f", app_state.delta_time_sec*1000.0f);

          for (I32 i = 0; i < ui_context.draw_commands.length; i += 1)
          {
            UI_DrawCommand* draw_command = UI_DrawCommandArrayGetPointer(&ui_context.draw_commands, i);
            switch (draw_command->type)
            {
              default: {} break;

              case UI_DrawCommandType_Rectangle:
              {
                DrawRect(command_buffer, data_buffer, draw_command->rectangle.bound, draw_command->rectangle.radius, draw_command->rectangle.color);
              } break;

              case UI_DrawCommandType_Text:
              {
                IGN_DrawText(command_buffer, data_buffer, draw_command->text.font, draw_command->text.content, draw_command->text.font_size, draw_command->text.position, draw_command->text.color);
              }
            }
          }

          if (app_state.mouse_inside)
          {
            RectF32 cursor = {
              .position = app_state.last_mouse_position,
              .size = MakeVec2(20.0f, 20.0f),
            };
            DrawRect(command_buffer, data_buffer, cursor, MakeVec4(0.0f, 10.0f, 10.0f, 10.0f), MakeVec4(0.8f, 0.8f, 0.8f, 1.0f));
          }

          if (app_state.command_palette.activated)
          {
            // DrawCommandPalette(command_buffer, data_buffer, &app_state.command_palette, OS_GetTimeTicks());
          }
        }
        R_EndRenderPass(command_buffer, 0);

        test_texture_values_offset = R_CopyTextureToBuffer(command_buffer, app_state.test_texture, transfer_buffer);
      }
      R_SubmitCommandBuffer(command_buffer);

      R_PresentTexture(command_buffer, swapchain_texture);

      if (app_state.last_mouse_position.x >= 0.0f && app_state.last_mouse_position.y >= 0)
      {
        U64 pixel_offset = sizeof(U16)*(((I32)app_state.window.size.x*(I32)app_state.last_mouse_position.y) + (I32)app_state.last_mouse_position.x);
        R_VK_BufferGetData(transfer_buffer, test_texture_values_offset + pixel_offset, &app_state.hover_entity_id, sizeof(U16));

        if (app_state.hover_entity_id != 0)
        {
          if (OS_IsMousePressed(OS_MouseButton_Left))
          {
            app_state.selected_entity = EntityArrayGetPointer(&app_state.entities, app_state.hover_entity_id - 1);
          }
        }
      }
    }
    // LOG_DEBUG("MOUSE: %.1fx, %.1fy\tOFFSET: %d, PIXEL VALUE: %d\n", app_state.last_mouse_position.x, app_state.last_mouse_position.y, pixel_offset, app_state.hover_entity_id);

    ResetArena(app_state.frame_arena);

    U64 end_tick_ms = OS_GetTimeTicks();

    app_state.delta_time_sec = (end_tick_ms - begin_time_ms)/1000.0f;
    begin_time_ms = end_tick_ms;
  }

  R_VK_DestroyBuffer(data_buffer);
  R_Shutdown(); // -AlNov: @BUG Driver Timeout (Vulkan Shutdown is not implemented)

  return 0;
}

func void
DrawEntity(R_CommandBuffer command_buffer, R_Buffer buffer, Entity* entity)
{
  for (AST_GeometryListNode* geometry_node = entity->mesh.geometry_list.first; geometry_node; geometry_node = geometry_node->next)
  {
    AST_Geometry* geometry = &geometry_node->data;

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

    U64 mesh_vertex_data_offset = R_PushBuffer(buffer, (U8*)geometry->vertecies, geometry->vertecies_count*sizeof(AST_Vertex));
    U64 mesh_index_data_offset = R_PushBuffer(buffer, geometry->index_data, geometry->index_size*geometry->index_count);

    struct
    {
      Mat4 instance_matrix;
      Mat4 bone_transform[64];
    } mesh_instance_vertex_data;
    mesh_instance_vertex_data.instance_matrix = Mat4F32FromTransform(entity->transform);
    for (I32 i = 0; i < entity->mesh.skeleton.joints.length; i += 1)
    {
      Joint joint = JointArrayGet(&entity->mesh.skeleton.joints, i);
      mesh_instance_vertex_data.bone_transform[i] = Mat4F32FromTransform(joint.global_transform);
    }

    U64 mesh_instance_vertex_data_offset = R_PushBuffer(buffer, (U8*)&mesh_instance_vertex_data, sizeof(mesh_instance_vertex_data));

    struct
    {
      Vec3 camera_position;
      F32 camera_position_padding;
      Vec3 ambient_color;
      F32 smoothness;
      Vec3 light_direction;
      F32 entity_id;
    } mesh_instance_fragment_data;
    mesh_instance_fragment_data.camera_position = app_state.camera.position;
    mesh_instance_fragment_data.ambient_color = MakeVec3(0.1f, 0.1f, 0.1f);
    mesh_instance_fragment_data.smoothness = entity->smoothness;
    mesh_instance_fragment_data.light_direction = NormalizeVec3(MakeVec3(1.0f, -1.0f, -1.0f));
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
    R_SamplerBindingInfo mesh_fragment_shader_instance_samplers[2] = {
      {
        .sampler = app_state.texture_sampler,
        .texture = entity->color_texture,
      },
      {
        .sampler = app_state.texture_sampler,
        .texture = app_state.mesh_normal_texture,
      }
    };

    R_BindGraphicsPipeline(command_buffer, app_state.mesh_pipeline);
    R_BindGlobalVertexShaderData(command_buffer, 1, &mesh_vertex_shader_global_uniform, 0, 0);
    R_BindInstanceVertexShaderData(command_buffer, 1, &mesh_vertex_shader_instance_uniform, 0, 0);
    R_BindInstanceFragmentShaderData(command_buffer, 1, &mesh_fragment_shader_instance_uniform, 2, mesh_fragment_shader_instance_samplers);
    R_BindVertexBuffer(command_buffer, buffer, mesh_vertex_data_offset);
    R_BindIndexBuffer(command_buffer, buffer, mesh_index_data_offset, R_INDEX_SIZE_U16);
    R_DrawIndexedPrimitives(command_buffer, geometry->index_count, 1, 0, 0, 0);
  }
}

func void
UpdateSkeletonGlobalTransform(Skeleton* skeleton)
{
  for (I32 i = 0; i < skeleton->joints.length; i += 1)
  {
    Joint* joint = JointArrayGetPointer(&skeleton->joints, i);

    joint->global_transform = joint->local_transform;

    if (joint->parent_id != JointID_Nil)
    {
      Joint* parent_joint = JointArrayGetPointer(&skeleton->joints, joint->parent_id);

      joint->global_transform.translation = AddVec3F32(
        RotateVec3F32(joint->local_transform.translation, parent_joint->global_transform.rotation),
        parent_joint->global_transform.translation
      );
      joint->global_transform.rotation = MulQuaternion(
        parent_joint->global_transform.rotation,
        joint->local_transform.rotation
      );
      joint->global_transform.scale = MulVec3F32(parent_joint->global_transform.scale, joint->local_transform.scale);
    }

    joint->inv_bind_pose = Mat4F32FromTransform(joint->global_transform);
    joint->inv_bind_pose = InverseMat4F32(joint->inv_bind_pose);
  }
}

func void
DrawSkeleton(R_CommandBuffer command_buffer, R_Buffer buffer, Skeleton* skeleton, Vec4F32 color)
{
  for (I32 i = 0; i < skeleton->joints.length; i += 1)
  {
    Joint joint1 = JointArrayGet(&skeleton->joints, i);

    if (joint1.parent_id != JointID_Nil)
    {
      Joint joint2 = JointArrayGet(&skeleton->joints, joint1.parent_id);

      Vec3F32 start = joint1.global_transform.translation;
      Vec3F32 end = joint2.global_transform.translation;

      DrawLine3D(command_buffer, buffer, start, end, color, 0.008f);
    }

    DrawLine3D(
      command_buffer, buffer, joint1.global_transform.translation,
      AddVec3F32(joint1.global_transform.translation, RotateVec3F32(MakeVec3F32(0.0f, 0.15f, 0.0f), joint1.global_transform.rotation)),
      MakeVec4F32(1.0f, 1.0f, 1.0f, 1.0f), 0.0025f
    );
  }
}

func void
DrawLine3D(R_CommandBuffer command_buffer, R_Buffer buffer, Vec3F32 start, Vec3F32 end, Vec4F32 color, F32 width)
{
  struct
  {
    Mat4 view_matrix;
    Mat4 projection_matrix;
    Vec3 camera_direction;
  } global_vertex_data;
  global_vertex_data.view_matrix = MakeLookAtMat4(
    app_state.camera.position,
    AddVec3(app_state.camera.position, app_state.camera.front),
    app_state.camera.up
  );
  global_vertex_data.projection_matrix = MakePerspectiveMat4(
    45.0f, (F32)app_state.window.size.w/(F32)app_state.window.size.h,
    0.1f, 100.0f
  );
  global_vertex_data.camera_direction = app_state.camera.front;

  U64 global_vertex_data_offset = R_PushBuffer(buffer, (U8*)&global_vertex_data, sizeof(global_vertex_data));

  struct
  {
    Vec4F32 line_color;
    Vec3F32 line_start;
    F32 line_width;
    Vec3F32 line_end;
  } instance_vertex_data;
  instance_vertex_data.line_color = color;
  instance_vertex_data.line_start = start;
  instance_vertex_data.line_width = width;
  instance_vertex_data.line_end = end;

  U64 instance_vertex_data_offset = R_PushBuffer(buffer, (U8*)&instance_vertex_data, sizeof(instance_vertex_data));

  R_UniformBufferBindingInfo line_vertex_global_uniform = {
    .buffer = buffer,
    .offset = global_vertex_data_offset,
    .size = sizeof(global_vertex_data),
  };

  R_UniformBufferBindingInfo line_vertex_instance_uniform = {
    .buffer = buffer,
    .offset = instance_vertex_data_offset,
    .size = sizeof(instance_vertex_data),
  };

  R_BindGraphicsPipeline(command_buffer, app_state.line_3d_pipeline);
  R_BindGlobalVertexShaderData(command_buffer, 1, &line_vertex_global_uniform, 0, 0);
  R_BindInstanceVertexShaderData(command_buffer, 1, &line_vertex_instance_uniform, 0, 0);
  R_DrawPrimitives(command_buffer, 6, 1, 0, 0);
}

func void
HandleEvents(Arena* arena, AppState* state)
{
  OS_EventList event_list = OS_GetEventList(arena, &state->window);
  state->last_mouse_position = OS_MousePosition(state->window);

  if (OS_IsKeyPressed(OS_KEY_U))
  {
    for (I32 i = 0; i < app_state.entities.length; i += 1)
    {
      Entity* entity = EntityArrayGetPointer(&app_state.entities, i);
      Skeleton* skeleton = &entity->mesh.skeleton;

      state->current_animation_index = (state->current_animation_index + 1)%entity->mesh.skeletal_animations.length;
      LOG_DEBUG("Current animation index = %i\n", state->current_animation_index);

      for (I32 j = 0; j < skeleton->joints.length; j += 1)
      {
        SkeletalAnimation* skeletal_animation = SkeletalAnimationArrayGetPointer(&entity->mesh.skeletal_animations, app_state.current_animation_index);
        Animation* animation = AnimationArrayGetPointer(&skeletal_animation->bone_animations, j);
        if (animation !=  &_animation_nil)
        {
          BeginAnimation(animation, OS_GetTimeTicks());
        }
      }
    }
  }

  if (OS_IsKeyPressed(OS_KEY_ESC))
  {
    state->is_window_closed = 1;
  }
  if (OS_IsKeyPressed(OS_KEY_TAB))
  {
    state->draw_ui = !state->draw_ui;
  }
  if (OS_IsKeyPressed(OS_KEY_F1))
  {
    ToggleCommandPalette(&state->command_palette);
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
  state->camera.position = AddVec3(state->camera.position, ScaleVec3(NormalizeVec3(direction), speed*state->delta_time_sec));

  if (OS_IsKeyDown(OS_KEY_ARROW_LEFT))
  {
    state->camera.yaw -= 25.0f*state->delta_time_sec;
  }
  if (OS_IsKeyDown(OS_KEY_ARROW_RIGHT))
  {
    state->camera.yaw += 25.0f*state->delta_time_sec;
  }
  if (OS_IsKeyDown(OS_KEY_ARROW_UP))
  {
    state->camera.pitch += 25.0f*state->delta_time_sec;
  }
  if (OS_IsKeyDown(OS_KEY_ARROW_DOWN))
  {
    state->camera.pitch -= 25.0f*state->delta_time_sec;
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
            
          if (state->window.size.x != 0 && state->window.size.y != 0)
          {
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
          else
          {
            state->to_render = 0;
          }
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

      default: break;
    }
  }
}

func void
IGN_DrawText(R_CommandBuffer command_buffer, R_Buffer buffer, FontBitmap font, Str8 text, U32 font_size, Vec2F32 position, Vec4F32 color)
{
  TextVertex vertecies[1028] = {0};
  U32 vertecies_count = 0;
  U16 indecies[1028] = {0};
  U32 indecies_count = 0;

  I32 line_count = 0;
  I32 symbols_on_line = 0;

  for (I32 i = 0; i < text.length; i += 1)
  {
    if (text.data[i] == '\n')
    {
      line_count += 1;
      symbols_on_line = 0;
      continue;
    }

    I32 glyph_id = text.data[i] - '!' + 1;
    
    Vec2 glyph_position = AddVec2(position, MakeVec2(symbols_on_line*(F32)font_size*0.5f, line_count*font_size*0.7f));
    Vec2 glyph_grid_xy = MakeVec2(glyph_id%font.glyphs_per_row, glyph_id/font.glyphs_per_row);
    Vec2 glyph_uv_size = DivVec2(MakeVec2(font.glyph_size.x, font.glyph_size.y), MakeVec2(font.bitmap_size.x, font.bitmap_size.y));
    
    vertecies[vertecies_count].position = glyph_position;;
    vertecies[vertecies_count].uv = MulVec2(glyph_grid_xy, glyph_uv_size);
    vertecies_count += 1;
    vertecies[vertecies_count].position = AddVec2(glyph_position, MakeVec2(font_size, 0.0f));
    vertecies[vertecies_count].uv = AddVec2(MulVec2(glyph_grid_xy, glyph_uv_size), MakeVec2(glyph_uv_size.x, 0.0f));
    vertecies_count += 1;
    vertecies[vertecies_count].position = AddVec2(glyph_position, MakeVec2(font_size, font_size));
    vertecies[vertecies_count].uv = AddVec2(MulVec2(glyph_grid_xy, glyph_uv_size), glyph_uv_size);
    vertecies_count += 1;
    vertecies[vertecies_count].position = AddVec2(glyph_position, MakeVec2(0.0f, font_size));
    vertecies[vertecies_count].uv = AddVec2(MulVec2(glyph_grid_xy, glyph_uv_size), MakeVec2(0.0f, glyph_uv_size.y));
    vertecies_count += 1;

    U16 offset = i*4;
    indecies[indecies_count] = 0 + offset;
    indecies_count += 1;
    indecies[indecies_count] = 2 + offset;
    indecies_count += 1;
    indecies[indecies_count] = 1 + offset;
    indecies_count += 1;
    indecies[indecies_count] = 2 + offset;
    indecies_count += 1;
    indecies[indecies_count] = 0 + offset;
    indecies_count += 1;
    indecies[indecies_count] = 3 + offset;
    indecies_count += 1;

    symbols_on_line += 1;
  }
  U64 vertex_buffer_offset = R_PushBuffer(buffer, (U8*)vertecies, sizeof(vertecies[0])*vertecies_count);
  U64 index_buffer_offset = R_PushBuffer(buffer, (U8*)indecies, sizeof(indecies[0])*indecies_count);

  R_BindGraphicsPipeline(command_buffer, app_state.font_pipeline);

  struct
  {
    Mat4 projection;
    Vec4 text_color;
  } font_vertex_shader_global_uniform_data;
  font_vertex_shader_global_uniform_data.projection = MakeOrthographicMat4(0.0f, app_state.window.size.x, 0.0f, app_state.window.size.y, -1.0f, 1.0f);
  font_vertex_shader_global_uniform_data.text_color = color;
  U64 font_vertex_shader_global_uniform_data_offset = R_PushBuffer(buffer, (U8*)&font_vertex_shader_global_uniform_data, sizeof(font_vertex_shader_global_uniform_data));
  R_UniformBufferBindingInfo font_vertex_shader_global_uniform = 
  {
    .buffer = buffer,
    .offset = font_vertex_shader_global_uniform_data_offset,
    .size = sizeof(font_vertex_shader_global_uniform_data),
  };
  R_BindGlobalVertexShaderData(command_buffer, 1, &font_vertex_shader_global_uniform, 0, 0);

  R_SamplerBindingInfo font_sampler = {
    .sampler = app_state.texture_sampler,
    .texture = app_state.font.bitmap,
  };
  R_BindGlobalFragmentShaderData(command_buffer, 0, 0, 1, &font_sampler);

  R_BindVertexBuffer(command_buffer, buffer, vertex_buffer_offset);
  R_BindIndexBuffer(command_buffer, buffer, index_buffer_offset, R_INDEX_SIZE_U16);
  // R_DrawPrimitives(command_buffer, 6, 1, 0, 0);
  R_DrawIndexedPrimitives(command_buffer, indecies_count, 1, 0, 0, 0);
}

func void
DrawRect(R_CommandBuffer command_buffer, R_Buffer buffer, RectF32 rect, Vec4 border_radius, Vec4 color)
{
  R_BindGraphicsPipeline(command_buffer,app_state.square_pipeline);
  struct
  {
    Mat4 projection;
  } square_global_vertex_data;
  square_global_vertex_data.projection = MakeOrthographicMat4(0.0f, app_state.window.size.x, 0.0f, app_state.window.size.y, -1.0f, 1.0f);
  U64 square_global_vertex_data_offset = R_PushBuffer(buffer, (U8*)&square_global_vertex_data, sizeof(square_global_vertex_data));
  R_UniformBufferBindingInfo square_vertex_shader_global_uniform = {
    .buffer = buffer,
    .offset = square_global_vertex_data_offset,
    .size = sizeof(square_global_vertex_data),
  };
  R_BindGlobalVertexShaderData(command_buffer, 1, &square_vertex_shader_global_uniform, 0, 0);

  struct
  {
    Vec2 position;
    Vec2 size;
  } square_instance_vertex_data;
  square_instance_vertex_data.position = rect.position;
  square_instance_vertex_data.size = rect.size;
  U64 square_instance_vertex_data_offset = R_PushBuffer(buffer, (U8*)&square_instance_vertex_data, sizeof(square_instance_vertex_data));
  R_UniformBufferBindingInfo square_vertex_shader_instance_uniform = {
    .buffer = buffer,
    .offset = square_instance_vertex_data_offset,
    .size = sizeof(square_instance_vertex_data),
  };
  R_BindInstanceVertexShaderData(command_buffer, 1, &square_vertex_shader_instance_uniform, 0, 0);

  struct
  {
    Vec4 color;
    Vec4 border_radius;
  } square_instance_fragment_data;
  square_instance_fragment_data.color = color;
  square_instance_fragment_data.border_radius = border_radius;
  U64 square_instance_fragment_shader_data_offset = R_PushBuffer(buffer, (U8*)&square_instance_fragment_data, sizeof(square_instance_fragment_data));
  R_UniformBufferBindingInfo square_fragment_shader_instance_uniform = {
    .buffer = buffer,
    .offset = square_instance_fragment_shader_data_offset,
    .size = sizeof(square_instance_fragment_data),
  };
  R_BindInstanceFragmentShaderData(command_buffer, 1, &square_fragment_shader_instance_uniform, 0, 0);

  R_DrawPrimitives(command_buffer, 6, 1, 0, 0);
}

func void
DrawCommandPalette(R_CommandBuffer command_buffer, R_Buffer buffer, CommandPalette* command_palette, U64 current_timestamp)
{
  DrawRect(command_buffer, buffer, command_palette->rectangle, MakeVec4F32(0.0f, 0.0f, 0.0f, 0.0f), command_palette->background_color);
}
