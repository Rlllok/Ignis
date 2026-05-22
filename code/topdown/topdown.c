#include "base/base_include.h"
#include "os/os_include.h"
#include "rhi/rhi_include.h"
#include "assets/animation.h"
#include "assets/mesh.h"
#include "assets/font.h"

#include "base/base_include.c"
#include "os/os_include.c"
#include "rhi/rhi_include.c"
#include "assets/animation.c"
#include "assets/mesh.c"
#include "assets/font.c"

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb/stb_image.h"

typedef struct TopDown_Mesh TopDown_Mesh;
struct TopDown_Mesh {
  AST_StaticMesh mesh;
  U64            vertex_buffer_offset[32]; // --AlNov: @NOTE @TODO Temporary solution. Offset per AST_Geometry in mesh
  U64            indecies_offset[32]; // --AlNov: @NOTE @TODO Temporary solution. Offset per AST_Geometry in mesh
};

func TopDown_Mesh TopDown_LoadAndPrepareMesh(Arena* arena, Str8 path);

typedef struct TopDown_Font TopDown_Font;
struct TopDown_Font {
  RHI_Texture glyphs[92];
  Vec2I32     sizes[92];
};

func TopDown_Font TopDown_LoadFontFromTTF(Arena* arena, Str8 path, U16 size);

typedef struct TopDown_Material TopDown_Material;
struct TopDown_Material {
  Vec3F32 color; F32 padding0;
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
  TopDown_EntityFlag_Bullet = (1 << 13) | TopDown_EntityFlag_Actor | TopDown_EntityFlag_Movable | TopDown_EntityFlag_Collision,
  TopDown_EntityFlag_Floor  = (1 << 14) | TopDown_EntityFlag_Actor,
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
    TopDown_Mesh*    mesh;
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
    Transform transform;
    F32       fov;

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

func void TopDown_DrawHexGrid();

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

typedef struct TopDown_DrawCommand TopDown_DrawCommand;
struct TopDown_DrawCommand {
  U64 vertex_buffer_offset;
  U64 indecies_count;
  U64 index_buffer_offset;
  U64 object_buffer_offset;
};
TopDown_DrawCommand TopDown_DrawCommandNil = ZeroStruct();
DefineArray(TopDown_DrawCommand, TopDown_DrawCommandArray, TopDown_DrawCommandNil)

func void TopDown_PrepareDrawCommands();

typedef struct TopDown_Context TopDown_Context;
struct TopDown_Context {
  Arena* global_arena;
  Arena* frame_arena;

  OS_Window* window;
  Vec2F32    cursor_position;

  // RHI Objects
  RHI_CommandBuffer    command_buffer;
  RHI_CommandBuffer    transfer_command_buffer;
  RHI_Buffer           vertex_buffer;
  RHI_Buffer           object_buffer;
  RHI_Buffer           transfer_buffer;
  RHI_Texture          default_texture;
  RHI_Texture          depth_texture;
  RHI_GraphicsPipeline entity_pipeline;
  RHI_GraphicsPipeline text_pipeline;
  RHI_GraphicsPipeline hex_grid_pipeline;
  RHI_GraphicsPipeline bounding_box_pipeline;

  // State
  B32 finished;
  B32 debug;
  F32 dt;

  // Assets
  TopDown_Mesh monkey_mesh;
  TopDown_Mesh bullet_mesh;
  TopDown_Mesh floor_mesh;
  TopDown_Mesh bounding_box_mesh;
  TopDown_Font font;

  // Game Objects
  TopDown_EntityArray entities;
  TopDown_EntityId    camera_id;
  TopDown_EntityId    player_id;
  TopDown_EntityId    first_bullet_id;
  U32                 max_bullet_count;

