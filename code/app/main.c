#include "base/base_include.h"
#include "os/os_include.h"
#include "render/r_include.h"

#include "base/base_include.c"
#include "os/os_include.c"
#include "render/r_include.c"

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"

// -------------------------------------------------------------------
// UI
typedef struct FontBitmap FontBitmap;
struct FontBitmap
{
  R_Texture bitmap;
  Vec2U32 bitmap_size; // --AlNov: @TODO Should be in texture
  Vec2U32 glyph_size;
  U32 glyphs_per_row;
};

typedef struct TextVertex TextVertex;
struct TextVertex
{
  Vec2 position;
  Vec2 uv;
};

func Vec2 GetTextSize(FontBitmap font, Str8 text, U32 font_size);
func void DrawText(R_CommandBuffer command_buffer, R_Buffer buffer, FontBitmap font, Str8 text, U32 font_size, Vec2F32 position, Vec4F32 color);

typedef U8 UI_PositionType;
enum UI_PositionTypeEnum
{
  UI_Position
} UI_PositionTypeEnum;

typedef U8 UI_LayoutDirection;
enum UI_LayoutDirectionEnum
{
  UI_LayoutDirection_TopToBottom,
  UI_LayoutDirection_LeftToRight,
} UI_LayoutDirectionEnum;

typedef U8 UI_SizeType;
enum UI_SizeTypeEnum
{
  UI_SizeType_None,
  UI_SizeType_Fixed,
  UI_SizeType_WrapLabel,
  UI_SizeType_WrapChildren,
  UI_SizeType_ParentPercent,
  UI_SizeType_Count,
} UI_SizeTypeEnum;

typedef struct UI_Size UI_Size;
struct UI_Size
{
  UI_SizeType type;
  F32 value;
};

#define UI_FixedSize(coordinate) ((UI_Size){.type = UI_SizeType_Fixed, .value = coordinate})
#define UI_WrapLabelSize() ((UI_Size){.type = UI_SizeType_WrapLabel})
#define UI_ParentPercentSize(percent) ((UI_Size){.type = UI_SizeType_ParentPercent, .value = percent})

typedef struct UI_BorderRadius UI_BorderRadius;
struct UI_BorderRadius
{
  union
  {
    Vec4F32 values;
    struct 
    {
      F32 top_left;
      F32 top_right;
      F32 bottom_left;
      F32 bottom_right;
    };
  };
};

typedef U16 UI_ElementFlags;
enum UI_ElementFlagEnum
{
  // Draw Flags
  UI_ElementFlag_DrawBackground = 1 << 0,
  UI_ElementFlag_DrawLabel = 1 << 1,
} UI_ElementFlagEnum;

typedef struct UI_Element UI_Element;
struct UI_Element
{
  UI_Element* next;
  UI_Element* previous;
  UI_Element* parent;

  Str8 label;
  UI_ElementFlags flags;
  FontBitmap font;
  Vec4 text_color;
  U32 font_size;
  RectF32 rect;
  UI_LayoutDirection layout;
  F32 child_gap;
  Vec2 child_offset;
  Vec4 background_color;
  UI_BorderRadius border_radius;
};
UI_Element UI_ElementDefaultValue = {0};
DefineArray(UI_Element, UI_ElementArray, UI_ElementDefaultValue)

typedef struct UI_State UI_State;
struct UI_State
{
  UI_Element* parent;

  Vec2 mouse_position;

  UI_Size size_x;
  UI_Size size_y;
  Vec2 fixed_position;
  U32 font_size;
  FontBitmap font;
  Vec4 text_color;
  Vec4 background_color;
} ui_state;

func void UI_BeginFrame(Vec2 mouse_position)
{
  ui_state = (UI_State){0};
  ui_state.mouse_position = mouse_position;
}

func void UI_EndFrame()
{
}

func void UI_SetParent(UI_Element* parent) {ui_state.parent = parent;}
func void UI_SetSizeX(UI_Size size) {ui_state.size_x = size;}
func void UI_SetSizeY(UI_Size size) {ui_state.size_y = size;}
func void UI_SetFont(FontBitmap font, U32 font_size) {ui_state.font = font; ui_state.font_size = font_size;}
func void UI_SetTextColor(Vec4 color) {ui_state.text_color = color;}
func void UI_SetBackgrouncColor(Vec4 color) {ui_state.background_color = color;}
func void UI_SetFixedPosition(Vec2 position) {ui_state.fixed_position = position;}

