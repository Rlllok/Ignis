#include "base/base_include.h"
#include "os/os_include.h"
#include "render/r_include.h"
#include "assets/animation.h"
#include "assets/mesh.h"

#include "base/base_include.c"
#include "os/os_include.c"
#include "render/r_include.c"
#include "assets/animation.c"
#include "assets/mesh.c"

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"

// -------------------------------------------------------------------
// UI
typedef U32 UI_ID;

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
func void IGN_DrawText(R_CommandBuffer command_buffer, R_Buffer buffer, FontBitmap font, Str8 text, U32 font_size, Vec2F32 position, Vec4F32 color);

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

typedef struct UI_Padding UI_Padding;
struct UI_Padding
{
  union
  {
    Vec4F32 v;
    struct
    {
      F32 top;
      F32 right;
      F32 bottom;
      F32 left;
    };
  };
};

typedef U16 UI_ElementFlags;
enum UI_ElementFlagEnum
{
  // Interaction Flags
  UI_ElementFlag_Hover = 1 << 0,
  UI_ElementFlag_Clickable = 1 << 1,

  // Draw Flags
  UI_ElementFlag_DrawBackground = 1 << 2,
  UI_ElementFlag_DrawLabel = 1 << 3,
} UI_ElementFlagEnum;

typedef struct UI_Element UI_Element;
struct UI_Element
{
  UI_ID id;

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
  UI_Padding padding;
  F32 child_gap;
  Vec2 child_offset;
  Vec4 background_color;
  UI_BorderRadius border_radius;
};

UI_Element UI_ElementDefaultValue = {0};
DefineArray(UI_Element, UI_ElementArray, UI_ElementDefaultValue)

typedef U8 UI_DrawCommandType;
enum UI_DrawCommandTypeEnum
{
  UI_DrawCommandType_Rectangle,
  UI_DrawCommandType_Text,
} UI_DrawCommandTypeEnum;

typedef struct UI_DrawCommand UI_DrawCommand;
struct UI_DrawCommand
{
  UI_DrawCommandType type;
  union
  {
    struct
    {
      Vec4 color;
      RectF32 bound;
      Vec4 radius;
    } rectangle;

    struct
    {
      Str8 content;
      FontBitmap font;
      F32 font_size;
      Vec4 color;
      Vec2 position;
    } text;
  };
};
UI_DrawCommand UI_DrawCommandDefaultValue = {0};
DefineArray(UI_DrawCommand, UI_DrawCommandArray, UI_DrawCommandDefaultValue)

typedef struct UI_Context UI_Context;
struct UI_Context
{
  UI_Element* current_parent;

  UI_Size size_x;
  UI_Size size_y;
  Vec2 fixed_position;
  U32 font_size;
  FontBitmap font;
  Vec4 text_color;
  Vec4 background_color;

  // Interaction
  UI_ID hot_id;
  UI_ID active_id;
  Vec2 mouse_position;

  // Draw
  UI_DrawCommandArray draw_commands;

  UI_ElementArray elements;
} ui_context;

func void UI_BeginFrame(UI_Context* context, Vec2 mouse_position)
{
  context->current_parent = 0;
  context->mouse_position = mouse_position;
  context->size_x = (UI_Size){0};
  context->size_y = (UI_Size){0};
  context->fixed_position = MakeVec2(0.0f, 0.0f);
  context->font_size = 0.0f;
  context->font = (FontBitmap){0};
  context->text_color = MakeVec4(0.0f, 0.0f, 0.0f, 0.0f);
  context->background_color = MakeVec4(0.0f, 0.0f, 0.0f, 0.0f);
  context->hot_id = 0;
  UI_DrawCommandArrayReset(&context->draw_commands);
}

func void UI_EndFrame()
{
}

func void UI_SetParent(UI_Element* parent) {ui_context.current_parent = parent;}
func void UI_SetSizeX(UI_Size size) {ui_context.size_x = size;}
func void UI_SetSizeY(UI_Size size) {ui_context.size_y = size;}
func void UI_SetFont(FontBitmap font, U32 font_size) {ui_context.font = font; ui_context.font_size = font_size;}
func void UI_SetTextColor(Vec4 color) {ui_context.text_color = color;}
func void UI_SetBackgroundColor(Vec4 color) {ui_context.background_color = color;}
func void UI_SetFixedPosition(Vec2 position) {ui_context.fixed_position = position;}

