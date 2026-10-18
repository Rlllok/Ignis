#include "base/base_include.h"
#include "os/os_include.h"
#include "rhi/rhi_include.h"
#include "draw/draw.h"

#include "base/base_include.c"
#include "os/os_include.c"
#include "rhi/rhi_include.c"
#include "draw/draw.c"

#define ASTR_WIDTH 480
#define ASTR_HEIGHT 270

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb/stb_image.h"

// -- SpaceArena -----------------------------------------------------
typedef struct ASTR_SpaceArena ASTR_SpaceArena;
struct ASTR_SpaceArena {
  Vec2F32 position;
  F32     radius;
  RectF32 sprite_scissor;
};

func void ASTR_DrawSpaceArena(RHI_CommandBuffer rhi_command_buffer, ASTR_SpaceArena* space_arena);

// -- Asteroids ------------------------------------------------------
typedef struct ASTR_Asteroid ASTR_Asteroid;
struct ASTR_Asteroid {
  Vec2F32 position;
  F32     rotation;
  F32     rotation_speed;
  B32     cw_rotation;
  Vec2F32 velocity;
  F32     collision_radius;
  RectF32 sprite_scissor;

  ASTR_Asteroid* next_active;
  ASTR_Asteroid* prev_active;
  ASTR_Asteroid* next_free;
};

typedef struct ASTR_AsteroidPool ASTR_AsteroidPool;
struct ASTR_AsteroidPool {
  ASTR_Asteroid* asteroids;
  U32            capacity;
  U32            count;

  ASTR_Asteroid* first_active;
  ASTR_Asteroid* last_active;
  ASTR_Asteroid* free_list;
};

func ASTR_AsteroidPool ASTR_AllocateAsteroidPool(Arena* arena, U32 capacity);
func void ASTR_SpawnAsteroid(ASTR_AsteroidPool* pool, Vec2F32 position, F32 rotation, F32 rotation_speed, Vec2F32 velocity);
func void ASTR_DespawnAsteroid(ASTR_AsteroidPool* pool, ASTR_Asteroid* asteroid);
func void ASTR_UpdateAsteroids(ASTR_AsteroidPool* pool, ASTR_SpaceArena* space_arena, F32 dt);
func void ASTR_DrawAsteroids(RHI_CommandBuffer rhi_command_buffer, ASTR_AsteroidPool* pool);

// -- Bullets --------------------------------------------------------
typedef struct ASTR_Bullet ASTR_Bullet;
struct ASTR_Bullet {
  Vec2F32 position;
  F32     rotation;
  Vec2F32 velocity;
  F32     collision_radius;
  RectF32 sprite_scissor;

  ASTR_Bullet* next_active;
  ASTR_Bullet* prev_active;
  ASTR_Bullet* next_free;
};

typedef struct ASTR_BulletPool ASTR_BulletPool;
struct ASTR_BulletPool {
  ASTR_Bullet* bullets;
  U32          capacity;
  U32          count;
  ASTR_Bullet* first_active;
  ASTR_Bullet* last_active;
  ASTR_Bullet* free_list;
};

func ASTR_BulletPool ASTR_AllocateBulletPool(Arena* arena, U32 capacity);
func void ASTR_SpawnBullet(ASTR_BulletPool* pool, Vec2F32 position, F32 rotation);
func void ASTR_DespawnBullet(ASTR_BulletPool* pool, ASTR_Bullet* bullet);
func void ASTR_UpdateBullets(ASTR_BulletPool* pool, ASTR_SpaceArena* space_arena, ASTR_AsteroidPool* asteroid_pool, F32 dt);
func void ASTR_DrawBullets(RHI_CommandBuffer rhi_command_buffer, ASTR_BulletPool* pool);

// -- SpaceShip ------------------------------------------------------
typedef struct ASTR_SpaceShip ASTR_SpaceShip;
struct ASTR_SpaceShip {
  Vec2F32 position;
  F32     rotation;
  F32     speed;
  F32     collision_radius;
  B32     destroyed;
  RectF32 sprite_scissor;
};

func void ASTR_UpdateSpaceShip(ASTR_SpaceArena* space_arena, ASTR_SpaceShip* space_ship, ASTR_AsteroidPool* asteroid_pool, ASTR_BulletPool* bullet_pool, F32 dt);
func void ASTR_DrawSpaceShip(RHI_CommandBuffer rhi_command_buffer, ASTR_SpaceShip* space_ship);

