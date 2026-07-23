#include "base/base_include.h"
#include "os/os_include.h"
#include "rhi/rhi_include.h"
#include "assets/animation.h"
#include "assets/mesh.h"

#include "base/base_include.c"
#include "os/os_include.c"
#include "rhi/rhi_include.c"
#include "assets/animation.h"
#include "assets/mesh.c"

typedef struct RB_StaticMesh RB_StaticMesh;
struct RB_StaticMesh {
  AST_StaticMesh mesh;
  U64            vertex_offset[32];
  U64            indecies_offset[32];
};

func RB_StaticMesh RB_LoadStaticMesh(Arena* arena, Str8 path);

typedef struct RB_Material RB_Material;
struct RB_Material {
  Vec3F32 color;
};

typedef struct RB_Camera RB_Camera;
struct RB_Camera {
  Transform transform;
  F32       fov;
  RectI32   viewport;
  Mat4F32   matrix;
  Mat4F32   inverse;
};

func void RB_InitCamera(Vec3F32 position, Quaternion rotation, F32 fov);
func void RB_UpdateCamera(F32 dt);

typedef struct RB_Options RB_Options;
struct RB_Options {
  Vec2U32 resolution;
};

typedef struct RB_Context RB_Context;
struct RB_Context {
  Arena*     global_arena;
  Arena*     frame_arena;
  OS_Window* window;

  RHI_CommandBuffer    rhi_command_buffer;
  RHI_Buffer           rhi_geometry_buffer;
  RHI_Buffer           rhi_draw_data_buffer;
  RHI_Texture          rhi_depth_texture;
  RHI_GraphicsPipeline rhi_default_pipeline;

  RB_Material   default_material;
  RB_StaticMesh sphere_mesh;

  RB_Camera camera;

  B32 finished;

  F32 dt;
} rb_context;

func void RB_Init(RB_Options* options);
func void RB_HandleOSEvents(OS_EventList events);
func void RB_HandleGlobalInput();

func void RB_UpdatePhysics(F32 dt);

func void RB_Render(F32 dt);

I32 main() {
  RB_Options options = {
    .resolution = MakeVec2U32(1280, 720),
  };
  RB_Init(&options);

  Vec3F32 camera_position = MakeVec3F32(0.0f, 5.0f, 0.0f);
  Quaternion camera_rotation = QuaternionLookAt(camera_position, MakeVec3F32(0.0f, 0.0f, 0.0f));
  RB_InitCamera(camera_position, camera_rotation, 80.0f);

  while (!rb_context.finished) {
    RB_HandleOSEvents(OS_DispatchEvents(rb_context.frame_arena, rb_context.window));

    RB_HandleGlobalInput();

    RB_UpdatePhysics(rb_context.dt);
    RB_UpdateCamera(rb_context.dt);
    RB_Render(rb_context.dt);
  }
  ResetArena(rb_context.frame_arena);

  return 0;
}

func RB_StaticMesh
RB_LoadStaticMesh(Arena* arena, Str8 path) {
  RB_StaticMesh result = ZeroStruct();
  result.mesh = AST_LoadStaticMeshFromGLTF(rb_context.global_arena, path);
  I32 geometry_index = 0;
  for (AST_GeometryListNode* geometry_node = result.mesh.geometry_list.first; geometry_node; geometry_node = geometry_node->next) {
    AST_Geometry* geometry = &geometry_node->data;
    result.vertex_offset[geometry_index] = RHI_PushBuffer(rb_context.rhi_geometry_buffer, (U8*)geometry->vertecies, geometry->vertecies_count*sizeof(AST_Vertex));
    result.indecies_offset[geometry_index] = RHI_PushBuffer(rb_context.rhi_geometry_buffer, (U8*)geometry->index_data, geometry->index_count*sizeof(U16));
    geometry_index += 1;
  }
  return result;
}

func void
RB_InitCamera(Vec3F32 position, Quaternion rotation, F32 fov) {
  rb_context.camera = (RB_Camera)ZeroStruct();
  rb_context.camera.transform.translation = position;
  rb_context.camera.transform.rotation = rotation;
  rb_context.camera.transform.scale = MakeVec3F32(1.0f, 1.0f, 1.0f);
  rb_context.camera.fov = fov;
  rb_context.camera.viewport.w = rb_context.window->size.x;
  rb_context.camera.viewport.h = rb_context.window->size.h;
}