typedef struct UI_ElementDescription UI_ElementDescription;
struct UI_ElementDescription
{
  Str8 label;
  UI_ElementFlags flags;
  UI_LayoutDirection layout;
  UI_Padding padding;
  F32 child_gap;
  UI_BorderRadius border_radius;
};

func UI_Element*
UI_BuildElement(UI_ElementArray* array, UI_ElementDescription description)
{
  UI_Element element = {0};
  element.id = array->length + 1;
  element.label = description.label;
  element.flags = description.flags;
  element.parent = ui_context.current_parent;
  element.layout = description.layout;
  element.padding = description.padding;
  element.child_gap = description.child_gap;
  element.font = ui_context.font;
  element.font_size = ui_context.font_size;
  element.border_radius = description.border_radius;
  element.text_color = ui_context.text_color;

  element.child_offset = MakeVec2(element.padding.left, element.padding.top);

  switch (ui_context.size_x.type)
  {
    default: Assert(1); break;

    case UI_SizeType_Fixed:
    {
      element.rect.size.x = ui_context.size_x.value;
    } break;
    case UI_SizeType_WrapLabel:
    {
      element.rect.size.x = GetTextSize(ui_context.font, element.label, element.font_size).x;
    } break;
    case UI_SizeType_WrapChildren:
    {
      // --AlNov: @TODO
      Assert(1);
    } break;
    case UI_SizeType_ParentPercent:
    {
      element.rect.size.x = element.parent->rect.size.x*ui_context.size_x.value;
    } break;
  }
  switch (ui_context.size_y.type)
  {
    default: Assert(1); break;

    case UI_SizeType_Fixed:
    {
      element.rect.size.y = ui_context.size_y.value;
    } break;
    case UI_SizeType_WrapLabel:
    {
      element.rect.size.y = GetTextSize(ui_context.font, element.label, element.font_size).y;
    } break;
    case UI_SizeType_WrapChildren:
    {
      // --AlNov: @TODO
      Assert(1);
    } break;
    case UI_SizeType_ParentPercent:
    {
      element.rect.size.y = element.parent->rect.size.y*ui_context.size_y.value;
    } break;
  }

  if (element.parent)
  {
    switch (element.parent->layout)
    {
      case UI_LayoutDirection_TopToBottom:
      {
        element.rect.position = AddVec2(element.parent->rect.position, element.parent->child_offset);
        element.parent->child_offset.y += element.rect.size.y + element.parent->child_gap;
      } break;
      case UI_LayoutDirection_LeftToRight:
      {
        element.rect.position = AddVec2(element.parent->rect.position, element.parent->child_offset);
        element.parent->child_offset.x += element.rect.size.x + element.parent->child_gap;
      } break;
    }
  }
  else
  {
    element.rect.position = ui_context.fixed_position;
  }

  element.text_color = ui_context.text_color;
  element.background_color = ui_context.background_color;

  if (element.flags & UI_ElementFlag_Hover)
  {
    if (InsideRectF32(element.rect, ui_context.mouse_position))
    {
      ui_context.hot_id = element.id;
      element.background_color = AddVec4(element.background_color, MakeVec4(0.2f, 0.2f, 0.2f, 0.0f));
    }
  }
  if (element.flags & UI_ElementFlag_Clickable)
  {
  }
  if (element.flags & UI_ElementFlag_DrawBackground)
  {
    UI_DrawCommandArrayAdd(
      &ui_context.draw_commands,
      (UI_DrawCommand){
        .type = UI_DrawCommandType_Rectangle,
        .rectangle = {
          .color = element.background_color,
          .bound = element.rect,
          .radius = element.border_radius.values,
        }
      }
    );
  }
  if (element.flags & UI_ElementFlag_DrawLabel)
  {
    UI_DrawCommandArrayAdd(
      &ui_context.draw_commands,
      (UI_DrawCommand){
        .type = UI_DrawCommandType_Text,
        .text = {
          .content = element.label,
          .font = element.font,
          .font_size = element.font_size,
          .color = element.text_color,
          .position = element.rect.position,
        }
      }
    );
  }

  return UI_ElementArrayGetPointer(array, UI_ElementArrayAdd(array, element));
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

func void
UI_Text(UI_ElementArray* array, Str8 label)
{
  UI_BuildElement(
    array,
    (UI_ElementDescription){
      .label = label,
      .flags = UI_ElementFlag_DrawLabel,
    }
  );
}

func void
UI_NumberInput(UI_ElementArray* array, Str8 label, F32* value)
{
  F32 result = *value;

  UI_Element* input = UI_BuildElement(
    array,
    (UI_ElementDescription){
      .label = label,
      .flags = UI_ElementFlag_Hover|
        UI_ElementFlag_DrawLabel|
        UI_ElementFlag_DrawBackground,
    }
  );

  if (ui_context.active_id == input->id)
  {
    if (OS_IsKeyPressed(OS_KEY_0))
    {
      result = result*10;
    }
    else if (OS_IsKeyPressed(OS_KEY_1))
    {
      result = result*10 + 1;
    }
    else if (OS_IsKeyPressed(OS_KEY_2))
    {
      result = result*10 + 2;
    }
    else if (OS_IsKeyPressed(OS_KEY_3))
    {
      result = result*10 + 3;
    }
    else if (OS_IsKeyPressed(OS_KEY_4))
    {
      result = result*10 + 4;
    }
    else if (OS_IsKeyPressed(OS_KEY_5))
    {
      result = result*10 + 5;
    }
    else if (OS_IsKeyPressed(OS_KEY_6))
    {
      result = result*10 + 6;
    }
    else if (OS_IsKeyPressed(OS_KEY_7))
    {
      result = result*10 + 7;
    }
    else if (OS_IsKeyPressed(OS_KEY_8))
    {
      result = result*10 + 8;
    }
    else if (OS_IsKeyPressed(OS_KEY_9))
    {
      result = result*10 + 9;
    }
    else if (OS_IsKeyPressed(OS_KEY_BACKSPACE))
    {
      result = (F32)((I32)(result/10));
    }
    else if (OS_IsKeyPressed(OS_KEY_RETURN))
    {
      ui_context.active_id = 0;
    }
  }
  else if (ui_context.hot_id == input->id)
  {
    if (OS_IsMousePressed(OS_MouseButton_Left)) ui_context.active_id = input->id;
  }

  *value = result;
}

func B32
UI_Button(UI_ElementArray* array, Str8 label)
{
  B32 result = 0;

  UI_Element* button = UI_BuildElement(
    array,
    (UI_ElementDescription){
      .label = label,
      .flags = UI_ElementFlag_Hover|
        UI_ElementFlag_DrawLabel|
        UI_ElementFlag_DrawBackground,
      .border_radius = {
        .top_right = 10.0f,
        .bottom_right = 10.0f,
      },
    }
  );

  if (ui_context.active_id == button->id)
  {
    if ((ui_context.hot_id == button->id) && OS_IsMouseReleased(OS_MouseButton_Left))
    {
      ui_context.active_id = 0;
      result = 1;
    }
  }
  else if (ui_context.hot_id == button->id)
  {
    if (OS_IsMousePressed(OS_MouseButton_Left)) ui_context.active_id = button->id;
  }

  return result;
}

func F32
UI_SliderF32(UI_ElementArray* array, Str8 label, F32 min, F32 max, F32* value)
{
  UI_Element* slider = UI_BuildElement(
    array,
    (UI_ElementDescription){
      .label = label,
      .flags = UI_ElementFlag_Hover|
        UI_ElementFlag_DrawLabel|
        UI_ElementFlag_DrawBackground,
    }
  );

  F32 slider_value = (*value - min)/(max-min);

  UI_DrawCommandArrayAdd(
    &ui_context.draw_commands,
    (UI_DrawCommand){
      .type = UI_DrawCommandType_Rectangle,
      .rectangle = {
        .color = MakeVec4(0.4f, 0.4f, 0.4f, 0.5f),
        .bound = {
          .position = slider->rect.position,
          .size.x = slider->rect.size.x*slider_value,
          .size.y = slider->rect.size.y,
        },
      },
    }
  );

  if (ui_context.active_id == slider->id)
  {
    if (OS_IsMouseDown(OS_MouseButton_Left))
    {
      F32 slider_value = (ui_context.mouse_position.x - slider->rect.position.x)/slider->rect.size.x;
      slider_value = Clamp(slider_value, 0.0f, 1.0f);
      *value = min*(1.0f - slider_value) + max*slider_value;
    }
    else
    {
      ui_context.active_id = 0;
    }
  }
  else if (ui_context.hot_id == slider->id)
  {
    if (OS_IsMousePressed(OS_MouseButton_Left))
    {
      ui_context.active_id = slider->id;
    }
  }

  return 0.0f;
}

func void DrawRect(R_CommandBuffer command_buffer, R_Buffer buffer, RectF32 rect, Vec4 border_radius, Vec4 color);

func void UpdateSkeletonGlobalTransform(Skeleton* skeleton);
func void StartAnimation(SkeletonAnimation* animation, U64 current_time);
func void EndAnimation(SkeletonAnimation* animation);
func void AnimateSkeleton(Skeleton* skeleton, SkeletonAnimation* animation, U64 current_time);
func void DrawSkeleton(R_CommandBuffer command_buffer, R_Buffer buffer, Skeleton* skeleton);

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
  AST_StaticMesh joint_mesh;

  SkeletonArray skeletons;
  SkeletonAnimationArray skeleton_animations;
  SkeletonAnimationIDArray running_animations;

  Animation animation;

  U32 hover_entity_id;

  F32 grid_scale;

  EntityArray entities;
  Entity* selected_entity;

  // --AlNov: @TODO Remove. It is for test
  F32 roll;
  F32 pitch;
  F32 yaw;

  B32 to_render;
  B32 draw_ui;

  CommandPalette command_palette;
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
  app_state.camera.position = MakeVec3(0.0f, 2.5f, 4.5f);
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
    .activated = 1,
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

  const I32 joint_count = 3;
  app_state.skeletons = SkeletonArrayAllocate(app_state.arena, 4);

  for (I32 skeleton_id = 0; skeleton_id < app_state.skeletons.capacity; skeleton_id += 1)
  {
    Skeleton skeleton = SkeletonArrayGet(&app_state.skeletons, skeleton_id);
    skeleton.joints = JointArrayAllocate(app_state.arena, joint_count);
    
    Vec3F32 translations[3] = {
      MakeVec3F32(0.0f, 0.0f, 0.0f),
      MakeVec3F32(0.0f, 1.0f, 0.0f),
      MakeVec3F32(0.0f, 0.25f, 0.0f),
    };

    Quaternion rotations[3] = {
      QuaternionFromEuler(RadiansFromDegrees(0.0f), 0.0f, 0.0f),
      QuaternionFromEuler(RadiansFromDegrees(90.0f), 0.0f, 0.0f),
      QuaternionFromEuler(RadiansFromDegrees(-15.0f), 0.0f, 0.0f),
    };

    for (I32 i = 0; i < skeleton.joints.capacity; i += 1)
    {
      Joint joint = {
        .parent_id = ((i - 1) >= 0) ? (i - 1) : JointID_Nil,
        .local_transform.translation = translations[i],
        .local_transform.rotation = rotations[i],
      };
      JointArrayAdd(&skeleton.joints, joint);
    }

    UpdateSkeletonGlobalTransform(&skeleton);
    SkeletonArrayAdd(&app_state.skeletons, skeleton);
  }

  app_state.skeleton_animations = SkeletonAnimationArrayAllocate(app_state.arena, app_state.skeletons.capacity);
  app_state.running_animations = SkeletonAnimationIDArrayAllocate(app_state.arena, 16);
  for (I32 animation_id = 0; animation_id < app_state.skeletons.length; animation_id += 1)
  {
    SkeletonAnimation animation = (SkeletonAnimation){
      .id = animation_id,
      .skeleton_id = animation_id,
      .key_samples = SkeletonKeySampleArrayAllocate(app_state.arena, 3),
      .duration = 4*1000/(animation_id + 1),
      .start_time = 0,
      .end_time = 0,
    };

    for (I32 i = 0; i < 3; i += 1)
    {
      Skeleton* skeleton = SkeletonArrayGetPointer(&app_state.skeletons, animation.skeleton_id);

      SkeletonKeySample key_sample = _skeleton_key_sample_nil;
      key_sample.local_joint_transforms = TransformArrayAllocate(app_state.arena, 3);
      key_sample.timestamp = (animation.duration/2)*i;

      Joint joint = JointArrayGet(&skeleton->joints, 0);
      Transform local_transform = joint.local_transform;
      Vec3F32 translation_table[3] = {
        MakeVec3F32(0.0f, 0.0f + (animation_id*1.0f), 0.0f),
        MakeVec3F32(1.0f, 1.0f + (animation_id*1.0f), 0.0f),
        MakeVec3F32(3.0f, 0.0f + (animation_id*1.0f), 0.0f),
      };
      Quaternion rotation_table[3] = {
        QuaternionFromEuler(0.0f, 0.0f, 0.0f),
        QuaternionFromEuler(90.0f, RadiansFromDegrees(300.0f), 0.0f),
        QuaternionFromEuler(180.0f, 0.0f, 0.0f),
      };
      local_transform.translation = translation_table[i];
      local_transform.rotation = rotation_table[i];
      TransformArrayAdd(&key_sample.local_joint_transforms, local_transform);

      joint = JointArrayGet(&skeleton->joints, 1);
      local_transform = joint.local_transform;
      local_transform.rotation = rotation_table[i];
      TransformArrayAdd(&key_sample.local_joint_transforms, local_transform);
      joint = JointArrayGet(&skeleton->joints, 2);
      local_transform = joint.local_transform;
      TransformArrayAdd(&key_sample.local_joint_transforms, joint.local_transform);

      SkeletonKeySampleArrayAdd(&animation.key_samples, key_sample);
    }

    SkeletonAnimationArrayAdd(&app_state.skeleton_animations, animation);
  }

  app_state.animation = (Animation){
    .duration = 1*1000,
    .looped = 1,
    .points = AnimationPointArrayAllocate(app_state.arena, 2)
  };
  AnimationPoint point = (AnimationPoint){
    .type = AnimationPointType_Linear,
    .timestamp = 0*1000,
    .linear.transform = (Transform){
      .translation = MakeVec3F32(0.0f, 0.0f, 0.0f),
      .rotation = IdentityQuaternion(),
      .scale = MakeVec3F32(1.0f, 1.0f, 1.0f),
    }
  };
  AnimationPointArrayAdd(&app_state.animation.points, point);
  point = (AnimationPoint){
    .type = AnimationPointType_Linear,
    .timestamp = 1*1000,
    .linear.transform = (Transform){
      .translation = MakeVec3F32(2.0f, 0.0f, 0.0f),
      .rotation = IdentityQuaternion(),
      .scale = MakeVec3F32(1.0f, 1.0f, 1.0f),
    }
  };
  AnimationPointArrayAdd(&app_state.animation.points, point);

  ui_context.elements = UI_ElementArrayAllocate(app_state.arena, 1024);
  ui_context.draw_commands = UI_DrawCommandArrayAllocate(app_state.arena, 1024);

  OS_Init(Megabytes(32));

  OS_CreateWindow(Str8C("Vulkan Triangle"), MakeVec2U32(1270, 720), &app_state.window);
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
  dummy_mesh.simple_animation.looped = 1;
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

  app_state.joint_mesh = AST_LoadStaticMeshFromGLTF(app_state.arena, Str8C("data/Cone/Cone.gltf"));

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
    UI_BeginFrame(&ui_context, app_state.last_mouse_position);
    UI_ElementArrayReset(&ui_context.elements);
    if (app_state.draw_ui)
    {
      UI_SetBackgroundColor(RGBAFromHex(0x1D1A26DD));
      UI_SetFont(app_state.font, 16);
      UI_SetTextColor(RGBAFromHex(0xE8B4B8FF));

      UI_SetFixedPosition(MakeVec2(0.0f, 0.0f));
      UI_SetSizeX(UI_FixedSize(250.0f));
      UI_SetSizeY(UI_FixedSize(app_state.window.size.y));
      UI_Element* right_box = UI_BuildElement(
        &ui_context.elements,
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
        UI_SetSizeX(UI_ParentPercentSize(1.0f));
        UI_SetSizeY(UI_FixedSize(20.0f));

        if (app_state.selected_entity != &EntityDefaultValue)
        {
          UI_Text(&ui_context.elements, app_state.selected_entity->name);

          UI_Element* euler_rotation = UI_BuildElement(
            &ui_context.elements,
            (UI_ElementDescription) {
              .layout = UI_LayoutDirection_LeftToRight,
              .child_gap = 2.0f,
            }
          );

          UI_SetParent(euler_rotation);
          {
            Vec3F32 euler = EulerFromQuaternion(app_state.selected_entity->transform.rotation);
            euler.x = DegreesFromRadians(euler.x);
            euler.y = DegreesFromRadians(euler.y);
            euler.z = DegreesFromRadians(euler.z);

            Vec3F32 input_euler = euler;

            char roll_cstring[128] = {0};
            sprintf(roll_cstring, "Roll: %.2f", input_euler.x);
            UI_NumberInput(&ui_context.elements, Str8C(roll_cstring), &input_euler.x);

            char pitch_cstring[128] = {0};
            sprintf(pitch_cstring, "Pitch: %.2f", input_euler.y);
            UI_NumberInput(&ui_context.elements, Str8C(pitch_cstring), &input_euler.y);

            char yaw_cstring[128] = {0};
            sprintf(yaw_cstring, "Yaw: %.2f", input_euler.z);
            UI_NumberInput(&ui_context.elements, Str8C(yaw_cstring), &input_euler.z);

            app_state.selected_entity->transform.rotation = QuaternionFromEuler(
              RadiansFromDegrees(input_euler.x),
              RadiansFromDegrees(input_euler.y),
              RadiansFromDegrees(input_euler.z)
            );
          }
          UI_SetParent(right_box);

          char position_x_slider_cstring[128] = {0};
          sprintf(position_x_slider_cstring, "X: %.1f", app_state.selected_entity->transform.translation.x);
          UI_SliderF32(&ui_context.elements, Str8C(position_x_slider_cstring), -5.0f, 5.0f, &app_state.selected_entity->transform.translation.x);
          char position_y_slider_cstring[128] = {0};
          sprintf(position_y_slider_cstring, "Y: %.1f", app_state.selected_entity->transform.translation.y);
          UI_SliderF32(&ui_context.elements, Str8C(position_y_slider_cstring), -5.0f, 5.0f, &app_state.selected_entity->transform.translation.y);
          char position_z_slider_cstring[128] = {0};
          sprintf(position_z_slider_cstring, "Z: %.1f", app_state.selected_entity->transform.translation.z);
          UI_SliderF32(&ui_context.elements, Str8C(position_z_slider_cstring), -5.0f, 5.0f, &app_state.selected_entity->transform.translation.z);

          char smoothness_slider_cstring[128] = {0};
          sprintf(smoothness_slider_cstring, "Smoothness: %.1f", app_state.selected_entity->smoothness);
          UI_SliderF32(&ui_context.elements, Str8C(smoothness_slider_cstring), 0.0f, 1.0f, &app_state.selected_entity->smoothness);
        }
      }
      UI_SetParent(0);
      if (app_state.command_palette.activated)
      {
        UI_SetFixedPosition(app_state.command_palette.rectangle.position);
        UI_SetSizeX(UI_FixedSize(app_state.command_palette.rectangle.size.x));
        UI_SetSizeY(UI_FixedSize(app_state.command_palette.rectangle.size.y));
        UI_Element* command_palette = UI_BuildElement(
          &ui_context.elements,
          (UI_ElementDescription){
            .label = Str8C("CommandPalette"),
            .flags = UI_ElementFlag_DrawBackground,
            .layout = UI_LayoutDirection_TopToBottom,
            .child_gap = 5.0f,
            .border_radius = app_state.command_palette.border_radius,
          }
        );
        UI_SetParent(command_palette);
        {
          UI_SetSizeX(UI_ParentPercentSize(1.0f));
          UI_SetSizeY(UI_FixedSize(40.0f));
          UI_Text(&ui_context.elements, app_state.command_palette.input);
          for (I32 i = 0; i < app_state.command_palette.commands.length; i += 1)
          {
            Command command = CommandArrayGet(&app_state.command_palette.commands, i);
            if (UI_Button(&ui_context.elements, command.name))
            {
              command.callback(command.data);
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
            // entity->transform = AnimateTransform(entity->mesh.simple_animation, OS_GetTimeTicks());
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
          for (I32 i = 0; i < app_state.running_animations.length; i += 1)
          {
            SkeletonAnimationID id = SkeletonAnimationIDArrayGet(&app_state.running_animations, i);
            if (id != SkeletonAnimationID_Nil)
            {
              LOG_DEBUG("Animation is going\n");
              SkeletonAnimation* animation = SkeletonAnimationArrayGetPointer(&app_state.skeleton_animations, id);
              Skeleton* skeleton = SkeletonArrayGetPointer(&app_state.skeletons, animation->skeleton_id);
              AnimateSkeleton(skeleton, animation, OS_GetTimeTicks());
            }
          }

          for (I32 i = 0; i < app_state.skeletons.length; i += 1)
          {
            Skeleton* skeleton = SkeletonArrayGetPointer(&app_state.skeletons, i);
            DrawSkeleton(command_buffer, data_buffer, skeleton);
          }
          
          for (I32 i = 0; i < app_state.entities.length; i += 1)
          {
            Entity* entity = EntityArrayGetPointer(&app_state.entities, i);
            Skeleton* skeleton = &entity->mesh.skeleton;
            for (I32 j = 0; j < skeleton->joints.length; j += 1)
            {
              Joint* joint = JointArrayGetPointer(&skeleton->joints, j);
              Animation* animation = AnimationArrayGetPointer(&entity->mesh.skeletal_animation.bone_animations, j);
              animation->looped = 1;
              joint->local_transform = AnimateTransform(*animation, OS_GetTimeTicks());
            }
            UpdateSkeletonGlobalTransform(skeleton);
            DrawSkeleton(command_buffer, data_buffer, skeleton);
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

func SkeletonID CreateSkeleton()
{
  // --AlNov: @TODO
  return  SkeletonID_Nil;
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
    }

    joint->inv_bind_pose = MakeMat4F32(1.0f);
    joint->inv_bind_pose = MulMat4F32(Mat4F32FromQuaternion(joint->global_transform.rotation), joint->inv_bind_pose);
    joint->inv_bind_pose = MulMat4F32(MakeTransposeMat4F32(joint->global_transform.translation), joint->inv_bind_pose);
    joint->inv_bind_pose = InverseMat4F32(joint->inv_bind_pose);
  }
}

func void
StartAnimation(SkeletonAnimation* animation, U64 current_time)
{
  EndAnimation(animation);
  
  if (app_state.running_animations.length < app_state.running_animations.capacity)
  {
    U32 free_slot = U32_MAX;
    for (I32 i = 0; i < app_state.running_animations.length; i += 1)
    {
      if (SkeletonAnimationIDArrayGet(&app_state.running_animations, i) == SkeletonAnimationID_Nil)
      {
        free_slot = i;
      }
    }

    if (free_slot == U32_MAX)
    {
      SkeletonAnimationIDArrayAdd(&app_state.running_animations, animation->id);
    }
    else
    {
      SkeletonAnimationIDArraySet(&app_state.running_animations, free_slot, animation->id);
    }
  
    animation->start_time = current_time;
    animation->end_time = animation->start_time + animation->duration;
  }
}

func void
EndAnimation(SkeletonAnimation* animation)
{
  for (I32 i = 0; i < app_state.running_animations.length; i += 1)
  {
    if (SkeletonAnimationIDArrayGet(&app_state.running_animations, i) == animation->id)
    {
      SkeletonAnimationIDArraySet(&app_state.running_animations, i, SkeletonAnimationID_Nil);
      animation->start_time = 0;
      animation->end_time = 0;
      break;
    }
  }
}

func void
AnimateSkeleton(Skeleton* skeleton, SkeletonAnimation* animation, U64 current_time)
{
  U64 animation_time = current_time - animation->start_time;
  if (animation_time > animation->duration)
  {
    EndAnimation(animation);
    return;
  }

  SkeletonKeySample current_sample = SkeletonKeySampleArrayGet(&animation->key_samples, 0);
  SkeletonKeySample next_sample = SkeletonKeySampleArrayGet(&animation->key_samples, 0);
  for (I32 i = 1; i < animation->key_samples.length; i += 1)
  {
    next_sample = SkeletonKeySampleArrayGet(&animation->key_samples, i);
    if (next_sample.timestamp > animation_time)
    {
      break;
    }
    current_sample = next_sample;
  }
  F32 blend_value = (F32)(animation_time - current_sample.timestamp)/(F32)(next_sample.timestamp - current_sample.timestamp);

  for (I32 i = 0; i < skeleton->joints.length; i += 1)
  {
    Transform transform0 = TransformArrayGet(&current_sample.local_joint_transforms, i);
    Transform transform1 = TransformArrayGet(&next_sample.local_joint_transforms, i);

    Joint* target_joint = JointArrayGetPointer(&skeleton->joints, i);
    target_joint->local_transform.translation = LerpVec3F32(transform0.translation, transform1.translation, blend_value);
    target_joint->local_transform.rotation = SlerpQuaternion(transform0.rotation, transform1.rotation, blend_value);
    if (i == 0)
    {
    #if 0
      LOG_DEBUG("Transform 0\t: %.2fx %.2fy %.2fz\n", transform0.translation.x, transform0.translation.y, transform0.translation.z);
      LOG_DEBUG("Animation Time\t%f\n", animation_time/1000.0f);
      LOG_DEBUG("Current Time\t%f\n", current_sample.timestamp/1000.0f);
      LOG_DEBUG("Next Time\t%f\n", next_sample.timestamp/1000.0f);
      LOG_DEBUG("Blend value\t%f\n", blend_value);
      LOG_DEBUG("CurrentTransform 0\t: %.2fx %.2fy %.2fz\n", target_joint->local_transform.translation.x, target_joint->local_transform.translation.y, target_joint->local_transform.translation.z);
      LOG_DEBUG("Transform 1\t: %.2fx %.2fy %.2fz\n", transform1.translation.x, transform1.translation.y, transform1.translation.z);
    #endif
    }

  }

  UpdateSkeletonGlobalTransform(skeleton);
}

func void
DrawSkeleton(R_CommandBuffer command_buffer, R_Buffer buffer, Skeleton* skeleton)
{
#if 0
  Vec4F32 colors[] = {
    MakeVec4F32(1.0f, 0.0f, 0.0f, 1.0f),
    MakeVec4F32(0.0f, 1.0f, 0.0f, 1.0f),
    MakeVec4F32(0.0f, 0.0f, 1.0f, 1.0f),
    MakeVec4F32(1.0f, 1.0f, 0.0f, 1.0f),
    MakeVec4F32(1.0f, 0.0f, 1.0f, 1.0f),
    MakeVec4F32(0.0f, 1.0f, 1.0f, 1.0f),
  };
#else
  Vec4F32 colors[] = {
    MakeVec4F32(1.0f, 0.0f, 1.0f, 1.0f),
  };
#endif

  for (I32 i = 0; i < skeleton->joints.length; i += 1)
  {
    Joint joint1 = JointArrayGet(&skeleton->joints, i);

    if (joint1.parent_id != JointID_Nil)
    {
      Joint joint2 = JointArrayGet(&skeleton->joints, joint1.parent_id);

      Vec3F32 start = joint1.global_transform.translation;
      Vec3F32 end = joint2.global_transform.translation;

      DrawLine3D(command_buffer, buffer, start, end, colors[i%CountArrayElements(colors)], 0.008f);
    }

    DrawLine3D(
      command_buffer, buffer,joint1.global_transform.translation,
      AddVec3F32(joint1.global_transform.translation, RotateVec3F32(MakeVec3F32(0.0f, 0.1f, 0.0f), joint1.global_transform.rotation)),
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

  if (OS_IsKeyPressed(OS_KEY_U))
  {
    for (I32 i = 0; i < state->skeleton_animations.length; i += 1)
    {
      LOG_DEBUG("Start Animations\n");
      StartAnimation(SkeletonAnimationArrayGetPointer(&state->skeleton_animations, i), OS_GetTimeTicks());
    }

    state->animation.start_timestamp = OS_GetTimeTicks();
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
