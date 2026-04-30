#include "base/base_include.h"
#include "os/os_include.h"
#include "rhi/rhi_include.h"
#include "assets/animation.h"
#include "assets/mesh.h"

#include "base/base_include.c"
#include "os/os_include.c"
#include "rhi/rhi_include.c"
#include "assets/animation.c"
#include "assets/mesh.c"

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"

typedef struct TopDown_Material TopDown_Material;
struct TopDown_Material {
  Vec3F32 color;
};

typedef struct TopDown_BoundingBox TopDown_BoundingBox;
struct TopDown_BoundingBox {
  Vec3F32 min;
  Vec3F32 max;
};

func TopDown_BoundingBox TopDown_BoundingBoxFromMesh(AST_StaticMesh* mesh);

typedef U32 TopDown_EntityFlag;
enum {
  TopDown_EntityFlag_Nil = 0,

  TopDown_EntityFlag_Actor     = 1 << 0,
  TopDown_EntityFlag_Movable   = 1 << 1,
  TopDown_EntityFlag_Collision = 1 << 2,

  TopDown_EntityFlag_Camera = (1 << 10),
  TopDown_EntityFlag_Player = (1 << 11) | TopDown_EntityFlag_Actor | TopDown_EntityFlag_Movable | TopDown_EntityFlag_Collision,
  TopDown_EntityFlag_Enemy  = (1 << 12) | TopDown_EntityFlag_Actor | TopDown_EntityFlag_Movable | TopDown_EntityFlag_Collision,
  TopDown_EntityFlag_Floor  =             TopDown_EntityFlag_Actor,
  TopDown_EntityFlag_Bullet = (1 << 13) | TopDown_EntityFlag_Actor | TopDown_EntityFlag_Movable | TopDown_EntityFlag_Collision,
};

typedef struct TopDown_EntityId TopDown_EntityId;
struct TopDown_EntityId {
  U32 id;
};

typedef struct TopDown_Entity TopDown_Entity;
struct TopDown_Entity {
  TopDown_EntityId   id;
  TopDown_EntityFlag kind_flags;

  struct {
    Transform        transform;
    TopDown_Material material;
    AST_StaticMesh*  mesh;
    B32              hidden;
  } actor;

  struct {
    F32 speed;
  } movable;

  struct {
    B32                 active;
    TopDown_BoundingBox bounding_box;
  } collision;

  struct {
    Vec3F32 position;
    F32     yaw;
    F32     pitch;
    F32     fov;

    Vec3F32 front;
    Vec3F32 right;
    Vec3F32 up;

    RectI32 viewport;

    Mat4F32 matrix;
    Mat4F32 inverse;
  } camera;

  struct {
    Vec3F32 start_point;
    Vec3F32 end_point;
    F32     duration;
    F32     current_time;
  } enemy;

  struct {
    Vec3F32 direction;
    B32 active;
    F32 lifetime;
    F32 current_time;
  } bullet;
};
TopDown_Entity TopDown_Entity_Nil = ZeroStruct();
DefineArray(TopDown_Entity, TopDown_EntityArray, TopDown_Entity_Nil)

func void TopDown_UpdateEntities();
func void TopDown_DrawEntities();

func void TopDown_DrawDebugCollision();

func TopDown_EntityId TopDown_CreateCamera();
func TopDown_EntityId TopDown_CreatePlayer();

func TopDown_EntityId TopDown_CreateEnemy();

func TopDown_EntityId TopDown_CreateBullet();
func void             TopDown_ActivateBullet(TopDown_EntityId parent_id);

func TopDown_EntityId TopDown_CreateFloor();

typedef struct TopDown_Light TopDown_Light;
struct TopDown_Light {
  Vec3F32 direction;
  F32 padding;
  Vec3F32 color;
};

func Vec3F32 TopDown_WorldFromScreen(Vec2F32 screen_position);

typedef struct TopDown_Context TopDown_Context;
struct TopDown_Context {
  Arena* global_arena;
  Arena* frame_arena;

  OS_Window* window;
  Vec2F32    cursor_position;

  // RHI Objects
  RHI_CommandBuffer    command_buffer;
  RHI_Buffer           frame_buffer;
  RHI_Buffer           transfer_buffer;
  RHI_Texture          default_texture;
  RHI_Texture          depth_texture;
  RHI_GraphicsPipeline pipeline;
  RHI_GraphicsPipeline debug_pipeline;