func void
RB_UpdateCamera(F32 dt) {
  Vec3F32 camera_front = RotateVec3F32(MakeVec3F32(0.0f, 0.0f, -1.0f), rb_context.camera.transform.rotation);
  Mat4F32 view_matrix = MakeLookAtMat4F32(rb_context.camera.transform.translation, AddVec3F32(rb_context.camera.transform.translation, camera_front), MakeVec3F32(0.0f, 1.0f, 0.0f));
  Mat4F32 projection_matrix = MakePerspectiveMat4F32(rb_context.camera.fov/2.0f, (F32)rb_context.window->size.x/(F32)rb_context.window->size.y, 1.0f, 100.0f);
  rb_context.camera.matrix = MulMat4F32(projection_matrix, view_matrix);
  rb_context.camera.inverse = InverseMat4F32(rb_context.camera.matrix);
}

func void
RB_Init(RB_Options* options) {
  rb_context.global_arena = AllocateArena(Gigabytes(2), Kilobytes(4));
  rb_context.frame_arena = AllocateArena(Gigabytes(2), Kilobytes(4));
  
  OS_Init(Megabytes(16));
  rb_context.window = OS_CreateWindow(Str8C("Rolling Ball"), options->resolution);
  OS_ShowWindow(rb_context.window);

  RHI_Init(rb_context.window);
  rb_context.rhi_command_buffer = RHI_GetCommandBuffer();
  rb_context.rhi_geometry_buffer = RHI_CreateBuffer(Str8C("GeometryBuffer"), Megabytes(64), RHI_BufferUsageFlag_Vertex|RHI_BufferUsageFlag_Index, RHI_BufferPropertyFlag_HostCoherent);
  rb_context.rhi_draw_data_buffer = RHI_CreateBuffer(Str8C("DrawDataBuffer"), Megabytes(64), RHI_BufferUsageFlag_Storage|RHI_BufferUsageFlag_Address, RHI_BufferPropertyFlag_HostCoherent);

  rb_context.rhi_depth_texture = RHI_CreateTexture(&(RHI_TextureCreateInfo) {
    .kind = RHI_TextureKind_2D,
    .format = RHI_TextureFormat_D16_UNORM,
    .usage_flags = RHI_TEXTURE_USAGE_FLAG_DEPTH_STENCIL_ATTACHMENT,
    .width = rb_context.window->size.w,
    .height = rb_context.window->size.h,
    .depth = 1,
    .num_levels = 1,
  });

  // Default Pipeline
  {
    RHI_ShaderArgumentKind arguments[] = {
      RHI_ShaderArgumentKind_BufferAddress,
      RHI_ShaderArgumentKind_BufferAddress,
    };
    RHI_Shader vertex_shader = RHI_CreateShader(
      rb_context.global_arena,
      &(RHI_ShaderCreateInfo){
        .file_name = Str8C("./data/TopDown/Shaders/topdown.vs"),
        .kind = RHI_ShaderKind_Vertex,
        .arguments = arguments,
        .arguments_count = ArrayLength(arguments),
      }
    );
    RHI_Shader fragment_shader = RHI_CreateShader(
      rb_context.global_arena,
      &(RHI_ShaderCreateInfo){
        .file_name = Str8C("./data/TopDown/Shaders/topdown.fs"),
        .kind = RHI_ShaderKind_Fragment,
        .arguments = arguments,
        .arguments_count = ArrayLength(arguments),
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

    rb_context.rhi_default_pipeline = RHI_CreateGraphicsPipeline(
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
          .depth_target_format = RHI_GetTextureFormat(rb_context.rhi_depth_texture),
        },
      }
    );
  }

  // Loading Assets
  rb_context.default_material = (RB_Material) {
    .color = RGBFromHex(0xff0000),
  };
  rb_context.sphere_mesh = RB_LoadStaticMesh(rb_context.global_arena, Str8C("data/TopDown/Models/TopDown_Triangle.gltf"));
}

func void
RB_HandleOSEvents(OS_EventList events) {
  for (OS_EventListNode* node = events.first; node != 0; node = node->next) {
    OS_Event event = node->data;

    switch (event.type) {
      default: {
      } break;
      case OS_EVENT_TYPE_EXIT: {
        rb_context.finished = 1;
      } break;
    }
  }
}

func void
RB_HandleGlobalInput() {
}

func void
RB_UpdatePhysics(F32 dt) {
}

