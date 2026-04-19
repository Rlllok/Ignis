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

typedef struct TopDown_Camera TopDown_Camera;
struct TopDown_Camera {
  Vec3F32 position;
  F32     yaw;
  F32     pitch;

  Vec3F32 front;
  Vec3F32 right;
  Vec3F32 up;
};

typedef struct TopDown_Context TopDown_Context;
struct TopDown_Context {
  Arena*     global_arena;
  Arena*     frame_arena;

  OS_Window* window;

  // RHI Objects
  RHI_CommandBuffer    command_buffer;
  RHI_Buffer           frame_buffer;
  RHI_Texture          depth_texture;
  RHI_GraphicsPipeline pipeline;

  // State
  B32 finished;

  // Assets
  AST_StaticMesh cube;

  // Game Objects
  TopDown_Camera camera;
} topdown_context = ZeroStruct();

I32 main() {
  topdown_context.global_arena = AllocateArena(Gigabytes(16), Kilobytes(64));
  topdown_context.frame_arena = AllocateArena(Gigabytes(16), Kilobytes(64));

  OS_Init(Megabytes(64));
  topdown_context.window = OS_CreateWindow(Str8C("TopDown"), MakeVec2U32(1280, 720));

  RHI_Init(topdown_context.window);
  // Init RHI Objects
  topdown_context.command_buffer = RHI_GetCommandBuffer();
  topdown_context.frame_buffer = RHI_CreateBuffer(Megabytes(16), RHI_BufferUsageFlag_Vertex|RHI_BufferUsageFlag_Index|RHI_BufferUsageFlag_Uniform, RHI_BufferPropertyFlag_HostCoherent);

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
      .file_name = Str8C("./data/shaders/topdown/topdown.vs"),
      .kind = RHI_ShaderKind_Vertex,
      .instance_uniforms_count = 1,
    }
  );
  RHI_Shader fragment_shader = RHI_CreateShader(
    topdown_context.global_arena,
    &(RHI_ShaderCreateInfo){
      .file_name = Str8C("./data/shaders/topdown/topdown.fs"),
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

  topdown_context.pipeline = RHI_CreateGraphicsPipeline(
    &(RHI_GraphicsPipelineCreateInfo) {
      .vertex_shader = vertex_shader,
      .fragment_shader = fragment_shader,
      .vertex_attributes_count = ArrayLength(vertex_attributes),
      .vertex_attributes = vertex_attributes,
      .color_targets_count = 1,
      .color_target_infos = &(RHI_GraphicsPipelineColorTargetInfo){
        .format = RHI_GetSwapchainTextureFormat(),
      },
      .depth_stencil_state = (RHI_PipelineDepthStencilState) {
        .depth_test_enable = 1,
        .depth_write_enable = 1,
        .depth_compare_operation = RHI_CompareOperation_Less,
        .depth_target_format = RHI_GetTextureFormat(topdown_context.depth_texture),
      },
    }
  );

  OS_ShowWindow(topdown_context.window);

  // Load Assets
  topdown_context.cube = AST_LoadStaticMeshFromGLTF(topdown_context.global_arena, Str8C("data/primitives/cube.gltf"));

  // Init Game Objects
  topdown_context.camera = (TopDown_Camera){
    .position = MakeVec3F32(1.0f, 5.0f, 10.0f),
    .yaw = -90.0f,
    .pitch = -30.0f,
    .front = MakeVec3F32(1.0f, 0.0f, -1.0f),
    .right = MakeVec3F32(1.0f, 0.0f, 1.0f),
    .up = MakeVec3F32(0.0f, 1.0f, 0.0f),
  };

  while (!topdown_context.finished) {
    OS_EventList events = OS_DispatchEvents(topdown_context.frame_arena, topdown_context.window);

    if (OS_KeyPressed(OS_KEY_ESC)) {
      topdown_context.finished = 1;
    }

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
      .clear_depth = 1.0f,
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

        for (AST_GeometryListNode* geometry_node = topdown_context.cube.geometry_list.first; geometry_node; geometry_node = geometry_node->next) {
          AST_Geometry* geometry = &geometry_node->data;
          
          Mat4F32 view_matrix = MakeLookAtMat4F32(topdown_context.camera.position, MakeVec3F32(0.0f, 0.0f, 0.0f), MakeVec3F32(0.0f, 1.0f, 0.0f));

          Mat4F32 projection_matrix = MakePerspectiveMat4F32(
            45.0f, (F32)topdown_context.window->size.x/(F32)topdown_context.window->size.y,
            0.1f, 100.0f
          );

          struct {
            Mat4F32 mvp;
          } instance_vs_data = {
            .mvp = MulMat4F32(projection_matrix, MulMat4F32(view_matrix, MakeTransposeMat4F32(MakeVec3F32(1.0f, 1.0f, 1.0f)))),
          };

          U64 instance_vs_data_offset = RHI_PushBuffer(topdown_context.frame_buffer, (U8*)&instance_vs_data, sizeof(instance_vs_data));
          U64 vertex_data_offset = RHI_PushBuffer(topdown_context.frame_buffer, (U8*)geometry->vertecies, geometry->vertecies_count*sizeof(AST_Vertex));
          U64 index_data_offset = RHI_PushBuffer(topdown_context.frame_buffer, (U8*)geometry->index_data, geometry->index_size*geometry->index_count);

          RHI_BindGraphicsPipeline(topdown_context.command_buffer, topdown_context.pipeline);
          RHI_BindInstanceVertexShaderData(topdown_context.command_buffer, 1, &(RHI_UniformBufferBindingInfo){
            .buffer = topdown_context.frame_buffer,
            .offset = instance_vs_data_offset,
            .size = sizeof(instance_vs_data),
          },
          0, 0);
          RHI_BindVertexBuffer(topdown_context.command_buffer, topdown_context.frame_buffer, vertex_data_offset);
          RHI_BindIndexBuffer(topdown_context.command_buffer, topdown_context.frame_buffer, index_data_offset, RHI_IndexSize_U16);
          RHI_DrawIndexedPrimitives(topdown_context.command_buffer, geometry->index_count, 1, 0, 0, 0);
        }
      RHI_EndRenderPass(topdown_context.command_buffer, render_pass);
    RHI_SubmitCommandBuffer(topdown_context.command_buffer);
  }

  return 0;
}