// -- Global Context -------------------------------------------------
struct {
  Arena* global_arena;
  Arena* frame_arena;

  OS_Window* window;

  B32 finished;
  B32 debug_mode;

  // RHI
  RHI_CommandBuffer   rhi_command_buffer;
  RHI_Buffer          rhi_frame_buffer;
  RHI_Buffer          rhi_transfer_buffer;
  RHI_ResourceTable   rhi_resource_table;
  RHI_TextureSampler  rhi_default_sampler;
  RHI_SamplerDeviceId rhi_default_sampler_id;
  RHI_Texture         rhi_offscreen_texture;
  RHI_TextureView     rhi_offscreen_texture_view;

  // Assets
  RHI_Texture     sprite_atlass;
  RHI_TextureView sprite_atlass_view;

  // Game Objects
  ASTR_SpaceArena   space_arena;
  ASTR_SpaceShip    space_ship;
  ASTR_AsteroidPool asteroid_pool;
  ASTR_BulletPool   bullet_pool;
} astr_context;

I32 main() {
  astr_context.global_arena = AllocateArena(Gigabytes(1), Kilobytes(16));
  astr_context.frame_arena = AllocateArena(Gigabytes(1), Kilobytes(16));

  OS_Init(Megabytes(16));
  astr_context.window = OS_CreateWindow(Str8C("Asteroids"), MakeVec2U32(1280, 720));
  OS_ShowWindow(astr_context.window);

  RHI_Init(astr_context.window);
  astr_context.rhi_command_buffer = RHI_GetCommandBuffer();
  astr_context.rhi_frame_buffer = RHI_CreateBuffer(Str8C("ASTR_FrameBuffer"), Megabytes(8), RHI_BufferUsageFlag_Storage|RHI_BufferUsageFlag_Address, RHI_BufferPropertyFlag_HostCoherent|RHI_BufferPropertyFlag_HostVisible);
  astr_context.rhi_transfer_buffer = RHI_CreateBuffer(Str8C("ASTR_TransferBuffer"), Megabytes(128), RHI_BufferUsageFlag_Transfer, RHI_BufferPropertyFlag_HostCoherent|RHI_BufferPropertyFlag_HostVisible);
  astr_context.rhi_resource_table = RHI_CreateResourceTable(astr_context.global_arena, 256, 1);

  astr_context.rhi_default_sampler = RHI_CreateTextureSampler(
    &(RHI_TextureSamplerCreateInfo){
      .mag_filter = RHI_FilterKind_Linear,
      .min_filter = RHI_FilterKind_Linear,
      .address_mode_u = RHI_SamplerAddressMode_Repeat,
      .address_mode_v = RHI_SamplerAddressMode_Repeat,
      .address_mode_w = RHI_SamplerAddressMode_Repeat,
      .mipmap_mode = RHI_SamplerMipmapMode_Linear,
    }
  );
  astr_context.rhi_default_sampler_id = RHI_ResourceTableAddSampler(astr_context.rhi_resource_table, astr_context.rhi_default_sampler_id);

  astr_context.rhi_offscreen_texture = RHI_CreateTexture(Str8C("Offscreen_Texture"), &(RHI_TextureCreateInfo) {
    .kind = RHI_TextureKind_2D,
    .format = RHI_TextureFormat_B8G8R8A8_UNORM,
    .usage_flags = RHI_TEXTURE_USAGE_FLAG_COLOR_ATTACHMENT,
    .width = ASTR_WIDTH,
    .height = ASTR_HEIGHT,
    .depth = 1,
    .num_levels = 1,
  });
  astr_context.rhi_offscreen_texture_view = RHI_ResourceTableAddTexture(astr_context.rhi_resource_table, (RHI_TextureViewInfo){
    .texture = astr_context.rhi_offscreen_texture,
    .kind = RHI_TextureKind_2D,
    .format = RHI_TextureFormat_B8G8R8A8_UNORM,
  });

  I32 tex_width = 0;
  I32 tex_height = 0;
  I32 tex_channels = 0;
  U8* tex_pixels = stbi_load("data/asteroids/Asteroids.png", &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);

  astr_context.sprite_atlass = RHI_CreateTexture(Str8C("Sprite_Atlass"), &(RHI_TextureCreateInfo) {
    .kind = RHI_TextureKind_2D,
    .format = RHI_TextureFormat_R8G8B8A8_UNORM,
    .usage_flags = RHI_TEXTURE_USAGE_FLAG_SAMPLED | RHI_TEXTURE_USAGE_FLAG_TRANSFER_DST,
    .width = tex_width,
    .height = tex_height,
    .depth = 1,
    .num_levels = 1,
  });
  U64 texture_offset = RHI_PushBuffer(astr_context.rhi_transfer_buffer, tex_pixels, tex_width*tex_height*4);
  RHI_CommandBuffer transfer_command_buffer = ZeroStruct();
  RHI_BeginCommandBuffer(transfer_command_buffer); {
    RHI_CopyBufferToTexture(transfer_command_buffer, astr_context.rhi_transfer_buffer, 0, astr_context.sprite_atlass);
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
  astr_context.sprite_atlass_view = RHI_ResourceTableAddTexture(astr_context.rhi_resource_table, (RHI_TextureViewInfo){
    .texture = astr_context.sprite_atlass,
    .kind = RHI_TextureKind_2D,
    .format = RHI_TextureFormat_R8G8B8A8_UNORM,
  });

  // Add Game Objects
  astr_context.space_arena = (ASTR_SpaceArena){
    .position = MakeVec2F32(ASTR_WIDTH/2.0f, ASTR_HEIGHT/2.0f),
    .radius = 123.0f,
    .sprite_scissor = MakeRectF32(0.0f, 0.0f, 246.0f, 246.0f),
  };

  astr_context.space_ship = (ASTR_SpaceShip){
    .position = MakeVec2F32(ASTR_WIDTH/2.0f, ASTR_HEIGHT/2.0f),
    .speed = 70.0f,
    .collision_radius = 8.0f,
    .sprite_scissor = MakeRectF32(256.0f, 0.0f, 16.0f, 16.0f),
  };

  astr_context.asteroid_pool = ASTR_AllocateAsteroidPool(astr_context.global_arena, 32);
  ASTR_SpawnAsteroid(&astr_context.asteroid_pool, MakeVec2F32(ASTR_WIDTH*0.5f - 60.0f, ASTR_HEIGHT*0.5f - 60.0f), 0.0f, 2.0f, MakeVec2F32(10.0f, 10.0f));
  ASTR_SpawnAsteroid(&astr_context.asteroid_pool, MakeVec2F32(ASTR_WIDTH*0.5f + 40.0f, ASTR_HEIGHT*0.5f - 60.0f), 0.0f, 2.0f, MakeVec2F32(10.0f, -10.0f));
  ASTR_SpawnAsteroid(&astr_context.asteroid_pool, MakeVec2F32(ASTR_WIDTH*0.5f - 20.0f, ASTR_HEIGHT*0.5f + 10.0f), 0.0f, 2.0f, MakeVec2F32(20.0f, 10.0f));

  astr_context.bullet_pool = ASTR_AllocateBulletPool(astr_context.global_arena, 64);

  D_Init(Megabytes(16));

  
  F32 target_frame_ms = 1000.0f/60.0f;
  U64 start_ts = OS_GetTimeTicks();
  F32 dt = 0.0f;
  while (!astr_context.finished) {
    OS_DispatchEvents(astr_context.frame_arena, astr_context.window);

    if (OS_KeyPressed(OS_KEY_ESC)) { astr_context.finished = 1; }
    if (OS_KeyPressed(OS_KEY_F2))  { astr_context.debug_mode = !astr_context.debug_mode; }

    ASTR_UpdateAsteroids(&astr_context.asteroid_pool, &astr_context.space_arena, dt);
    ASTR_UpdateBullets(&astr_context.bullet_pool, &astr_context.space_arena, &astr_context.asteroid_pool, dt);
    if (!astr_context.space_ship.destroyed) {
      ASTR_UpdateSpaceShip(&astr_context.space_arena, &astr_context.space_ship, &astr_context.asteroid_pool, &astr_context.bullet_pool, dt);
    }

    RHI_BeginCommandBuffer(astr_context.rhi_command_buffer); {
      // Render scene to offscreen texture
      {
        RHI_ColorTarget color_targets[] = {
          {
            .texture = astr_context.rhi_offscreen_texture,
            .load_operation = RHI_AttachmentLoadOperation_Clear,
            .store_operation = RHI_AttachmentStoreOperation_Store,
            .clear_color = RGBAFromHex(0x00000000),
          }
        };
        RHI_RenderPassInfo render_pass_info = {
          .render_area = MakeRectI32(0, 0, ASTR_WIDTH, ASTR_HEIGHT),
          .color_targets = color_targets,
          .color_targets_count = ArrayLength(color_targets),
          .depth_target = 0,
        };
        RHI_BeginRenderPass(astr_context.rhi_command_buffer, render_pass_info); {
          RHI_BindResourceTable(astr_context.rhi_command_buffer, astr_context.rhi_resource_table);
          RectF32 draw_area = MakeRectF32(0.0f, 0.0f, 50.0f, 50.0f);
          ASTR_DrawSpaceArena(astr_context.rhi_command_buffer, &astr_context.space_arena);
          if (!astr_context.space_ship.destroyed) {
            ASTR_DrawSpaceShip(astr_context.rhi_command_buffer, &astr_context.space_ship);
          }
          ASTR_DrawAsteroids(astr_context.rhi_command_buffer, &astr_context.asteroid_pool);
          ASTR_DrawBullets(astr_context.rhi_command_buffer, &astr_context.bullet_pool);
        }
        RHI_EndRenderPass(astr_context.rhi_command_buffer);
      }

      // Render offscreen texture to swapchain
      {
        RHI_Texture swapchain_texture = RHI_AcquireSwapchainTexture(astr_context.rhi_command_buffer);
        RHI_ColorTarget color_targets[] = {
          {
            .texture = swapchain_texture,
            .load_operation = RHI_AttachmentLoadOperation_Clear,
            .store_operation = RHI_AttachmentStoreOperation_Store,
            .clear_color = RGBAFromHex(0xfbf0d9ff),
          }
        };
        RHI_RenderPassInfo render_pass_info = {
          .render_area = MakeRectI32(0, 0, swapchain_texture.size.x, swapchain_texture.size.y),
          .color_targets = color_targets,
          .color_targets_count = ArrayLength(color_targets),
          .depth_target = 0,
        };
        RHI_BeginRenderPass(astr_context.rhi_command_buffer, render_pass_info); {
          RHI_BindResourceTable(astr_context.rhi_command_buffer, astr_context.rhi_resource_table);
          RectF32 viewport_f32 = { .x = 0, .y = 0, .w = astr_context.window->size.x, .h = astr_context.window->size.y };
          RectF32 rect = {
            .x = 0.0f,
            .y = 0.0f,
            .w = ASTR_WIDTH,
            .h = ASTR_HEIGHT,
          };
          D_DrawSpritePart(astr_context.rhi_command_buffer, astr_context.rhi_frame_buffer, viewport_f32, 0.0f, astr_context.rhi_offscreen_texture_view, rect, MakeVec4F32(1.0f, 1.0f, 1.0f, 1.0f));
        }
        RHI_EndRenderPass(astr_context.rhi_command_buffer);
      }

      RHI_Present(astr_context.rhi_command_buffer);
    }
    RHI_EndCommandBuffer(astr_context.rhi_command_buffer);
    RHI_SubmitCommandBuffer(astr_context.rhi_command_buffer, 0, 0, 0, 0);

    ResetArena(astr_context.frame_arena);

    U64 end_ts = OS_GetTimeTicks();
    U64 dt_ms = end_ts - start_ts;
    dt = (F32)(dt_ms)*0.001f;
    start_ts = end_ts;

    F32 time_to_sleep_ms = target_frame_ms - dt_ms;
    if (time_to_sleep_ms > 0.0f) {
      OS_Sleep(time_to_sleep_ms);
    }
  }

  return 0;
}

// -- SpaceArena -----------------------------------------------------
func void
ASTR_DrawSpaceArena(RHI_CommandBuffer rhi_command_buffer, ASTR_SpaceArena* space_arena) {
  RectF32 draw_area = MakeRectF32(space_arena->position.x - space_arena->sprite_scissor.w/2.0f, space_arena->position.y - space_arena->sprite_scissor.h/2.0f, space_arena->sprite_scissor.w, space_arena->sprite_scissor.h);
  D_DrawSpritePart(astr_context.rhi_command_buffer, astr_context.rhi_frame_buffer, draw_area, 0.0f, astr_context.sprite_atlass_view, space_arena->sprite_scissor, RGBAFromHex(0x5f4b32ff));
  if (astr_context.debug_mode) {
    F32 r = space_arena->radius;
    RectF32 collision_rect = MakeRectF32(space_arena->position.x - r, space_arena->position.y - r, r*2.0f, r*2.0f);
    D_DrawRectWithBorder(rhi_command_buffer, astr_context.rhi_frame_buffer, collision_rect, MakeVec4F32(r, r, r, r), MakeVec4F32(0.0f, 0.0f, 0.0f, 0.0f), 1.0f, MakeVec4F32(1.0f, 0.0f, 0.0f, 0.8f));
  }
}

// -- Asteroids ------------------------------------------------------
func ASTR_AsteroidPool
ASTR_AllocateAsteroidPool(Arena* arena, U32 capacity) {
  ASTR_AsteroidPool result = ZeroStruct();
  result.asteroids = (ASTR_Asteroid*)PushArena(arena, sizeof(ASTR_Asteroid)*capacity);
  result.capacity = capacity;
  return result;
}

func void
ASTR_SpawnAsteroid(ASTR_AsteroidPool* pool, Vec2F32 position, F32 rotation, F32 rotation_speed, Vec2F32 velocity) {
  ASTR_Asteroid* asteroid = 0;

  if (pool->free_list == 0) {
    if (pool->count < pool->capacity) {
      asteroid = pool->asteroids + pool->count;
      pool->count += 1;
    }
  }
  else {
    asteroid = pool->free_list;
    StackPop_Next(pool->free_list, next_free);
  }
  
  if (asteroid != 0) {
    *asteroid = (ASTR_Asteroid)ZeroStruct();
    asteroid->position = position;
    asteroid->rotation = rotation;
    asteroid->rotation_speed = rotation_speed;
    asteroid->velocity = velocity;
    asteroid->collision_radius = 8;
    asteroid->sprite_scissor = MakeRectF32(256.0f, 48.0f, 16.0f, 16.0f);
    DllPushBack_NextPrev(pool->first_active, pool->last_active, asteroid, next_active, prev_active);
  }
}

func void
ASTR_DespawnAsteroid(ASTR_AsteroidPool* pool, ASTR_Asteroid* asteroid) {
  if (asteroid) {
    DllRemove_NextPrev(pool->first_active, pool->last_active, asteroid, next_active, prev_active);
    StackPush_Next(pool->free_list, asteroid, next_free);
  }
}

func void
ASTR_UpdateAsteroids(ASTR_AsteroidPool* pool, ASTR_SpaceArena* space_arena, F32 dt) {
  for (ASTR_Asteroid* asteroid = pool->first_active; asteroid != 0; asteroid = asteroid->next_active) {
    asteroid->position = AddVec2F32(asteroid->position, ScaleVec2F32(asteroid->velocity, dt));
    asteroid->rotation += (1.0f - asteroid->cw_rotation*2.0f)*(asteroid->rotation_speed*dt);

    Vec2F32 asteroid_to_arena_position = SubVec2F32(asteroid->position, space_arena->position);
    F32 asteroid_distance_to_edge = space_arena->radius - MagnitudeVec2F32(asteroid_to_arena_position) - asteroid->collision_radius;
    if (asteroid_distance_to_edge < 0.0f) {
      asteroid->velocity = ScaleVec2F32(asteroid->velocity, -1.0f);
      asteroid->cw_rotation = !asteroid->cw_rotation;
    }

    for (ASTR_Asteroid* other_asteroid = asteroid->next_active; other_asteroid != 0; other_asteroid = other_asteroid->next_active) {
      F32 distance = MagnitudeVec2F32(SubVec2F32(asteroid->position, other_asteroid->position));
      if (distance < (asteroid->collision_radius + other_asteroid->collision_radius)) {
        asteroid->velocity = ScaleVec2F32(asteroid->velocity, -1.0f);
        other_asteroid->velocity = ScaleVec2F32(other_asteroid->velocity, -1.0f);
      }
    }
  }
}

func void
ASTR_DrawAsteroids(RHI_CommandBuffer rhi_command_buffer, ASTR_AsteroidPool* pool) {
  for (ASTR_Asteroid* asteroid = pool->first_active; asteroid != 0; asteroid = asteroid->next_active) {
    RectF32 draw_area = MakeRectF32(asteroid->position.x - asteroid->sprite_scissor.w/2.0f, asteroid->position.y - asteroid->sprite_scissor.h/2.0f, asteroid->sprite_scissor.w, asteroid->sprite_scissor.h);
    D_DrawSpritePart(astr_context.rhi_command_buffer, astr_context.rhi_frame_buffer, draw_area, asteroid->rotation, astr_context.sprite_atlass_view, asteroid->sprite_scissor, RGBAFromHex(0x5f4b32ff));
    if (astr_context.debug_mode) {
      F32 r = asteroid->collision_radius;
      RectF32 collision_rect = MakeRectF32(asteroid->position.x - r, asteroid->position.y - r, r*2.0f, r*2.0f);
      D_DrawRectWithBorder(rhi_command_buffer, astr_context.rhi_frame_buffer, collision_rect, MakeVec4F32(r, r, r, r), MakeVec4F32(0.0f, 0.0f, 0.0f, 0.0f), 1.0f, MakeVec4F32(1.0f, 0.0f, 0.0f, 0.8f));
    }
  }
}

// -- Bullets --------------------------------------------------------
func ASTR_BulletPool
ASTR_AllocateBulletPool(Arena* arena, U32 capacity) {
  ASTR_BulletPool result = ZeroStruct();
  result.bullets = (ASTR_Bullet*)PushArena(arena, sizeof(ASTR_Bullet)*capacity);
  result.capacity = capacity;
  return result;
}

func void
ASTR_SpawnBullet(ASTR_BulletPool* pool, Vec2F32 position, F32 rotation) {
  ASTR_Bullet* bullet = 0;

  if (pool->free_list == 0) {
    if (pool->count < pool->capacity) {
      bullet = pool->bullets + pool->count;
      pool->count += 1;
    }
  } else {
    bullet = pool->free_list;
    StackPop_Next(pool->free_list, next_free);
  }

  if (bullet != 0) {
    bullet->position = position;
    bullet->rotation = rotation;
    F32 rotation_cos = cosf(bullet->rotation);
    F32 rotation_sin = sinf(bullet->rotation);
    bullet->velocity = ScaleVec2F32(MakeVec2F32(-rotation_sin, -rotation_cos), 30.0f);
    bullet->collision_radius = 4;
    bullet->sprite_scissor = MakeRectF32(256.0f, 16.0f, 8.0f, 8.0f);
    bullet->next_active = 0;
    bullet->next_free = 0;
    DllPushBack_NextPrev(pool->first_active, pool->last_active, bullet, next_active, prev_active);
  }
}

func void
ASTR_DespawnBullet(ASTR_BulletPool* pool, ASTR_Bullet* bullet) {
  if (bullet != 0) {
    DllRemove_NextPrev(pool->first_active, pool->last_active, bullet, next_active, prev_active);
    StackPush_Next(pool->free_list, bullet, next_free);
  }
}

func void
ASTR_UpdateBullets(ASTR_BulletPool* pool, ASTR_SpaceArena* space_arena, ASTR_AsteroidPool* asteroid_pool, F32 dt) {
  for (ASTR_Bullet* bullet = pool->first_active; bullet != 0; bullet = bullet->next_active) {
    bullet->position = AddVec2F32(bullet->position, ScaleVec2F32(bullet->velocity, dt));
    
    Vec2F32 bullet_to_arena_position = SubVec2F32(bullet->position, space_arena->position);
    F32 bullet_distance_to_edge = space_arena->radius - MagnitudeVec2F32(bullet_to_arena_position) - bullet->collision_radius;
    if (bullet_distance_to_edge < 0.0f) {
      bullet->velocity = ScaleVec2F32(bullet->velocity, -1.0f);
    }

    for (ASTR_Asteroid* asteroid = asteroid_pool->first_active; asteroid != 0; asteroid = asteroid->next_active) {
      F32 distance = MagnitudeVec2F32(SubVec2F32(bullet->position, asteroid->position));
      if (distance < (bullet->collision_radius + asteroid->collision_radius)) {
        ASTR_DespawnBullet(pool, bullet);
        ASTR_DespawnAsteroid(asteroid_pool, asteroid);
      }
    }
  }
}

func void
ASTR_DrawBullets(RHI_CommandBuffer rhi_command_buffer, ASTR_BulletPool* pool) {
  for (ASTR_Bullet* bullet = pool->first_active; bullet != 0; bullet = bullet->next_active) {
    RectF32 draw_area = MakeRectF32(bullet->position.x - bullet->sprite_scissor.w/2.0f, bullet->position.y - bullet->sprite_scissor.h/2.0f, bullet->sprite_scissor.w, bullet->sprite_scissor.h);
    D_DrawSpritePart(astr_context.rhi_command_buffer, astr_context.rhi_frame_buffer, draw_area, bullet->rotation, astr_context.sprite_atlass_view, bullet->sprite_scissor, RGBAFromHex(0x5f4b32ff));
    if (astr_context.debug_mode) {
      F32 r = bullet->collision_radius;
      RectF32 collision_rect = MakeRectF32(bullet->position.x - r, bullet->position.y - r, r*2.0f, r*2.0f);
      D_DrawRectWithBorder(rhi_command_buffer, astr_context.rhi_frame_buffer, collision_rect, MakeVec4F32(r, r, r, r), MakeVec4F32(0.0f, 0.0f, 0.0f, 0.0f), 1.0f, MakeVec4F32(1.0f, 0.0f, 0.0f, 0.8f));
    }
  }
}

// -- SpaceShip ------------------------------------------------------
func void
ASTR_UpdateSpaceShip(ASTR_SpaceArena* space_arena, ASTR_SpaceShip* space_ship, ASTR_AsteroidPool* asteroid_pool, ASTR_BulletPool* bullet_pool, F32 dt) {
  Vec2F32 direction = MakeVec2F32(0.0f, 0.0f);
  if (OS_KeyDown(OS_KEY_W)) { direction.y -= 1.0f; }
  if (OS_KeyDown(OS_KEY_S)) { direction.y += 1.0f; }
  if (OS_KeyDown(OS_KEY_D)) { direction.x += 1.0f; }
  if (OS_KeyDown(OS_KEY_A)) { direction.x -= 1.0f; }
  Vec2F32 velocity = ScaleVec2F32(NormalizeVec2F32(direction), space_ship->speed*dt);
  space_ship->position = AddVec2F32(space_ship->position, velocity);

  if (OS_KeyPressed(OS_KEY_SPACE)) { ASTR_SpawnBullet(bullet_pool, space_ship->position, space_ship->rotation); }

  Vec2F32 ship_to_arena_position = SubVec2F32(space_ship->position, space_arena->position);

  F32 look_at_angle = atan2f(ship_to_arena_position.x, ship_to_arena_position.y);
  space_ship->rotation = look_at_angle;

  F32 ship_distance_to_edge = space_arena->radius - MagnitudeVec2F32(ship_to_arena_position) - space_ship->collision_radius;
  if (ship_distance_to_edge < 0.0f) {
    space_ship->position = AddVec2F32(space_ship->position, ScaleVec2F32(NormalizeVec2F32(ship_to_arena_position), ship_distance_to_edge - 1.0f));
  }
  
  for (ASTR_Asteroid* asteroid = asteroid_pool->first_active; asteroid != 0; asteroid = asteroid->next_active) {
    F32 distance = MagnitudeVec2F32(SubVec2F32(asteroid->position, space_ship->position));
    if (distance < (asteroid->collision_radius + space_ship->collision_radius)) {
      space_ship->destroyed = 1;
    }
  }
}

func void
ASTR_DrawSpaceShip(RHI_CommandBuffer rhi_command_buffer, ASTR_SpaceShip* space_ship) {
  RectF32 draw_area = MakeRectF32(space_ship->position.x - space_ship->sprite_scissor.w*0.5f, space_ship->position.y - space_ship->sprite_scissor.w*0.5f, space_ship->sprite_scissor.w, space_ship->sprite_scissor.h);
  D_DrawSpritePart(rhi_command_buffer, astr_context.rhi_frame_buffer, draw_area, space_ship->rotation, astr_context.sprite_atlass_view, space_ship->sprite_scissor, RGBAFromHex(0xf06449ff));
  if (astr_context.debug_mode) {
    F32 r = space_ship->collision_radius;
    RectF32 collision_rect = MakeRectF32(space_ship->position.x - r, space_ship->position.y - r, r*2.0f, r*2.0f);
    D_DrawRectWithBorder(rhi_command_buffer, astr_context.rhi_frame_buffer, collision_rect, MakeVec4F32(r, r, r, r), MakeVec4F32(0.0f, 0.0f, 0.0f, 0.0f), 1.0f, MakeVec4F32(1.0f, 0.0f, 0.0f, 0.8f));
  }
}