  // State
  B32 finished;
  B32 debug;
  F32 dt;

  // Assets
  AST_StaticMesh monkey_mesh;
  AST_StaticMesh bullet_mesh;
  AST_StaticMesh floor_mesh;
  AST_StaticMesh bounding_box_mesh;

  // Game Objects
  TopDown_EntityArray entities;
  TopDown_EntityId    camera_id;
  TopDown_EntityId    player_id;
  TopDown_EntityId    first_bullet_id;
  U32                 max_bullet_count;
} topdown_context = ZeroStruct();

func TopDown_Entity*
TopDown_GetEntity(TopDown_EntityId id) {
  return TopDown_EntityArrayGetPointer(&topdown_context.entities, id.id);
}

I32 main() {
  topdown_context.global_arena = AllocateArena(Gigabytes(16), Kilobytes(64));
  topdown_context.frame_arena = AllocateArena(Gigabytes(16), Kilobytes(64));

  OS_Init(Megabytes(64));
  topdown_context.window = OS_CreateWindow(Str8C("TopDown"), MakeVec2U32(1280, 720));

  RHI_Init(topdown_context.window);
  // Init RHI Objects
  topdown_context.command_buffer = RHI_GetCommandBuffer();
  topdown_context.frame_buffer = RHI_CreateBuffer(Megabytes(64), RHI_BufferUsageFlag_Vertex|RHI_BufferUsageFlag_Index|RHI_BufferUsageFlag_Uniform, RHI_BufferPropertyFlag_HostCoherent);
  topdown_context.transfer_buffer = RHI_CreateBuffer(Megabytes(128), RHI_BufferUsageFlag_Transfer, RHI_BufferPropertyFlag_HostCoherent);

  I32 tex_width = 0;
  I32 tex_height = 0;
  I32 tex_channels = 0;
  U8* tex_pixels = stbi_load("data/uv_checker.png", &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);

  topdown_context.default_texture = RHI_CreateTexture(&(RHI_TextureCreateInfo) {
    .kind = RHI_TextureKind_2D,
    .format = RHI_TextureFormat_R8G8B8A8_UNORM,
    .usage_flags = RHI_TEXTURE_USAGE_FLAG_SAMPLED | RHI_TEXTURE_USAGE_FLAG_TRANSFER_DST,
    .width = tex_width,
    .height = tex_height,
    .depth = 1,
    .num_levels = 1,
  });
  U64 texture_offset = RHI_PushBuffer(topdown_context.transfer_buffer, tex_pixels, tex_width*tex_height*4);
  RHI_CopyBufferToTexture(0, topdown_context.transfer_buffer, texture_offset, tex_width*tex_height*4, topdown_context.default_texture);

  topdown_context.depth_texture = RHI_CreateTexture(&(RHI_TextureCreateInfo) {
    .kind = RHI_TextureKind_2D,
    .format = RHI_TextureFormat_D16_UNORM,
    .usage_flags = RHI_TEXTURE_USAGE_FLAG_DEPTH_STENCIL_ATTACHMENT,
    .width = topdown_context.window->size.w,
    .height = topdown_context.window->size.h,
    .depth = 1,
    .num_levels = 1,
  });
  
  RHI_Shader vertex_shader = RHI_CreateShader(
    topdown_context.global_arena,
    &(RHI_ShaderCreateInfo){
      .file_name = Str8C("./data/TopDown/Shaders/topdown.vs"),
      .kind = RHI_ShaderKind_Vertex,
      .instance_uniforms_count = 1,
    }
  );
  RHI_Shader fragment_shader = RHI_CreateShader(
    topdown_context.global_arena,
    &(RHI_ShaderCreateInfo){
      .file_name = Str8C("./data/TopDown/Shaders/topdown.fs"),
      .kind = RHI_ShaderKind_Fragment,
      .global_uniforms_count = 1,
    }
  );

  RHI_VertexAttribute vertex_attributes[] = {
    {
      .location = 0,
      .format = RHI_VertexAttributeFormat_Vec3F32,
      .offset = offsetof(AST_Vertex, position),
    },
    {
      .location = 1,
      .format = RHI_VertexAttributeFormat_Vec3F32,
      .offset = offsetof(AST_Vertex, normal),
    },
    {
      .location = 2,
      .format = RHI_VertexAttributeFormat_Vec3F32,
      .offset = offsetof(AST_Vertex, tangent),
    },
    {
      .location = 3,
      .format = RHI_VertexAttributeFormat_Vec2F32,
      .offset = offsetof(AST_Vertex, uv),
    },
    {
      .location = 4,
      .format = RHI_VertexAttributeFormat_Vec4I32,
      .offset = offsetof(AST_Vertex, joint_ids),
    },
    {
      .location = 5,
      .format = RHI_VertexAttributeFormat_Vec4F32,
      .offset = offsetof(AST_Vertex, joint_weights),
    },
  };

  topdown_context.pipeline = RHI_CreateGraphicsPipeline(
    &(RHI_GraphicsPipelineCreateInfo) {
      .vertex_shader = vertex_shader,
      .fragment_shader = fragment_shader,
      .vertex_attributes_count = ArrayLength(vertex_attributes),
      .vertex_attributes = vertex_attributes,
      .color_targets_count = 1,
      .color_target_infos = &(RHI_GraphicsPipelineColorTargetInfo) {
        .format = RHI_GetSwapchainTextureFormat(),
      },
      .depth_stencil_state = (RHI_PipelineDepthStencilState) {
        .depth_test_enable = 1,
        .depth_write_enable = 1,
        .depth_compare_operation = RHI_CompareOperation_Greater,
        .depth_target_format = RHI_GetTextureFormat(topdown_context.depth_texture),
      },
    }
  );

  RHI_Shader debug_vertex_shader = RHI_CreateShader(
    topdown_context.global_arena,
    &(RHI_ShaderCreateInfo) {
      .file_name = Str8C("./data/TopDown/Shaders/debug.vs"),
      .kind = RHI_ShaderKind_Vertex,
      .instance_uniforms_count = 1,
    }
  );
  RHI_Shader debug_fragment_shader = RHI_CreateShader(
    topdown_context.global_arena,
    &(RHI_ShaderCreateInfo) {
      .file_name = Str8C("./data/TopDown/Shaders/debug.fs"),
      .kind = RHI_ShaderKind_Fragment,
    }
  );

  topdown_context.debug_pipeline = RHI_CreateGraphicsPipeline(
    &(RHI_GraphicsPipelineCreateInfo) {
      .vertex_shader = debug_vertex_shader,
      .fragment_shader = debug_fragment_shader,
      .vertex_attributes_count = ArrayLength(vertex_attributes),
      .vertex_attributes = vertex_attributes,
      .color_targets_count = 1,
      .color_target_infos = &(RHI_GraphicsPipelineColorTargetInfo) {
        .format = RHI_GetSwapchainTextureFormat(),
        .blend_enable = 1,
      },
    }
  );

  OS_ShowWindow(topdown_context.window);

  // Load Assets
  topdown_context.monkey_mesh = AST_LoadStaticMeshFromGLTF(topdown_context.global_arena, Str8C("data/TopDown/Models/TopDown_Triangle.gltf")),
  topdown_context.bullet_mesh = AST_LoadStaticMeshFromGLTF(topdown_context.global_arena, Str8C("data/TopDown/Models/TopDown_Projectile.gltf"));
  topdown_context.floor_mesh = AST_LoadStaticMeshFromGLTF(topdown_context.global_arena, Str8C("data/primitives/plane.gltf"));
  topdown_context.bounding_box_mesh = AST_LoadStaticMeshFromGLTF(topdown_context.global_arena, Str8C("data/primitives/cube.gltf"));

  // Init Game Objects
  topdown_context.entities = TopDown_EntityArrayAllocate(topdown_context.global_arena, 128);
  TopDown_EntityArrayAdd(&topdown_context.entities, (TopDown_Entity)ZeroStruct()); 

  topdown_context.player_id = TopDown_CreatePlayer();
  topdown_context.camera_id = TopDown_CreateCamera();
  TopDown_CreateEnemy();
  topdown_context.max_bullet_count = 64;
  topdown_context.first_bullet_id = TopDown_CreateBullet();
  for (I32 i = 0; i < topdown_context.max_bullet_count - 1; i += 1) {
    TopDown_CreateBullet();
  }
  TopDown_CreateFloor();

  U64 start_ts = OS_GetTimeTicks();
  while (!topdown_context.finished) {
    OS_EventList events = OS_DispatchEvents(topdown_context.frame_arena, topdown_context.window);
    topdown_context.cursor_position = OS_MousePosition(topdown_context.window);

    if (OS_KeyPressed(OS_KEY_ESC)) {
      topdown_context.finished = 1;
    }

    if (OS_KeyPressed(OS_KEY_F1)) {
      topdown_context.debug = !topdown_context.debug;
    }

    // Update World
    TopDown_UpdateEntities();

    // Draw World
    RHI_BeginCommandBuffer(topdown_context.command_buffer);
      RHI_Texture swapchain_texture = RHI_AcquireSwapchainTexture(topdown_context.command_buffer);

      RHI_ColorTarget color_targets = {
        .texture = swapchain_texture,
        .load_operation = RHI_AttachmentLoadOperation_Clear,
        .store_operation = RHI_AttachmentStoreOperation_Store,
        .clear_color = RGBAFromHex(0xffffffff),
      };

    RHI_DepthStencilTarget depth_target = {
      .texture = topdown_context.depth_texture,
      .load_operation = RHI_AttachmentLoadOperation_Clear,
      .store_operation = RHI_AttachmentStoreOperation_Store,
      .clear_depth = 0.0f,
    };

      RHI_RenderPass* render_pass = RHI_BeginRenderPass(topdown_context.command_buffer, 1, &color_targets, &depth_target);
        RectI32 rect = {
          .x = 0,
          .y = 0,
          .w = topdown_context.window->size.x,
          .h = topdown_context.window->size.y,
        };
        RHI_SetViewport(topdown_context.command_buffer, rect);
        RHI_SetScissor(topdown_context.command_buffer, rect);

        RHI_BindGraphicsPipeline(topdown_context.command_buffer, topdown_context.pipeline);
        struct {
          TopDown_Light light;
        } global_fs_data = {
          .light = {
            .direction = NormalizeVec3F32(MakeVec3F32(1.0f, -1.0f, 0.0f)),
            .color = MakeVec3F32(1.0f, 1.0f, 1.0f),
          },
        };
        U64 global_fs_buffer_offset = RHI_PushBuffer(topdown_context.frame_buffer, (U8*)&global_fs_data, sizeof(global_fs_data));
        RHI_BindGlobalFragmentShaderData(topdown_context.command_buffer,
          1, &(RHI_UniformBufferBindingInfo) {
            .binding = 0,
            .buffer = topdown_context.frame_buffer,
            .offset = global_fs_buffer_offset,
            .size = sizeof(global_fs_data),
          },
          0, 0
        );

        TopDown_DrawEntities();

      RHI_EndRenderPass(topdown_context.command_buffer, render_pass);

      if (topdown_context.debug) {
        RHI_ColorTarget debug_color_targets = {
          .texture = swapchain_texture,
          .load_operation = RHI_AttachmentLoadOperation_Load,
          .store_operation = RHI_AttachmentStoreOperation_Store,
        };
        RHI_RenderPass* debug_render_pass = RHI_BeginRenderPass(topdown_context.command_buffer, 1, &debug_color_targets, 0);
          RHI_BindGraphicsPipeline(topdown_context.command_buffer, topdown_context.debug_pipeline);
          TopDown_DrawDebugCollision();
        RHI_EndRenderPass(topdown_context.command_buffer, debug_render_pass);
      }
    RHI_SubmitCommandBuffer(topdown_context.command_buffer);

    U64 end_ts = OS_GetTimeTicks();
    U64 dt_ms = end_ts - start_ts;
    topdown_context.dt = (F32)(dt_ms)*0.001f;
    start_ts = end_ts;

    F32 target_frame_time = 1000.0f/60.0f;
    F32 time_to_sleep = target_frame_time - dt_ms;
    if (time_to_sleep > 0.0f) {
      OS_Sleep(time_to_sleep);
    }
  }

  return 0;
}

