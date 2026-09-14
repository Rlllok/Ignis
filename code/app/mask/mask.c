#include "base/base_include.h"
#include "os/os_include.h"
#include "rhi/rhi_include.h"
#include "draw/draw.h"

#include "base/base_include.c"
#include "os/os_include.c"
#include "rhi/rhi_include.c"
#include "draw/draw.c"

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb/stb_image.h"

// -- Sprite ---------------------------------------------------------
typedef U64 SpriteId;

typedef struct Sprite Sprite;
struct Sprite {
  SpriteId            id;
  RHI_Texture         rhi_texture;
  RHI_TextureDeviceId rhi_texture_device_id;
};
Sprite sprite_nil = ZeroStruct();
DefineArray(Sprite, SpriteArray, sprite_nil);

typedef struct SpriteAnimation SpriteAnimation;
struct SpriteAnimation {
  SpriteId id ;
  I32      frames_count;
  I32      current_frame;
  F32      frame_duration;
  F32      animation_duration;
  Vec2F32  sprite_offset;
};

// -- Entities -------------------------------------------------------
typedef U64 EntityId;

typedef U32 EntityFlag;
enum {
  EntityFlag_Drawable = 1 << 1,
  EntityFlag_Collision = 1 << 2,
};

typedef struct Entity Entity;
struct Entity {
  EntityId id;
  Vec2F32  position;

  EntityFlag flags;
  union {
    struct {
      Sprite* sprite;
      RectF32 scissor;
    } sprite_static;

    SpriteAnimation sprite_animation;
  };
};
Entity entity_nil = ZeroStruct();
DefineArray(Entity, EntityArray, entity_nil);

// -- Level ----------------------------------------------------------
typedef struct Level Level;
struct Level {
  Arena* arena;

  // Assets
  SpriteArray sprites;

  // Entities
  EntityArray entities;
  EntityId    player;
};

typedef struct EntityCreateInfo EntityCreateInfo;
struct EntityCreateInfo {
  EntityFlag flags;
  Vec2F32    position;
  SpriteId   sprite_id;
  RectF32    sprite_scissor;
};

func SpriteId AddSprite(Level* level, Str8 sprite_path);
func EntityId AddEntity(Level* level, EntityCreateInfo* entity_info);

func B32 PrepareLevelAssets(Level* level);

func void UpdateLevelState(Level* level, F32 dt);

// -- Game Context ---------------------------------------------------
struct {
  Arena* global_arena;
  Arena* frame_arena;
  OS_Window* window;

  // RHI
  RHI_CommandBuffer   rhi_command_buffer;
  RHI_Buffer          rhi_transfer_buffer;
  RHI_Buffer          rhi_frame_buffer;
  RHI_ResourceTable   rhi_resource_table;
  RHI_Texture         rhi_offscreen_texture;
  RHI_TextureDeviceId rhi_offscreen_texture_id;
  RHI_TextureSampler  rhi_default_sampler;

  // states
  B32 finished;

  // Game's data
  Level level;
} mask_context;