func void
RB_Render(F32 dt) {
  RHI_BeginCommandBuffer(rb_context.rhi_command_buffer); {
    RHI_Texture swapchain_texture = RHI_AcquireSwapchainTexture(rb_context.rhi_command_buffer);
    RHI_ColorTarget color_targets[] = {
      {
        .texture = swapchain_texture,
        .load_operation = RHI_AttachmentLoadOperation_Clear,
        .store_operation = RHI_AttachmentStoreOperation_Store,
        .clear_color = RGBAFromHex(0xffffffff),
      },
    };
    RHI_DepthStencilTarget depth_target = {
      .texture = rb_context.rhi_depth_texture,
      .load_operation = RHI_AttachmentLoadOperation_Clear,
      .store_operation = RHI_AttachmentStoreOperation_DontCare,
      .clear_depth = 0.0f,
    };
    RHI_Resource render_pass_resources[] = {
      {
        .kind = RHI_ResourceKind_Buffer,
        .buffer = rb_context.rhi_geometry_buffer,
      },
      {
        .kind = RHI_ResourceKind_Buffer,
        .buffer = rb_context.rhi_draw_data_buffer,
      },
    };
    RHI_RenderPass* render_pass = RHI_BeginRenderPass(rb_context.rhi_command_buffer, ArrayLength(color_targets), color_targets, &depth_target, render_pass_resources, ArrayLength(render_pass_resources)); {
      RectI32 viewport = {
        .x = 0,
        .y = 0,
        .w = rb_context.window->size.x,
        .h = rb_context.window->size.y,
      };
      RHI_SetViewport(rb_context.rhi_command_buffer, viewport);

      RHI_BindGraphicsPipeline(rb_context.rhi_command_buffer, rb_context.rhi_default_pipeline);
      struct {
        Vec3F32 light_direction; F32 light_direction_padding;
        Vec3F32 light_color; F32 light_color_padding;
      } scene_data = {
        .light_direction = NormalizeVec3F32(MakeVec3F32(0.0f, -1.0f, 1.0f)),
        .light_color = RGBFromHex(0xff0000),
      };
      U64 scene_data_offset = RHI_PushBuffer(rb_context.rhi_draw_data_buffer, (U8*)&scene_data, sizeof(scene_data));
      I32 geometry_index = 0;
      for (AST_GeometryListNode* geometry_node = rb_context.sphere_mesh.mesh.geometry_list.first; geometry_node; geometry_node = geometry_node->next) {
        AST_Geometry* geometry = &geometry_node->data;
        struct {
          Mat4F32     transform;
          Mat4F32     camera_transform;
          RB_Material material;
        } object_data = {
          .transform = Mat4F32FromTransform(IdentityTransform()),
          .camera_transform = rb_context.camera.matrix,
          .material = rb_context.default_material,
        };
        U64 object_data_offset = RHI_PushBuffer(rb_context.rhi_draw_data_buffer, (U8*)&object_data, sizeof(object_data));
        RHI_ShaderArgument arguments[] = {
          {
            .kind = RHI_ShaderArgumentKind_BufferAddress,
            .address = RHI_BufferDeviceAddress(rb_context.rhi_draw_data_buffer) + scene_data_offset,
          },
          {
            .kind = RHI_ShaderArgumentKind_BufferAddress,
            .address = RHI_BufferDeviceAddress(rb_context.rhi_draw_data_buffer) + object_data_offset,
          }
        };
        RHI_BindShaderArguments(rb_context.rhi_command_buffer, RHI_ShaderKind_Vertex|RHI_ShaderKind_Fragment, arguments, ArrayLength(arguments));
        RHI_BindIndexBuffer(rb_context.rhi_command_buffer, rb_context.rhi_geometry_buffer, rb_context.sphere_mesh.indecies_offset[geometry_index], RHI_IndexSize_U16);
        RHI_BindVertexBuffer(rb_context.rhi_command_buffer, rb_context.rhi_geometry_buffer, rb_context.sphere_mesh.vertex_offset[geometry_index]);
        RHI_DrawIndexedPrimitives(rb_context.rhi_command_buffer, geometry->index_count, 1, 0, 0, 0);
        geometry_index += 1;
      }
    }
    RHI_EndRenderPass(rb_context.rhi_command_buffer, render_pass);

    RHI_Present(rb_context.rhi_command_buffer);
  }
  RHI_EndCommandBuffer(rb_context.rhi_command_buffer);
  RHI_SubmitCommandBuffer(rb_context.rhi_command_buffer, 0, 0, 0, 0);
}