func TopDown_BoundingBox
TopDown_BoundingBoxFromMesh(AST_StaticMesh* mesh) {
  TopDown_BoundingBox result = ZeroStruct();

  for (AST_GeometryListNode* geometry_node = mesh->geometry_list.first; geometry_node; geometry_node = geometry_node->next) {
    AST_Geometry* geometry = &geometry_node->data;
    for (U64 vertex_index = 0; vertex_index < geometry->vertecies_count; vertex_index += 1) {
      AST_Vertex* vertex = geometry->vertecies + vertex_index;
      if (vertex->position.x < result.min.x) result.min.x = vertex->position.x;
      if (vertex->position.y < result.min.y) result.min.y = vertex->position.y;
      if (vertex->position.z < result.min.z) result.min.z = vertex->position.z;
      if (vertex->position.x > result.max.x) result.max.x = vertex->position.x;
      if (vertex->position.y > result.max.y) result.max.y = vertex->position.y;
      if (vertex->position.z > result.max.z) result.max.z = vertex->position.z;
    }
  }

  return result;
}

func Vec3F32
TopDown_WorldFromScreen(Vec2F32 screen_position)  {
  Vec3F32 result = MakeVec3F32(0.0f, 0.0f, 0.0f);
  TopDown_Entity* camera = TopDown_GetEntity(topdown_context.camera_id);
  Vec4F32 near_plane = TransformVec4F32(MakeVec4F32((2.0f*screen_position.x)/camera->camera.viewport.w  - 1.0f, (2.0f*screen_position.y)/camera->camera.viewport.h - 1.0f, 0.0f, 1.0f), camera->camera.inverse);
  near_plane = ScaleVec4F32(near_plane, 1.0f/near_plane.w);
  Vec4F32 far_plane = TransformVec4F32(MakeVec4F32((2.0f*screen_position.x)/camera->camera.viewport.w  - 1.0f, (2.0f*screen_position.y)/camera->camera.viewport.h - 1.0f, 1.0f, 1.0f), camera->camera.inverse);
  far_plane = ScaleVec4F32(far_plane, 1.0f/far_plane.w);

  Vec3F32 ray_origin = MakeVec3F32(near_plane.x, near_plane.y, near_plane.z);
  Vec3F32 ray_direction = NormalizeVec3F32(SubVec3F32(MakeVec3F32(far_plane.x, far_plane.y, far_plane.z), ray_origin));
  F32 target_height = 0.0f;
  result = AddVec3F32(ray_origin, ScaleVec3F32(ray_direction, (target_height - ray_origin.y) / ray_direction.y));
  return result;
}