I32 main() {
  // Init
  mask_context.global_arena = AllocateArena(Gigabytes(2), Kilobytes(4));
  mask_context.frame_arena = AllocateArena(Gigabytes(2), Kilobytes(4));
  
  OS_Init(Megabytes(16));
  mask_context.window = OS_CreateWindow(Str8C("MaskGame"), MakeVec2U32(1280, 720));
  OS_ShowWindow(mask_context.window);

  RHI_Init(mask_context.window);
  mask_context.rhi_command_buffer = RHI_GetCommandBuffer();
  mask_context.rhi_frame_buffer = RHI_CreateBuffer(Str8C("Mask_FrameBuffer"), Megabytes(16), RHI_BufferUsageFlag_Storage|RHI_BufferUsageFlag_Address, RHI_BufferPropertyFlag_HostVisible|RHI_BufferPropertyFlag_HostCoherent);
  mask_context.rhi_transfer_buffer = RHI_CreateBuffer(Str8C("Mask_TransferBuffer"), Megabytes(128), RHI_BufferUsageFlag_Transfer, RHI_BufferPropertyFlag_HostCoherent);
  mask_context.rhi_resource_table = RHI_CreateResourceTable(mask_context.global_arena, 256, 1);
  
  mask_context.rhi_offscreen_texture = RHI_CreateTexture(&(RHI_TextureCreateInfo) {
    .kind = RHI_TextureKind_2D,
    .format = RHI_TextureFormat_B8G8R8A8_UNORM,
    .usage_flags = RHI_TEXTURE_USAGE_FLAG_COLOR_ATTACHMENT,
    .width = 320,
    .height = 180,
    .depth = 1,
    .num_levels = 1,
  });
  mask_context.rhi_offscreen_texture_id = RHI_ResourceTableAddTexture(mask_context.rhi_resource_table, mask_context.rhi_offscreen_texture);

  D_Init(Megabytes(16));

  // Load level
  mask_context.level = (Level)ZeroStruct();
  mask_context.level.arena = AllocateArena(Megabytes(256), Megabytes(4));
  mask_context.level.sprites = SpriteArrayAllocate(mask_context.global_arena, 8);
  mask_context.level.entities = EntityArrayAllocate(mask_context.global_arena, 128);
  SpriteId player_sprite = AddSprite(&mask_context.level, Str8C("data/mask/images/character.png"));
  mask_context.level.player = AddEntity(&mask_context.level, &(EntityCreateInfo){
    .flags = EntityFlag_Drawable,
    .position = MakeVec2F32(50.0f, 50.0f),
    .sprite_id = player_sprite,
    .sprite_scissor = MakeRectF32(0.0f, 0.0f, 16.0f, 22.0f),
  });

  PrepareLevelAssets(&mask_context.level);
  
  U64 start_ts = OS_GetTimeTicks();
  F32 dt = 0.0f;
  while (!mask_context.finished) {
    OS_DispatchEvents(mask_context.frame_arena, mask_context.window);
    
    if (OS_KeyPressed(OS_KEY_ESC)) {
      mask_context.finished = 1;
    }

    UpdateLevelState(&mask_context.level, dt);

    RHI_BeginCommandBuffer(mask_context.rhi_command_buffer); {
      // Render offscreen texture
      {
        RHI_ColorTarget color_targets[] = {
          {
            .texture = mask_context.rhi_offscreen_texture,
            .load_operation = RHI_AttachmentLoadOperation_Clear,
            .store_operation = RHI_AttachmentStoreOperation_Store,
            .clear_color = RGBAFromHex(0xffffffff),
          }
        };
        RHI_RenderPass* render_pass = RHI_BeginRenderPass(mask_context.rhi_command_buffer, ArrayLength(color_targets), color_targets, 0, 0, 0); {
          RHI_SetViewport(mask_context.rhi_command_buffer, (RectI32){.x = 0, .y = 0, .w = 320, .h = 180});
          RHI_SetScissor(mask_context.rhi_command_buffer, (RectI32){.x = 0, .y = 0, .w = 320, .h = 180});
          RHI_BindResourceTable(mask_context.rhi_command_buffer, mask_context.rhi_resource_table);
          for (I32 entity_index = 0; entity_index < mask_context.level.entities.length; entity_index += 1) {
            Entity* entity = EntityArrayGetPointer(&mask_context.level.entities, entity_index);
            if ((entity->flags & EntityFlag_Drawable) == EntityFlag_Drawable) {
              RectF32 rect = {
                .position = entity->position,
                .size = entity->sprite_static.scissor.size,
              };
              D_DrawSpritePart(mask_context.rhi_command_buffer, mask_context.rhi_frame_buffer, rect, entity->sprite_static.sprite->rhi_texture_device_id, entity->sprite_static.scissor);
            }
          }
        }
        RHI_EndRenderPass(mask_context.rhi_command_buffer, render_pass);
      }

      // Render to swapchain texture
      {
        RHI_Texture swapchain_texture = RHI_AcquireSwapchainTexture(mask_context.rhi_command_buffer);
        RHI_ColorTarget color_targets[] = {
          {
            .texture = swapchain_texture,
            .load_operation = RHI_AttachmentLoadOperation_Clear,
            .store_operation = RHI_AttachmentStoreOperation_Store,
            .clear_color = RGBAFromHex(0xffffffff),
          }
        };
        RHI_RenderPass* render_pass = RHI_BeginRenderPass(mask_context.rhi_command_buffer, ArrayLength(color_targets), color_targets, 0, 0, 0); {
          RHI_SetViewport(mask_context.rhi_command_buffer, (RectI32){.x = 0, .y = 0, .w = mask_context.window->size.x, .h = mask_context.window->size.h});
          RHI_SetScissor(mask_context.rhi_command_buffer, (RectI32){.x = 0, .y = 0, .w = mask_context.window->size.x, .h = mask_context.window->size.h});
          RHI_BindResourceTable(mask_context.rhi_command_buffer, mask_context.rhi_resource_table);
          RectF32 viewport = {
            .x = 0.0f,
            .y = 0.0f,
            .w = 1280.0f,
            .h = 720.0f,
          };
          RectF32 rect = {
            .x = 0.0f,
            .y = 0.0f,
            .w = 320.0f,
            .h = 180.0f,
          };
          D_DrawSpritePart(mask_context.rhi_command_buffer, mask_context.rhi_frame_buffer, viewport, mask_context.rhi_offscreen_texture_id, rect);
        }
        RHI_EndRenderPass(mask_context.rhi_command_buffer, render_pass);
      }
      
      RHI_Present(mask_context.rhi_command_buffer);
    }
    RHI_EndCommandBuffer(mask_context.rhi_command_buffer);
    RHI_SubmitCommandBuffer(mask_context.rhi_command_buffer, 0, 0, 0, 0);

    U64 end_ts = OS_GetTimeTicks();
    U64 dt_ms = end_ts - start_ts;
    dt = (F32)(dt_ms)*0.001f;
    start_ts = end_ts;

    F32 target_frame_time = 1000.0f/60.0f;
    F32 time_to_sleep = target_frame_time - dt_ms;
    if (time_to_sleep > 0.0f) {
      OS_Sleep(time_to_sleep);
    }
  }
}