typedef struct UI_ElementDescription UI_ElementDescription;
struct UI_ElementDescription
{
  Str8 label;
  UI_ElementFlags flags;
  UI_LayoutDirection layout;
  F32 child_gap;
  UI_BorderRadius border_radius;
};

func UI_Element*
UI_BuildElement(UI_ElementArray* array, UI_ElementDescription description)
{
  UI_Element element = {0};
  element.label = description.label;
  element.flags = description.flags;
  element.parent = ui_state.parent;
  element.layout = description.layout;
  element.child_gap = description.child_gap;
  element.font = ui_state.font;
  element.font_size = ui_state.font_size;
  element.border_radius = description.border_radius;

  switch (ui_state.size_x.type)
  {
    default: Assert(0); break;

    case UI_SizeType_Fixed:
    {
      element.rect.size.x = ui_state.size_x.value;
    } break;
    case UI_SizeType_WrapLabel:
    {
      element.rect.size.x = GetTextSize(ui_state.font, element.label, element.font_size).x;
    } break;
    case UI_SizeType_WrapChildren:
    {
      // --AlNov: @TODO
      Assert(0);
    } break;
    case UI_SizeType_ParentPercent:
    {
      element.rect.size.x = element.parent->rect.size.x*ui_state.size_x.value;
    } break;
  }
  switch (ui_state.size_y.type)
  {
    default: Assert(0); break;

    case UI_SizeType_Fixed:
    {
      element.rect.size.y = ui_state.size_y.value;
    } break;
    case UI_SizeType_WrapLabel:
    {
      element.rect.size.y = GetTextSize(ui_state.font, element.label, element.font_size).y;
    } break;
    case UI_SizeType_WrapChildren:
    {
      // --AlNov: @TODO
      Assert(0);
    } break;
    case UI_SizeType_ParentPercent:
    {
      element.rect.size.y = element.parent->rect.size.y*ui_state.size_y.value;
    } break;
  }

  if (element.parent)
  {
    switch (element.parent->layout)
    {
      case UI_LayoutDirection_TopToBottom:
      {
        element.rect.position = AddVec2(element.parent->rect.position,element.parent->child_offset);
        element.parent->child_offset.y += element.rect.size.y + element.parent->child_gap;
      } break;
      case UI_LayoutDirection_LeftToRight:
      {
        element.rect.position = AddVec2(element.parent->rect.position,element.parent->child_offset);
        element.parent->child_offset.x += element.rect.size.x + element.parent->child_gap;
      } break;
    }
  }
  else
  {
    element.rect.position = ui_state.fixed_position;
  }

  element.text_color = ui_state.text_color;
  element.background_color = ui_state.background_color;

  I32 index = UI_ElementArrayAdd(array, element);
  return UI_ElementArrayGetPointer(array, index);
}

func UI_Element*
UI_Layout(UI_ElementArray* array, Str8 label)
{
  UI_Element* layout = UI_BuildElement(
    array,
    (UI_ElementDescription){
      .label = label,
      .flags = UI_ElementFlag_DrawBackground,
    }
  );
  return layout;
}

func B32
UI_Button(UI_ElementArray* array, Str8 label)
{
  UI_Element* button = UI_BuildElement(
    array,
    (UI_ElementDescription){
      .label = label,
      .flags = UI_ElementFlag_DrawLabel|UI_ElementFlag_DrawBackground,
    }
  );

  return InsideRectF32(button->rect, ui_state.mouse_position);
}

func void DrawRect(R_CommandBuffer command_buffer, R_Buffer buffer, RectF32 rect, Vec4 border_radius, Vec4 color);

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
  R_GraphicsPipeline font_pipeline;

  R_TextureSampler texture_sampler;
  R_Texture mesh_texture;

  FontBitmap font;

  R_Texture depth_texture; // -AlNov: @TODO should it be created for R_VK_Swapchain?
  R_Texture test_texture;

  Camera camera;

  U32 hover_entity_id;

  F32 grid_scale;

  EntityArray entities;
  const Entity* selected_entity;

  UI_ElementArray ui_elements;
  B32 draw_ui;
} app_state;