func void
TopDown_UpdateEntities() {
  for (I32 entity_index = 1; entity_index < topdown_context.entities.length; entity_index += 1) {
    TopDown_Entity* entity = TopDown_EntityArrayGetPointer(&topdown_context.entities, entity_index);
    
    switch (entity->kind_flags) {
      default: {} break;

      case TopDown_EntityFlag_Player: {
        if (OS_KeyPressed(OS_KEY_SPACE)) {
          TopDown_ActivateBullet(topdown_context.player_id);
        }

        Vec3F32 input_direction = MakeVec3F32(0.0f, 0.0f, 0.0f);
        if (OS_KeyDown(OS_KEY_W)) {
          input_direction.z = -1.0f;
        }
        if (OS_KeyDown(OS_KEY_S)) {
          input_direction.z = 1.0f;
        }
        if (OS_KeyDown(OS_KEY_D)) {
          input_direction.x = 1.0f;
        }
        if (OS_KeyDown(OS_KEY_A)) {
          input_direction.x = -1.0f;
        }
        input_direction = NormalizeVec3F32(input_direction);

        Vec3F32 velocity = ScaleVec3F32(input_direction, entity->movable.speed*topdown_context.dt);
        entity->actor.transform.translation = AddVec3F32(entity->actor.transform.translation, velocity);
        Vec3F32 target_position = TopDown_WorldFromScreen(topdown_context.cursor_position);
        entity->actor.transform.rotation = QuaternionLookAt(entity->actor.transform.translation, target_position);
      } break;

      case TopDown_EntityFlag_Enemy: {
        if (entity->enemy.current_time > entity->enemy.duration) {
          entity->enemy.current_time = 0.0f;
        }
        F32 t = entity->enemy.current_time/entity->enemy.duration;
        if (t < 0.5f) {
          Vec3F32 direction = NormalizeVec3F32(SubVec3F32(entity->enemy.end_point, entity->actor.transform.translation));
          Vec3F32 velocity = ScaleVec3F32(direction, entity->movable.speed*topdown_context.dt);
          entity->actor.transform.translation = AddVec3F32(entity->actor.transform.translation, velocity);
        } else {
          Vec3F32 direction = NormalizeVec3F32(SubVec3F32(entity->enemy.start_point, entity->actor.transform.translation));
          Vec3F32 velocity = ScaleVec3F32(direction, entity->movable.speed*topdown_context.dt);
          entity->actor.transform.translation = AddVec3F32(entity->actor.transform.translation, velocity);
        }
        entity->enemy.current_time += topdown_context.dt;
      } break;

      case TopDown_EntityFlag_Camera: {
        TopDown_Entity* player = TopDown_GetEntity(topdown_context.player_id);
        entity->camera.position = AddVec3F32(player->actor.transform.translation, MakeVec3F32(0.0f, 20.0f, 10.0f));

        Mat4F32 view_matrix = MakeLookAtMat4F32(entity->camera.position, player->actor.transform.translation, MakeVec3F32(0.0f, 1.0f, 0.0f));
        Mat4F32 projection_matrix = MakePerspectiveMat4F32(
          entity->camera.fov/2.0f, (F32)topdown_context.window->size.x/(F32)topdown_context.window->size.y,
          1.0f, 100.0f
        );
        entity->camera.matrix = MulMat4F32(projection_matrix, view_matrix);
        entity->camera.inverse = InverseMat4F32(entity->camera.matrix);
      } break;

      case TopDown_EntityFlag_Bullet: {
        if (entity->bullet.active) {
          Vec3F32 velocity = ScaleVec3F32(entity->bullet.direction, entity->movable.speed*topdown_context.dt);
          entity->actor.transform.translation = AddVec3F32(entity->actor.transform.translation, velocity);
          // entity->actor.transform.rotation = QuaternionFromEuler(0.0f, RadiansFromDegrees(720.0f)*(entity->bullet.current_time/entity->bullet.lifetime), 0.0f);
          
          entity->bullet.current_time += topdown_context.dt;
          entity->bullet.active = entity->bullet.current_time < entity->bullet.lifetime;
          entity->actor.hidden = !entity->bullet.active;
          entity->collision.active = entity->bullet.active;
        }
      } break;
    }
  }
}