  // Rendering
  TopDown_DrawCommandArray draw_commands;
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
  topdown_context.vertex_buffer = RHI_CreateBuffer(Str8C("VertexIndexBuffer"), Megabytes(64), RHI_BufferUsageFlag_Vertex|RHI_BufferUsageFlag_Index, RHI_BufferPropertyFlag_HostCoherent);
  topdown_context.object_buffer = RHI_CreateBuffer(Str8C("ObjectBuffer"), Megabytes(64), RHI_BufferUsageFlag_Storage|RHI_BufferUsageFlag_Address, RHI_BufferPropertyFlag_HostCoherent);
  topdown_context.transfer_buffer = RHI_CreateBuffer(Str8C("TransferBuffer"), Megabytes(128), RHI_BufferUsageFlag_Transfer, RHI_BufferPropertyFlag_HostCoherent);

  topdown_context.draw_commands = TopDown_DrawCommandArrayAllocate(topdown_context.global_arena, 2048);

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
  RHI_BeginCommandBuffer(topdown_context.transfer_command_buffer); {
    RHI_CopyBufferToTexture(topdown_context.transfer_command_buffer, topdown_context.transfer_buffer, 0, topdown_context.default_texture);
  }
  RHI_EndCommandBuffer(topdown_context.transfer_command_buffer);
  id<MTLSharedEvent> sync_event = [_rhi_metal_context.device newSharedEvent];
  [_rhi_metal_context.command_queue signalEvent:sync_event value:1];
  RHI_SubmitCommandBuffer(topdown_context.transfer_command_buffer);

  topdown_context.depth_texture = RHI_CreateTexture(&(RHI_TextureCreateInfo) {
    .kind = RHI_TextureKind_2D,
    .format = RHI_TextureFormat_D16_UNORM,
    .usage_flags = RHI_TEXTURE_USAGE_FLAG_DEPTH_STENCIL_ATTACHMENT,
    .width = topdown_context.window->size.w,
    .height = topdown_context.window->size.h,
    .depth = 1,
    .num_levels = 1,
  });
  