// -- Level ----------------------------------------------------------
func SpriteId
AddSprite(Level* level, Str8 sprite_path) {
  I32 tex_width = 0;
  I32 tex_height = 0;
  I32 tex_channels = 0;
  U8* tex_pixels = stbi_load(CFromStr8(sprite_path), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);

  RHI_Texture rhi_texture = RHI_CreateTexture(&(RHI_TextureCreateInfo) {
    .kind = RHI_TextureKind_2D,
    .format = RHI_TextureFormat_R8G8B8A8_UNORM,
    .usage_flags = RHI_TEXTURE_USAGE_FLAG_SAMPLED | RHI_TEXTURE_USAGE_FLAG_TRANSFER_DST,
    .width = tex_width,
    .height = tex_height,
    .depth = 1,
    .num_levels = 1,
  });
  U64 texture_offset = RHI_PushBuffer(mask_context.rhi_transfer_buffer, tex_pixels, tex_width*tex_height*4);
  RHI_CommandBuffer transfer_command_buffer = ZeroStruct();
  RHI_BeginCommandBuffer(transfer_command_buffer); {
    RHI_CopyBufferToTexture(transfer_command_buffer, mask_context.rhi_transfer_buffer, 0, rhi_texture);
  }
  RHI_EndCommandBuffer(transfer_command_buffer);

  RHI_Semaphore copy_semaphore = RHI_CreateSemaphore();
  RHI_SemaphoreSignalInfo signal_semaphore = {
    .semaphore = copy_semaphore,
    .value = 1,
  };
  RHI_SubmitCommandBuffer(transfer_command_buffer, 0, 0, &signal_semaphore, 1);
  RHI_WaitSemaphore(copy_semaphore, 1);
  RHI_DestroySemaphore(copy_semaphore);

  // --AlNov: @TODO I don't like api
  U64 sprite_array_index = SpriteArrayAdd(&level->sprites, (Sprite)ZeroStruct());
  Sprite* sprite = SpriteArrayGetPointer(&level->sprites, sprite_array_index);
  sprite->id = sprite_array_index;
  sprite->rhi_texture = rhi_texture;
  return sprite_array_index;
}

func EntityId
AddEntity(Level* level, EntityCreateInfo* entity_info) {
  // --AlNov: @TODO Doesn't handle failure.
  U64 entity_array_index = EntityArrayAdd(&level->entities, (Entity)ZeroStruct());
  Entity* entity = EntityArrayGetPointer(&level->entities, entity_array_index);
  entity->id = entity_array_index;
  entity->position = entity_info->position;
  entity->flags = entity_info->flags;
  entity->sprite_static.sprite = SpriteArrayGetPointer(&level->sprites, entity_info->sprite_id);
  entity->sprite_static.scissor = entity_info->sprite_scissor;
  return entity_array_index;
}

func B32
PrepareLevelAssets(Level* level) {
  mask_context.rhi_default_sampler = RHI_CreateTextureSampler(
    &(RHI_TextureSamplerCreateInfo) {
      .mag_filter = RHI_FilterKind_Linear,
      .min_filter = RHI_FilterKind_Linear,
      .address_mode_u = RHI_SamplerAddressMode_Repeat,
      .address_mode_v = RHI_SamplerAddressMode_Repeat,
      .address_mode_w = RHI_SamplerAddressMode_Repeat,
      .mipmap_mode = RHI_SamplerMipmapMode_Linear,
    }
  );
  RHI_ResourceTableAddSampler(mask_context.rhi_resource_table, mask_context.rhi_default_sampler);
  for (I32 i = 0; i < level->sprites.length; i += 1) {
    Sprite* sprite = SpriteArrayGetPointer(&level->sprites, i);
    sprite->rhi_texture_device_id = RHI_ResourceTableAddTexture(mask_context.rhi_resource_table, sprite->rhi_texture);
    sprite->rhi_texture_device_id = RHI_ResourceTableAddTexture(mask_context.rhi_resource_table, sprite->rhi_texture);
    sprite->rhi_texture_device_id = RHI_ResourceTableAddTexture(mask_context.rhi_resource_table, sprite->rhi_texture);
  }

  return 1;
}

func void
UpdateLevelState(Level* level, F32 dt) {
  Entity* player = EntityArrayGetPointer(&level->entities, level->player);
  Vec2F32 direction = MakeVec2F32(0.0f, 0.0f);
  if (OS_KeyDown(OS_KEY_W)) { direction.y -= 1.0f; }
  if (OS_KeyDown(OS_KEY_S)) { direction.y += 1.0f; }
  if (OS_KeyDown(OS_KEY_D)) { direction.x += 1.0f; }
  if (OS_KeyDown(OS_KEY_A)) { direction.x -= 1.0f; }
  Vec2F32 velocity = ScaleVec2F32(NormalizeVec2F32(direction), 150.0f);
  player->position = AddVec2F32(player->position, ScaleVec2F32(velocity, dt));
}