func void
TopDown_DrawEntities() {
  for (I32 entity_index = 1; entity_index < topdown_context.entities.length; entity_index += 1) {
    TopDown_Entity* entity = TopDown_EntityArrayGetPointer(&topdown_context.entities, entity_index);
    TopDown_Entity* camera = TopDown_GetEntity(topdown_context.camera_id);

    B32 to_draw = (entity->kind_flags & TopDown_EntityFlag_Actor) && (!entity->actor.hidden);
    if (to_draw) {
      for (AST_GeometryListNode* geometry_node = entity->actor.mesh->geometry_list.first; geometry_node; geometry_node = geometry_node->next) {
        AST_Geometry* geometry = &geometry_node->data;
        
        struct {
          Mat4F32 transform;
          Mat4F32 camera_transform;
          TopDown_Material material;
        } instance_vs_data = {
          .transform = Mat4F32FromTransform(entity->actor.transform),
          .camera_transform = camera->camera.matrix,
          .material = entity->actor.material,
        };

        U64 instance_vs_data_offset = RHI_PushBuffer(topdown_context.frame_buffer, (U8*)&instance_vs_data, sizeof(instance_vs_data));
        U64 vertex_data_offset = RHI_PushBuffer(topdown_context.frame_buffer, (U8*)geometry->vertecies, geometry->vertecies_count*sizeof(AST_Vertex));
        U64 index_data_offset = RHI_PushBuffer(topdown_context.frame_buffer, (U8*)geometry->index_data, geometry->index_size*geometry->index_count);

        RHI_BindInstanceVertexShaderData(topdown_context.command_buffer, 1, &(RHI_UniformBufferBindingInfo){
          .binding = 0,
          .buffer = topdown_context.frame_buffer,
          .offset = instance_vs_data_offset,
          .size = sizeof(instance_vs_data),
        },
        0, 0);
        RHI_BindVertexBuffer(topdown_context.command_buffer, topdown_context.frame_buffer, vertex_data_offset);
        RHI_BindIndexBuffer(topdown_context.command_buffer, topdown_context.frame_buffer, index_data_offset, RHI_IndexSize_U16);
        RHI_DrawIndexedPrimitives(topdown_context.command_buffer, geometry->index_count, 1, 0, 0, 0);
      }
    }
  }
}

