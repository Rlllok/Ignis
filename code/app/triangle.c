#include "base/base_include.h"
#include "os/os_include.h"
#include "rhi/rhi_include.h"

#include "base/base_include.c"
#include "os/os_include.c"
#include "rhi/rhi_include.c"

typedef struct Vertex Vertex;
struct Vertex {
  Vec3F32 position;
};

typedef struct TriangleData TriangleData;
struct TriangleData {
  Vec3F32 tint;
  F32 tint_padding;
};

I32 main() {
  LogInfo("Hello\n");

  Arena* arena = AllocateArena(Gigabytes(16), Kilobytes(64));
  B32 finished = 0;

  OS_Init(Megabytes(32));

  Vec2U32 window_size = MakeVec2U32(1280, 720);
  OS_Window* window = OS_CreateWindow(Str8C("Simple Triangle Test"), window_size);

  RHI_Init(window);

  RHI_CommandBuffer command_buffer = RHI_GetCommandBuffer();

  RHI_Buffer storage_buffer = RHI_CreateBuffer(Megabytes(16), RHI_BufferUsageFlag_Vertex|RHI_BufferUsageFlag_Uniform, RHI_BufferPropertyFlag_HostCoherent|RHI_BufferPropertyFlag_HostVisible);
  RHI_Buffer address_buffer = RHI_CreateBuffer(Megabytes(16), RHI_BufferUsageFlag_Address|RHI_BufferUsageFlag_Storage, RHI_BufferPropertyFlag_HostCoherent|RHI_BufferPropertyFlag_HostVisible);
  RHI_DeviceAddress address_buffer_pointer = RHI_BufferDeviceAddress(address_buffer);

  TriangleData triangle_datas[] = {
    { .tint = MakeVec3F32(0.2f, 0.2f, 0.5f) },
    { .tint = MakeVec3F32(0.9f, 0.2f, 0.5f) },
    { .tint = MakeVec3F32(0.2f, 0.7f, 0.5f) },
  };
  U64 triangle_data_offset = RHI_PushBuffer(address_buffer, (U8*)triangle_datas, sizeof(TriangleData)*3);

  RHI_Shader triangle_vertex_shader = RHI_CreateShader(
    arena,
    &(RHI_ShaderCreateInfo) {
      .file_name = Str8C("./data/shaders/triangle.vs"),
      .kind = RHI_ShaderKind_Vertex,
    }
  );
  RHI_Shader triangle_fragment_shader = RHI_CreateShader(
    arena,
    &(RHI_ShaderCreateInfo) {
      .file_name = Str8C("./data/shaders/triangle.fs"),
      .kind = RHI_ShaderKind_Fragment,
      .global_uniforms_count = 1,
    }
  );

  RHI_GraphicsPipelineCreateInfo triangle_pipeline_info = {
    .vertex_shader = triangle_vertex_shader,
    .fragment_shader = triangle_fragment_shader,
    .vertex_attributes_count = 1,
    .vertex_attributes = &(RHI_VertexAttribute) {
      .location = 0,
      .format = RHI_VertexAttributeFormat_Vec3F32,
      .offset = offsetof(Vertex, position),
    },
    .color_targets_count = 1,
    .color_target_infos = &(RHI_GraphicsPipelineColorTargetInfo) {
      .format = RHI_GetSwapchainTextureFormat(),
    },
  };
  RHI_GraphicsPipeline triangle_pipeline = RHI_CreateGraphicsPipeline(&triangle_pipeline_info);

  OS_ShowWindow(window);

  while (!finished) {
    OS_EventList event_list = OS_DispatchEvents(arena, window);

    if (OS_KeyPressed(OS_KEY_ESC)) {
      finished = 1;
    }

    RHI_BeginCommandBuffer(command_buffer);
      RHI_Texture swapchain_texture = RHI_AcquireSwapchainTexture(command_buffer);

      RHI_ColorTarget color_target = {
        .texture = swapchain_texture,
        .load_operation = RHI_AttachmentLoadOperation_Clear,
        .store_operation = RHI_AttachmentStoreOperation_Store,
        .clear_color = MakeVec4F32(0.08f, 0.09f, 0.18f, 1.0f),
      };
      RHI_RenderPass* triangle_pass = RHI_BeginRenderPass(command_buffer, 1, &color_target, 0);
        RHI_BindGraphicsPipeline(command_buffer, triangle_pipeline);
        RectI32 rect = {
          .x = 0,
          .y = 0,
          .w = window_size.x,
          .h = window_size.y,
        };
        RHI_SetViewport(command_buffer, rect);
        RHI_SetScissor(command_buffer, rect);

        Vertex vertecies[] = {
          MakeVec3F32(0.0f, 0.5f, 0.0f),
          MakeVec3F32(0.5f, -0.5f, 0.0f),
          MakeVec3F32(-0.5f, -0.5f, 0.0f),
        };
        U64 vertecies_offset = RHI_PushBuffer(storage_buffer, (U8*)(vertecies), sizeof(Vertex)*ArrayLength(vertecies));

        struct {
          U64 buffer_address;
        } global_fs_data = {
          .buffer_address = address_buffer_pointer + triangle_data_offset,
        };
        U64 global_fs_data_offset = RHI_PushBuffer(storage_buffer, (U8*)&global_fs_data, sizeof(global_fs_data));
        RHI_BindGlobalFragmentShaderData(command_buffer,
          1, &(RHI_UniformBufferBindingInfo) {
            .binding = 0,
            .buffer = storage_buffer,
            .offset = global_fs_data_offset,
            .size = sizeof(global_fs_data),
          },
          0, 0
        );

        RHI_BindVertexBuffer(command_buffer, storage_buffer, 0);
        RHI_DrawPrimitives(command_buffer, ArrayLength(vertecies), 1, 0, 0);
      RHI_EndRenderPass(command_buffer, triangle_pass);
    RHI_SubmitCommandBuffer(command_buffer);
  }

  RHI_Shutdown();

  return 0;
}