func void HandleEvents(Arena* arena, AppState* state);

I32 main(void)
{
  UI_BorderRadius test_border = {
    .top_left = 5.0f
  };

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
  app_state.ui_elements = UI_ElementArrayAllocate(app_state.arena, 64);
  app_state.draw_ui = 1;

  F32 new_variable = 0;

  OS_Init(Megabytes(32));

  OS_CreateWindow(Str8C("Vulkan Triangle"), MakeVec2U32(1270, 720), &app_state.window);
  OS_ShowWindow(&app_state.window);

  R_Init(R_RENDERER_TYPE_VK, &app_state.window);

  R_CommandBuffer command_buffer = R_GetCommandBuffer();

  R_BufferUsageFlags triangle_buffer_usage_flags = R_BUFFER_USAGE_FLAG_VERTEX|R_BUFFER_USAGE_FLAG_INDEX|R_BUFFER_USAGE_FLAG_UNIFORM;
  R_Buffer data_buffer = R_CreateBuffer(Megabytes(64), triangle_buffer_usage_flags, R_BUFFER_PROPERTY_FLAG_HOST_COHERENT);
  R_Buffer transfer_buffer = R_CreateBuffer(Megabytes(64), R_BUFFER_USAGE_FLAG_TRANSFER, R_BUFFER_PROPERTY_FLAG_HOST_COHERENT);

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

    app_state.mesh_texture = R_CreateTexture(
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

    U64 mesh_texture_offset = R_PushBuffer(data_buffer, tex_pixels, texture_size);
    R_CopyBufferToTexture(0, data_buffer, mesh_texture_offset, texture_size, app_state.mesh_texture);
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

    R_ShaderCreateInfo font_fragment_shader_info = {
      .file_name = Str8C("./data/shaders/font.fs.glsl"),
      .type = R_SHADER_TYPE_FRAGMENT,
      .global_samplers_count = 1,
    };
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

    // UI
    UI_BeginFrame(app_state.last_mouse_position);
    UI_ElementArrayReset(&app_state.ui_elements);
    if (app_state.draw_ui)
    {
      UI_SetFont(app_state.font, 20);
      UI_SetTextColor(MakeVec4(1.0f, 1.0f, 0.2f, 1.0f));

      UI_SetFixedPosition(MakeVec2(0.0f, 0.0));
      UI_SetSizeX(UI_FixedSize(app_state.window.size.x));
      UI_SetSizeY(UI_FixedSize(30.0f));
      UI_SetBackgrouncColor(MakeVec4(0.0f, 0.0f, 0.0f, 0.4f));
      UI_Element* top_bar = UI_BuildElement(
        &app_state.ui_elements,
        (UI_ElementDescription){
          .label = Str8C("Top Bar"),
          .flags = UI_ElementFlag_DrawBackground,
          .layout = UI_LayoutDirection_LeftToRight,
        }
      );
      UI_SetParent(top_bar);
      {
        UI_SetSizeX(UI_WrapLabelSize());
        UI_SetSizeY(UI_ParentPercentSize(1.0f));
        UI_Button(&app_state.ui_elements, Str8C("Top"));
      }
      UI_SetParent(0);

      UI_SetFixedPosition(MakeVec2(0.0f, app_state.window.size.y*0.5f));
      UI_SetSizeX(UI_FixedSize(200.0f));
      UI_SetSizeY(UI_FixedSize(app_state.window.size.y*0.5f));
      UI_Element* right_box = UI_BuildElement(
        &app_state.ui_elements,
        (UI_ElementDescription){
          .label = Str8C("Right Box"),
          .flags = UI_ElementFlag_DrawBackground,
          .layout = UI_LayoutDirection_TopToBottom,
          .child_gap = 5.0f,
          .border_radius = {
            .top_left = 0.0f,
            .top_right = 20.0f,
            .bottom_left = 0.0f,
            .bottom_right = 20.0f,
          }
        }
      );
      UI_SetParent(right_box);
      {
        UI_SetBackgrouncColor(MakeVec4(0.0f, 0.0f, 0.0f, 0.6f));

        UI_SetSizeX((UI_Size){.type = UI_SizeType_ParentPercent, .value = 1.0f});
        UI_SetSizeY((UI_Size){.type = UI_SizeType_WrapLabel,});
        if(UI_Button(&app_state.ui_elements, Str8C("Test Button")))
        {
          if (OS_IsMousePressed(OS_MouseButton_Left))
          {
            LOG_DEBUG("BUTTON 1 Pressed\n");
          }
        }
        for (I32 i = 0; i < 8; i += 1)
        {
          UI_Button(&app_state.ui_elements, Str8C("Test Button 2"));
        // UI_Button(&app_state.ui_elements, Str8C("Test Button 3"));
        }
      }
      UI_SetParent(0);
    }

    R_ResetBuffer(data_buffer);

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
        sprintf(frame_time_cstring, "%.3f", app_state.delta_time*1000.0f);
        DrawText(command_buffer, data_buffer, app_state.font, Str8C(frame_time_cstring), 24, MakeVec2(0.0f, 0.0f), MakeVec4(1.0f, 1.0f, 1.0f, 1.0f));
        DrawText(command_buffer, data_buffer, app_state.font, Str8C("Testing Text Rendering."), 40, MakeVec2(0.0f, 50.0f), RGBAFromHex(0x9ABBD1FF));

        for (I32 i = 0; i < app_state.ui_elements.length; i += 1)
        {
          UI_Element* ui_element = UI_ElementArrayGetPointer(&app_state.ui_elements, i);

          if ((ui_element->flags & UI_ElementFlag_DrawBackground) == UI_ElementFlag_DrawBackground)
          {
            DrawRect(command_buffer, data_buffer, ui_element->rect, ui_element->border_radius.values, ui_element->background_color);
          }

          if ((ui_element->flags & UI_ElementFlag_DrawLabel) == UI_ElementFlag_DrawLabel)
          {
            DrawText(command_buffer, data_buffer, ui_element->font, ui_element->label, ui_element->font_size, ui_element->rect.position, MakeVec4(1.0f, 0.0f, 1.0f, 1.0f));
          }
        }

        if (app_state.mouse_inside)
        {
          RectF32 cursor = {
            .position = app_state.last_mouse_position,
            .size = MakeVec2(20.0f, 20.0f),
          };
          DrawRect(command_buffer, data_buffer, cursor, MakeVec4(0.0f, 10.0f, 10.0f, 10.0f), MakeVec4(1.0f, 1.0f, 1.0f, 1.0f));
        }
      }
      R_EndRenderPass(command_buffer, 0);

      // R_CopyTexture(command_buffer, swapchain_texture, app_state.test_texture);
      test_texture_values_offset = R_CopyTextureToBuffer(command_buffer, app_state.test_texture, transfer_buffer);
    }
    R_SubmitCommandBuffer(command_buffer);

    R_PresentTexture(command_buffer, swapchain_texture);

    if (app_state.last_mouse_position.x >= 0.0f && app_state.last_mouse_position.y >= 0)
    {
      U64 pixel_offset = sizeof(U16)*(((I32)app_state.window.size.x*(I32)app_state.last_mouse_position.y) + (I32)app_state.last_mouse_position.x);
      R_VK_BufferGetData(transfer_buffer, test_texture_values_offset + pixel_offset, &app_state.hover_entity_id, sizeof(U16));
    }
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
  if (OS_IsKeyPressed(OS_KEY_TAB))
  {
    state->draw_ui = !state->draw_ui;
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

func Vec2
GetTextSize(FontBitmap font, Str8 text, U32 font_size)
{
  // --AlNov: @TODO Font spacing is hardcoded and not correct (Bitmap Grid size is 30px, but glyphs is smaller)
  Vec2 result = MakeVec2(font_size*0.5f, font_size);

  for (I32 i = 0; i < text.length; i += 1)
  {
    if (text.data[i] == '\n')
    {
      result.y += font_size*0.7f;
      continue;
    }
    result.x += font_size*0.5f;
  }

  return result;
}

func void
DrawText(R_CommandBuffer command_buffer, R_Buffer buffer, FontBitmap font, Str8 text, U32 font_size, Vec2F32 position, Vec4F32 color)
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