func void
TopDown_DrawDebugCollision() {
  for (I32 entity_index = 1; entity_index < topdown_context.entities.length; entity_index += 1) {
    TopDown_Entity* entity = TopDown_EntityArrayGetPointer(&topdown_context.entities, entity_index);
    TopDown_Entity* camera = TopDown_GetEntity(topdown_context.camera_id);

    B32 to_draw = (entity->kind_flags & TopDown_EntityFlag_Collision) && entity->collision.active;
    if (to_draw) {
      for (AST_GeometryListNode* geometry_node = topdown_context.bounding_box_mesh.geometry_list.first; geometry_node; geometry_node = geometry_node->next) {
        AST_Geometry* geometry = &geometry_node->data;

        TopDown_BoundingBox bounding_box = entity->collision.bounding_box;

        Transform bounding_box_transform = {
          .translation = entity->actor.transform.translation,
          .rotation = entity->actor.transform.rotation,
          .scale = MulVec3F32(entity->actor.transform.scale, SubVec3F32(bounding_box.max, bounding_box.min)),
        };
        
        struct {
          Mat4F32 transform;
        } instance_vs_data = {
          .transform = MulMat4F32(camera->camera.matrix, Mat4F32FromTransform(bounding_box_transform)),
        };

        U64 instance_vs_data_offset = RHI_PushBuffer(topdown_context.frame_buffer, (U8*)&instance_vs_data, sizeof(instance_vs_data));
        U64 vertex_data_offset = RHI_PushBuffer(topdown_context.frame_buffer, (U8*)geometry->vertecies, geometry->vertecies_count*sizeof(AST_Vertex));
        U64 index_data_offset = RHI_PushBuffer(topdown_context.frame_buffer, (U8*)geometry->index_data, geometry->index_size*geometry->index_count);

        RHI_BindInstanceVertexShaderData(topdown_context.command_buffer, 1, &(RHI_UniformBufferBindingInfo){
          .binding = 0, 
          .buffer = topdown_context.frame_buffer,
          .offset = instance_vs_data_offset,
          .size = sizeof(instance_vs_data),
        },
        0, 0);
        RHI_BindVertexBuffer(topdown_context.command_buffer, topdown_context.frame_buffer, vertex_data_offset);
        RHI_BindIndexBuffer(topdown_context.command_buffer, topdown_context.frame_buffer, index_data_offset, RHI_IndexSize_U16);
        RHI_DrawIndexedPrimitives(topdown_context.command_buffer, geometry->index_count, 1, 0, 0, 0);
      }
    }
  }
}