  // Entity Pipeline
  {
    RHI_ShaderArgumentKind vs_arguments[] = {
      RHI_ShaderArgumentKind_BufferAddress,
    };
    RHI_Shader vertex_shader = RHI_CreateShader(
      topdown_context.global_arena,
      &(RHI_ShaderCreateInfo){
        .file_name = Str8C("./data/TopDown/Shaders/topdown.vs"),
        .kind = RHI_ShaderKind_Vertex,
        .arguments = vs_arguments,
        .arguments_count = ArrayLength(vs_arguments),
      }
    );
    RHI_Shader fragment_shader = RHI_CreateShader(
      topdown_context.global_arena,
      &(RHI_ShaderCreateInfo){
        .file_name = Str8C("./data/TopDown/Shaders/topdown.fs"),
        .kind = RHI_ShaderKind_Fragment,
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

    topdown_context.entity_pipeline = RHI_CreateGraphicsPipeline(
      &(RHI_GraphicsPipelineCreateInfo) {
        .vertex_shader = &vertex_shader,
        .fragment_shader = &fragment_shader,
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
  }
  
  {
    RHI_ShaderArgumentKind arguments[] = {
      RHI_ShaderArgumentKind_BufferAddress,
    };

    RHI_Shader vertex_shader = RHI_CreateShader(
      topdown_context.global_arena,
      &(RHI_ShaderCreateInfo) {
        .file_name = Str8C("./data/TopDown/Shaders/text.vs"),
        .kind = RHI_ShaderKind_Vertex,
        .arguments = arguments,
        .arguments_count = ArrayLength(arguments),
      }
    );

    RHI_Shader fragment_shader = RHI_CreateShader(
      topdown_context.global_arena,
      &(RHI_ShaderCreateInfo) {
        .file_name = Str8C("./data/TopDown/Shaders/text.fs"),
        .kind = RHI_ShaderKind_Fragment,
        .arguments = arguments,
        .arguments_count = ArrayLength(arguments),
      }
    );

    topdown_context.text_pipeline = RHI_CreateGraphicsPipeline(
      &(RHI_GraphicsPipelineCreateInfo) {
        .vertex_shader = &vertex_shader,
        .fragment_shader = &fragment_shader,
        .color_targets_count = 1,
        .color_target_infos = &(RHI_GraphicsPipelineColorTargetInfo) {
          .format = RHI_GetSwapchainTextureFormat(),
          .blend_enable = 1,
        },
      }
    );
  }

  // Grid Pipeline
  {
    RHI_ShaderArgumentKind vs_arguments[] = {
      RHI_ShaderArgumentKind_BufferAddress,
    };

    RHI_Shader hex_grid_vertex_shader = RHI_CreateShader(
      topdown_context.global_arena,
      &(RHI_ShaderCreateInfo) {
        .file_name = Str8C("./data/TopDown/Shaders/hex_grid.vs"),
        .kind = RHI_ShaderKind_Vertex,
        .arguments = vs_arguments,
        .arguments_count = ArrayLength(vs_arguments),
      }
    );

    RHI_ShaderArgumentKind fs_arguments[] = {
      RHI_ShaderArgumentKind_BufferAddress
    };

    RHI_Shader hex_grid_fragment_shader = RHI_CreateShader(
      topdown_context.global_arena,
      &(RHI_ShaderCreateInfo) {
        .file_name = Str8C("./data/TopDown/Shaders/hex_grid.fs"),
        .kind = RHI_ShaderKind_Fragment,
        .arguments = fs_arguments,
        .arguments_count = ArrayLength(fs_arguments),
      }
    );

    topdown_context.hex_grid_pipeline = RHI_CreateGraphicsPipeline(
      &(RHI_GraphicsPipelineCreateInfo) {
        .vertex_shader = &hex_grid_vertex_shader,
        .fragment_shader = &hex_grid_fragment_shader,
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
  }

  // Bounding Box Pipline
  {
    RHI_ShaderArgumentKind arguments[] = {
      RHI_ShaderArgumentKind_BufferAddress,
    };

    RHI_Shader vertex_shader = RHI_CreateShader(
      topdown_context.global_arena,
      &(RHI_ShaderCreateInfo) {
        .file_name = Str8C("./data/TopDown/Shaders/debug.vs"),
        .kind = RHI_ShaderKind_Vertex,
        .arguments = arguments,
        .arguments_count = ArrayLength(arguments),
      }
    );

    RHI_Shader fragment_shader = RHI_CreateShader(
      topdown_context.global_arena,
      &(RHI_ShaderCreateInfo) {
        .file_name = Str8C("./data/TopDown/Shaders/debug.fs"),
        .kind = RHI_ShaderKind_Fragment,
        .arguments = arguments,
        .arguments_count = ArrayLength(arguments),
      }
    );

    topdown_context.bounding_box_pipeline = RHI_CreateGraphicsPipeline(
      &(RHI_GraphicsPipelineCreateInfo) {
        .vertex_shader = &vertex_shader,
        .fragment_shader = &fragment_shader,
        .color_targets_count = 1,
        .color_target_infos = &(RHI_GraphicsPipelineColorTargetInfo) {
          .format = RHI_GetSwapchainTextureFormat(),
          .blend_enable = 1,
        },
      }
    );
  }

  OS_ShowWindow(topdown_context.window);

  // Load Assets
  topdown_context.monkey_mesh = TopDown_LoadAndPrepareMesh(topdown_context.global_arena, Str8C("data/TopDown/Models/TopDown_Triangle.gltf"));
  topdown_context.bullet_mesh = TopDown_LoadAndPrepareMesh(topdown_context.global_arena, Str8C("data/TopDown/Models/TopDown_Projectile.gltf"));
  topdown_context.floor_mesh = TopDown_LoadAndPrepareMesh(topdown_context.global_arena, Str8C("data/primitives/plane.gltf"));
  topdown_context.bounding_box_mesh = TopDown_LoadAndPrepareMesh(topdown_context.global_arena, Str8C("data/primitives/cube.gltf"));
  topdown_context.font = TopDown_LoadFontFromTTF(topdown_context.global_arena, Str8C("data/fonts/RobotoMono-Regular.ttf"), 32);

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
    TopDown_PrepareDrawCommands();

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
      
      RHI_Resource render_pass_resources[] = {
        {
          .kind = RHI_ResourceKind_Buffer,
          .buffer = topdown_context.vertex_buffer,
        },
        {
          .kind = RHI_ResourceKind_Buffer,
          .buffer = topdown_context.object_buffer,
        },
      };
  
      RHI_RenderPass* render_pass = RHI_BeginRenderPass(topdown_context.command_buffer, 1, &color_targets, &depth_target, render_pass_resources, ArrayLength(render_pass_resources));
        RectI32 rect = {
          .x = 0,
          .y = 0,
          .w = topdown_context.window->size.x,
          .h = topdown_context.window->size.y,
        };
        RHI_SetViewport(topdown_context.command_buffer, rect);
        RHI_SetScissor(topdown_context.command_buffer, rect);

        TopDown_DrawHexGrid();
        TopDown_DrawEntities();

      RHI_EndRenderPass(topdown_context.command_buffer, render_pass);

      if (topdown_context.debug) {
        RHI_Resource debug_render_pass_resources[] = {
          {
            .kind = RHI_ResourceKind_Buffer,
            .buffer = topdown_context.object_buffer,
          },
        };

        RHI_ColorTarget debug_color_targets = {
          .texture = swapchain_texture,
          .load_operation = RHI_AttachmentLoadOperation_Load,
          .store_operation = RHI_AttachmentStoreOperation_Store,
        };
        RHI_RenderPass* debug_render_pass = RHI_BeginRenderPass(topdown_context.command_buffer, 1, &debug_color_targets, 0, debug_render_pass_resources, ArrayLength(debug_render_pass_resources)); {
          RHI_BindGraphicsPipeline(topdown_context.command_buffer, topdown_context.bounding_box_pipeline);
          TopDown_DrawDebugCollision();
        }
        RHI_EndRenderPass(topdown_context.command_buffer, debug_render_pass);
      }

      RHI_Resource text_render_pass_resources[] = {
        {
          .kind = RHI_ResourceKind_Buffer,
          .buffer = topdown_context.object_buffer,
        },
        {
          .kind = RHI_ResourceKind_ArrayOfTextures,
          .textures.array = &topdown_context.default_texture,
          .textures.count = 1,
        },
      };

      RHI_ColorTarget text_color_targets = {
        .texture = swapchain_texture,
        .load_operation = RHI_AttachmentLoadOperation_Load,
        .store_operation = RHI_AttachmentStoreOperation_Store,
      };
      RHI_RenderPass* text_render_pass = RHI_BeginRenderPass(topdown_context.command_buffer, 1, &text_color_targets, 0, text_render_pass_resources, ArrayLength(text_render_pass_resources)); {
          RHI_Metal_Texture* mtl_texture = RHI_Metal_TextureFromHandle(topdown_context.font.glyphs['J' - 32]);
          Vec2I32 glyph_size = topdown_context.font.sizes['J' - 32];
          struct {
            Mat4F32 projection;
            Vec4F32 position_size;
            MTLResourceID texture_id;
          } glyph_data = {
            .projection = MakeOrthographicMat4F32(0.0f, topdown_context.window->size.w, topdown_context.window->size.y, 0.0f, -1.0f, 1.0f),
            .position_size = MakeVec4F32(50.0f, 50.0f, glyph_size.x, glyph_size.y),
            .texture_id = mtl_texture->mtl.gpuResourceID,
          };
          U64 glyph_data_offset = RHI_PushBuffer(topdown_context.object_buffer, (U8*)&glyph_data, sizeof(glyph_data));

        RHI_ShaderArgument arguments[] = {
          {
            .kind = RHI_ShaderArgumentKind_BufferAddress,
            .address = RHI_BufferDeviceAddress(topdown_context.object_buffer) + glyph_data_offset,
          },
        };
        RHI_BindGraphicsPipeline(topdown_context.command_buffer, topdown_context.text_pipeline);
        RHI_BindShaderArguments(topdown_context.command_buffer, RHI_ShaderKind_Vertex|RHI_ShaderKind_Fragment, arguments, ArrayLength(arguments));
        RHI_DrawPrimitives(topdown_context.command_buffer, 6, 1, 0, 0);
      }
      RHI_EndRenderPass(topdown_context.command_buffer, text_render_pass);
    RHI_EndCommandBuffer(topdown_context.command_buffer);
    RHI_SubmitCommandBuffer(topdown_context.command_buffer);
    RHI_Present(topdown_context.command_buffer);

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

func TopDown_Mesh
TopDown_LoadAndPrepareMesh(Arena* arena, Str8 path) {
  TopDown_Mesh result = ZeroStruct();
  result.mesh = AST_LoadStaticMeshFromGLTF(topdown_context.global_arena, path);
  I32 geometry_index = 0;
  for (AST_GeometryListNode* geometry_node = result.mesh.geometry_list.first; geometry_node; geometry_node = geometry_node->next) {
    AST_Geometry* geometry = &geometry_node->data;
    result.vertex_buffer_offset[geometry_index] = RHI_PushBuffer(topdown_context.vertex_buffer, (U8*)geometry->vertecies, geometry->vertecies_count*sizeof(AST_Vertex));
    result.indecies_offset[geometry_index] = RHI_PushBuffer(topdown_context.vertex_buffer, (U8*)geometry->index_data, geometry->index_count*sizeof(U16));
    geometry_index += 1;
  }
  return result;
}

func TopDown_Font
TopDown_LoadFontFromTTF(Arena* arena, Str8 path, U16 size) {
  TopDown_Font result = ZeroStruct();
  ScratchArena scratch = BeginScratchArena(arena); {
    AST_Font ast_font = AST_FontFromTTF(scratch.arena, Str8C("data/fonts/RobotoMono-Regular.ttf"), size);
    
    for(I32 ascii_code = 33; ascii_code <= 126; ascii_code += 1) {
      AST_FontGlyph* ast_glyph = ast_font.glyphs + ascii_code - 32;

      result.glyphs[ascii_code - 32] = RHI_CreateTexture(&(RHI_TextureCreateInfo) {
        .kind = RHI_TextureKind_2D,
        .format = RHI_TextureFormat_R8_UNORM,
        .usage_flags = RHI_TEXTURE_USAGE_FLAG_SAMPLED|RHI_TEXTURE_USAGE_FLAG_TRANSFER_DST,
        .width = ast_glyph->width,
        .height = ast_glyph->height,
        .depth = 1,
        .num_levels = 1,
      });
      U64 texture_offset = RHI_PushBuffer(topdown_context.transfer_buffer, ast_glyph->bitmap, ast_glyph->width*ast_glyph->height);
      RHI_BeginCommandBuffer(topdown_context.transfer_command_buffer); {
        RHI_CopyBufferToTexture(topdown_context.transfer_command_buffer, topdown_context.transfer_buffer, texture_offset, result.glyphs[ascii_code - 32]);
      }
      RHI_EndCommandBuffer(topdown_context.transfer_command_buffer);
      id<MTLSharedEvent> sync_event = [_rhi_metal_context.device newSharedEvent];
      [_rhi_metal_context.command_queue signalEvent:sync_event value:1];
      RHI_SubmitCommandBuffer(topdown_context.transfer_command_buffer);

      result.sizes[ascii_code - 32] = MakeVec2I32(ast_glyph->width, ast_glyph->height);
    }
  }
  EndScratchArena(scratch);

  return result;
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
TopDown_PrepareDrawCommands() {
  TopDown_DrawCommandArrayReset(&topdown_context.draw_commands);

  for (I32 entity_index = 1; entity_index < topdown_context.entities.length; entity_index += 1) {
    TopDown_Entity* entity = TopDown_EntityArrayGetPointer(&topdown_context.entities, entity_index);
    TopDown_Entity* camera = TopDown_GetEntity(topdown_context.camera_id);

    B32 is_floor = ((entity->kind_flags & TopDown_EntityFlag_Floor) == TopDown_EntityFlag_Floor);
    B32 is_actor = ((entity->kind_flags & TopDown_EntityFlag_Actor) == TopDown_EntityFlag_Actor);
    B32 to_draw = (!is_floor) && (is_actor) && (!entity->actor.hidden);
    if (to_draw) {
      I32 geometry_index = 0;
      for (AST_GeometryListNode* geometry_node = entity->actor.mesh->mesh.geometry_list.first; geometry_node; geometry_node = geometry_node->next) {
        AST_Geometry* geometry = &geometry_node->data;

        struct {
          Mat4F32          transform;
          Mat4F32          camera_transform;
          TopDown_Material material;
        } object_data = {
          .transform = Mat4F32FromTransform(entity->actor.transform),
          .camera_transform = camera->camera.matrix,
          .material = entity->actor.material,
        };

        TopDown_DrawCommand draw_command = {
          .indecies_count = geometry->index_count,
          .index_buffer_offset = entity->actor.mesh->indecies_offset[geometry_index],
          .vertex_buffer_offset = entity->actor.mesh->vertex_buffer_offset[geometry_index],
          .object_buffer_offset = RHI_PushBuffer(topdown_context.object_buffer, (U8*)&object_data, sizeof(object_data)),
        };
        TopDown_DrawCommandArrayAdd(&topdown_context.draw_commands, draw_command);
        
        geometry_index += 1;
      }
    }
  }
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
        Vec3F32 cursor_world = TopDown_WorldFromScreen(topdown_context.cursor_position);
        Vec3F32 player_to_cursor = SubVec3F32(cursor_world, player->actor.transform.translation);
        F32 distance = MagnitudeVec3F32(player_to_cursor);
        F32 min_distance = 5.0f;
        F32 max_distance = 15.0f;
        F32 max_camera_player_offset = 2.0f;
        F32 t = 0;
        if (distance > min_distance) {
          t = (distance - min_distance)/(max_distance - min_distance);
        }
        Vec3F32 cursor_offset = ScaleVec3F32(NormalizeVec3F32(player_to_cursor), max_camera_player_offset*t);

        F32 camera_height = 25.0f;
        Vec3F32 camera_offset = AddVec3F32(MakeVec3F32(0.0f, camera_height, camera_height/5.67128f), cursor_offset);
        entity->camera.transform.translation = AddVec3F32(player->actor.transform.translation, camera_offset);

        Vec3F32 camera_front = RotateVec3F32(MakeVec3F32(0.0f, 0.0f, -1.0f), entity->camera.transform.rotation);

        Mat4F32 view_matrix = MakeLookAtMat4F32(entity->camera.transform.translation, AddVec3F32(entity->camera.transform.translation, camera_front), MakeVec3F32(0.0f, 1.0f, 0.0f));
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
TopDown_DrawHexGrid() {
  TopDown_Entity* camera = TopDown_GetEntity(topdown_context.camera_id);

  Transform floor_transform = {
    .translation = camera->actor.transform.translation,
    .rotation = IdentityQuaternion(),
    .scale = MakeVec3F32(100.0f, 0.0f, 100.0f),
  };

  struct {
    Mat4F32 transform;
    Mat4F32 camera_transform;
    Vec3F32 background_color; F32 padding0;
    Vec3F32 grid_color; F32 padding1;
  } grid_data = {
    .transform = Mat4F32FromTransform(floor_transform),
    .camera_transform = camera->camera.matrix,
    .background_color = MakeVec3F32(0.02f, 0.11f, 0.01f),
    .grid_color = MakeVec3F32(0.98f, 0.75f, 0.34f),
  };
  U64 grid_data_offset = RHI_PushBuffer(topdown_context.object_buffer, (U8*)&grid_data, sizeof(grid_data));

  struct {
    RHI_DeviceAddress grid_data_address;
  } args = {
    .grid_data_address = RHI_BufferDeviceAddress(topdown_context.object_buffer) + grid_data_offset,
  };
  RHI_ShaderArgument arguments[] = {
    {
      .kind = RHI_ShaderArgumentKind_BufferAddress,
      .address = RHI_BufferDeviceAddress(topdown_context.object_buffer) + grid_data_offset,
    },
  };

  RHI_BindGraphicsPipeline(topdown_context.command_buffer, topdown_context.hex_grid_pipeline);
  // --AlNov: @TODO (Investigate Metal)
  // Shader Arguments should be binded to vertex and fragment stages simultaniously.
  // In vulkan stages shares push_constants. So there was a problem (as vertex arguments and fragment arguments was binded separatly) of data
  // overwriting. Solved by binding to both stages and repeting push_constants in both shadres
  // (this is why it coult be better to use one shader file for both stages)
  RHI_BindShaderArguments(topdown_context.command_buffer, RHI_ShaderKind_Vertex|RHI_ShaderKind_Fragment, arguments, ArrayLength(arguments));
  RHI_DrawPrimitives(topdown_context.command_buffer, 6, 1, 0, 0);
}

func void
TopDown_DrawEntities() {
  if (topdown_context.draw_commands.length == 0) return;

  RHI_BindGraphicsPipeline(topdown_context.command_buffer, topdown_context.entity_pipeline);

  TopDown_DrawCommand* first_draw_command = TopDown_DrawCommandArrayGetPointer(&topdown_context.draw_commands, 0);
  RHI_ShaderArgument arguments[] = {
    {
      .kind = RHI_ShaderArgumentKind_BufferAddress,
      .address = RHI_BufferDeviceAddress(topdown_context.object_buffer) + first_draw_command->object_buffer_offset,
    }
  };
  RHI_BindShaderArguments(topdown_context.command_buffer, RHI_ShaderKind_Vertex, arguments, ArrayLength(arguments));

  for (I32 draw_command_index = 0; draw_command_index < topdown_context.draw_commands.length; draw_command_index += 1) {
    TopDown_DrawCommand* draw_command = TopDown_DrawCommandArrayGetPointer(&topdown_context.draw_commands, draw_command_index);

    RHI_BindIndexBuffer(topdown_context.command_buffer, topdown_context.vertex_buffer, draw_command->index_buffer_offset, RHI_IndexSize_U16);
    RHI_BindVertexBuffer(topdown_context.command_buffer, topdown_context.vertex_buffer, draw_command->vertex_buffer_offset);
    RHI_DrawIndexedPrimitives(topdown_context.command_buffer, draw_command->indecies_count, 1, 0, 0, draw_command_index);
  }
}

func void
TopDown_DrawDebugCollision() {
  for (I32 entity_index = 1; entity_index < topdown_context.entities.length; entity_index += 1) {
    TopDown_Entity* entity = TopDown_EntityArrayGetPointer(&topdown_context.entities, entity_index);
    TopDown_Entity* camera = TopDown_GetEntity(topdown_context.camera_id);

    B32 to_draw = (entity->kind_flags & TopDown_EntityFlag_Collision) && entity->collision.active;
    if (to_draw) {
      TopDown_BoundingBox bounding_box = entity->collision.bounding_box;

      Transform bounding_box_transform = {
        .translation = entity->actor.transform.translation,
        .rotation = entity->actor.transform.rotation,
        .scale = SubVec3F32(bounding_box.max, bounding_box.min),
      };

      struct {
        Mat4F32 transform;
        Mat4F32 camera_transform;
        Vec4F32 rgba;
      } bounding_box_data = {
        .transform = Mat4F32FromTransform(bounding_box_transform),
        .camera_transform = camera->camera.matrix,
        .rgba = MakeVec4F32(0.14f, 0.87f, 0.09f, 0.2f),
      };
      U64 bounding_box_data_offset = RHI_PushBuffer(topdown_context.object_buffer, (U8*)&bounding_box_data, sizeof(bounding_box_data));

      RHI_ShaderArgument arguments[] = {
        {
          .kind = RHI_ShaderArgumentKind_BufferAddress,
          .address = RHI_BufferDeviceAddress(topdown_context.object_buffer) + bounding_box_data_offset,
        }
      };
      RHI_BindGraphicsPipeline(topdown_context.command_buffer, topdown_context.bounding_box_pipeline);
      RHI_BindShaderArguments(topdown_context.command_buffer, RHI_ShaderKind_Vertex|RHI_ShaderKind_Fragment, arguments, ArrayLength(arguments));
      RHI_DrawPrimitives(topdown_context.command_buffer, 36, 1, 0, 0);
    }
  }

#if 0 
  for (I32 entity_index = 1; entity_index < topdown_context.entities.length; entity_index += 1) {
    TopDown_Entity* entity = TopDown_EntityArrayGetPointer(&topdown_context.entities, entity_index);
    TopDown_Entity* camera = TopDown_GetEntity(topdown_context.camera_id);

    B32 to_draw = (entity->kind_flags & TopDown_EntityFlag_Collision) && entity->collision.active;
    if (to_draw) {
      for (AST_GeometryListNode* geometry_node = topdown_context.bounding_box_mesh.mesh.geometry_list.first; geometry_node; geometry_node = geometry_node->next) {
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

        RHI_BindVertexBuffer(topdown_context.command_buffer, topdown_context.frame_buffer, vertex_data_offset);
        RHI_BindIndexBuffer(topdown_context.command_buffer, topdown_context.frame_buffer, index_data_offset, RHI_IndexSize_U16);
        RHI_DrawIndexedPrimitives(topdown_context.command_buffer, geometry->index_count, 1, 0, 0, 0);
      }
    }
  }
#endif
}

func TopDown_EntityId
TopDown_CreateCamera() {
  TopDown_EntityId result = ZeroStruct();

  if (topdown_context.camera_id.id == 0) {
    TopDown_Entity camera = {
      .kind_flags = TopDown_EntityFlag_Camera,
      .camera = {
        .transform = {
          .translation = MakeVec3F32(0.0f, 0.0f, 0.0f),
          .rotation = QuaternionFromEuler(RadiansFromDegrees(80.0f), 0.0f, 0.0f),
        },
        .fov = 90.0f,
        .viewport = (RectI32) {
          .x = 0,
          .y = 0,
          .w = topdown_context.window->size.x,
          .h = topdown_context.window->size.y,
        },
      },
    };

    Vec3F32 camera_front = RotateVec3F32(MakeVec3F32(0.0f, 0.0f, -1.0f), camera.camera.transform.rotation);
    Mat4F32 view_matrix = MakeLookAtMat4F32(camera.camera.transform.translation, AddVec3F32(camera.camera.transform.translation, camera_front), MakeVec3F32(0.0f, 1.0f, 0.0f));
    Mat4F32 projection_matrix = MakePerspectiveMat4F32(
      camera.camera.fov/2.0f, (F32)topdown_context.window->size.x/(F32)topdown_context.window->size.y,
      1.0f, 100.0f
    );
    camera.camera.matrix = MulMat4F32(projection_matrix, view_matrix);
    camera.camera.inverse = InverseMat4F32(camera.camera.matrix);

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
        .bounding_box = TopDown_BoundingBoxFromMesh(&topdown_context.monkey_mesh.mesh),
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
      .bounding_box = TopDown_BoundingBoxFromMesh(&topdown_context.monkey_mesh.mesh),
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
      .bounding_box = TopDown_BoundingBoxFromMesh(&topdown_context.bullet_mesh.mesh),
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