func TopDown_EntityId
TopDown_CreateCamera() {
  TopDown_EntityId result = ZeroStruct();

  if (topdown_context.camera_id.id == 0) {
    TopDown_Entity camera = {
      .kind_flags = TopDown_EntityFlag_Camera,
      .camera = {
        .position = MakeVec3F32(1.0f, 2.0f, 5.0f),
        .yaw = -90.0f,
        .pitch = -30.0f,
        .fov = 90.0f,
        .front = MakeVec3F32(1.0f, 0.0f, -1.0f),
        .right = MakeVec3F32(1.0f, 0.0f, 1.0f),
        .up = MakeVec3F32(0.0f, 1.0f, 0.0f),
        .viewport = (RectI32) {
          .x = 0,
          .y = 0,
          .w = topdown_context.window->size.x,
          .h = topdown_context.window->size.y,
        },
      },
    };
    result.id = TopDown_EntityArrayAdd(&topdown_context.entities, camera);
  }
  else {
    LogDebug("Camera is already created (id = %i)\n", topdown_context.camera_id)
  }

  return result;
}

func TopDown_EntityId
TopDown_CreatePlayer() {
  TopDown_EntityId result = ZeroStruct();

  if (topdown_context.player_id.id == 0) {
    TopDown_Entity player = {
      .kind_flags = TopDown_EntityFlag_Player,
      .actor = {
        .transform = IdentityTransform(),
        .material = {
          .color = MakeVec3F32(1.0f, 1.0f, 0.0f),
        },
        .mesh = &topdown_context.monkey_mesh,
      },
      .collision = {
        .active = 1,
        .bounding_box = TopDown_BoundingBoxFromMesh(&topdown_context.monkey_mesh),
      },
      .movable = {
        .speed = 3.0f,
      }
    };
    result.id = TopDown_EntityArrayAdd(&topdown_context.entities, player);
  }
  else {
    LogDebug("Player is already created (id = %i)\n", topdown_context.player_id);
  }
  
  return result;
}

func TopDown_EntityId
TopDown_CreateEnemy() {
  TopDown_EntityId result = ZeroStruct();

  TopDown_Entity enemy = {
    .kind_flags = TopDown_EntityFlag_Enemy,
    .actor = {
      .transform = IdentityTransform(),
      .material = {
        .color = MakeVec3F32(1.0f, 0.0f, 1.0f),
      },
      .mesh = &topdown_context.monkey_mesh,
    },
    .movable = {
      .speed = 7.0f,
    },
    .collision = {
      .active = 1,
      .bounding_box = TopDown_BoundingBoxFromMesh(&topdown_context.monkey_mesh),
    },
    .enemy = {
      .start_point = MakeVec3F32(-5.0f, 0.0f, 5.0f),
      .end_point = MakeVec3F32(5.0f, 0.0f, 5.0f),
      .duration = 10.0f,
      .current_time = 0.0f,
    },
  };
  result.id = TopDown_EntityArrayAdd(&topdown_context.entities, enemy);

  return result;
}

func TopDown_EntityId
TopDown_CreateBullet() {
  TopDown_EntityId result = ZeroStruct();

  TopDown_Entity bullet = {
    .kind_flags = TopDown_EntityFlag_Bullet,
    .actor = {
      .transform = IdentityTransform(),
      .material = {
        .color = MakeVec3F32(0.5f, 0.0f, 1.0f),
      },
      .mesh = &topdown_context.bullet_mesh,
      .hidden = 1,
    },
    .movable = {
      .speed = 10.0f,
    },
    .collision = {
      .active = 0,
      .bounding_box = TopDown_BoundingBoxFromMesh(&topdown_context.bullet_mesh),
    },
    .bullet = {
      .lifetime = 3.0f,
    }
  };
  result.id = TopDown_EntityArrayAdd(&topdown_context.entities, bullet);

  return result;
}

func void
TopDown_ActivateBullet(TopDown_EntityId parent_id) {
  for (I32 bullet_id = topdown_context.first_bullet_id.id; bullet_id < topdown_context.max_bullet_count; bullet_id += 1) {
    TopDown_Entity* bullet = TopDown_EntityArrayGetPointer(&topdown_context.entities, bullet_id);
    TopDown_Entity* parent = TopDown_GetEntity(parent_id);

    if (!bullet->bullet.active) {
      Vec3F32 parent_forward = RotateVec3F32(MakeVec3F32(0.0f, 0.0f, 1.0f), parent->actor.transform.rotation);

      bullet->actor.transform.translation = parent->actor.transform.translation;
      bullet->actor.transform.rotation = parent->actor.transform.rotation;
      bullet->actor.hidden = 1;
      bullet->bullet.direction = parent_forward;
      bullet->bullet.active = 1;
      bullet->bullet.current_time = 0.0f;

      break;
    }
  }
}

func TopDown_EntityId
TopDown_CreateFloor() {
  TopDown_EntityId result = ZeroStruct();

  TopDown_Entity floor = {
    .kind_flags = TopDown_EntityFlag_Floor,
    .actor = {
      .transform = {
        .translation = MakeVec3F32(0.0f, 0.0f, 0.0f),
        .rotation = IdentityQuaternion(),
        .scale = MakeVec3F32(10.0f, 1.0f, 10.0f),
      },
      .material = {
        .color = MakeVec3F32(0.5f, 0.5f, 0.5f),
      },
      .mesh = &topdown_context.floor_mesh,
    },
  };
  result.id = TopDown_EntityArrayAdd(&topdown_context.entities, floor);

  return result;
}
