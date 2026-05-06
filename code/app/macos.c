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

typedef struct AppContext AppContext;
struct AppContext {
  Arena* arena;
  Arena* frame_arena;
  B32    finished;
  F32    dt;

  OS_Window* window;

  RHI_GraphicsPipeline pipeline;
  RHI_Buffer           storage_buffer;
  RHI_Buffer           uniform_buffer;
  RHI_Buffer           materials_buffer;
  RHI_Buffer           entity_datas_buffer;
  RHI_Buffer           arguments_buffer;
  U64                  vertecies_offset;
  U64                  indecies_offset;
} app_context;

func void Draw(RHI_CommandBuffer command_buffer, F32 dt);

typedef struct Material Material;
struct Material {
  Vec4F32 color;
};

typedef struct EntityData EntityData;
struct EntityData {
  Vec4F32 translation;
};

typedef struct Arguments Arguments;
struct Arguments {
  U64 materials;
  U64 entity_datas;
};

I32 main() {
  LogInfo("Hello MacOS\n");

  app_context.arena = AllocateArena(Gigabytes(4), Kilobytes(16));
  app_context.frame_arena = AllocateArena(Gigabytes(4), Kilobytes(16));
  app_context.finished = 0;
  app_context.dt = 0.0f;

  OS_Init(Megabytes(16));

  Vec2U32 window_size = MakeVec2U32(1280, 720);
  app_context.window = OS_CreateWindow(Str8C("Simple Triangle Test (MacOS)"), window_size);

  RHI_Init(app_context.window);

  OS_ShowWindow(app_context.window);

  app_context.storage_buffer = RHI_CreateBuffer(Str8C("StorageBuffer"), Megabytes(16), RHI_BufferUsageFlag_Vertex|RHI_BufferUsageFlag_Index, RHI_BufferPropertyFlag_HostCoherent|RHI_BufferPropertyFlag_HostVisible);
  Vertex vertecies[] = {
    MakeVec3F32(0.0f, 0.5f, 0.0f),
    MakeVec3F32(0.5f, -0.5f, 0.0f),
    MakeVec3F32(-0.5f, -0.5f, 0.0f),
  };
  app_context.vertecies_offset = RHI_PushBuffer(app_context.storage_buffer, (U8*)(vertecies), sizeof(Vertex)*ArrayLength(vertecies));

  app_context.materials_buffer = RHI_CreateBuffer(Str8C("MaterialsBuffer"), Megabytes(16), RHI_BufferUsageFlag_Storage|RHI_BufferUsageFlag_Address, RHI_BufferPropertyFlag_HostCoherent|RHI_BufferPropertyFlag_HostVisible);
  Material materials [] = {
    MakeVec4F32(1.0f, 0.0f, 0.0f, 1.0f),
    MakeVec4F32(0.0f, 1.0f, 0.0f, 1.0f),
    MakeVec4F32(0.0f, 0.0f, 1.0f, 1.0f),
  };
  RHI_PushBuffer(app_context.materials_buffer, (U8*)materials, sizeof(Material)*3);

  app_context.entity_datas_buffer = RHI_CreateBuffer(Str8C("ObjectsDataBuffer"), Megabytes(16), RHI_BufferUsageFlag_Storage|RHI_BufferUsageFlag_Address, RHI_BufferPropertyFlag_HostCoherent|RHI_BufferPropertyFlag_HostVisible);
  EntityData entity_datas[] = {
    {
      .translation = MakeVec4F32(-1.0f, 0.0f, 0.0f, 1.0f),
    },
    {
      .translation = MakeVec4F32(0.0f, 0.0f, 0.0f, 1.0f),
    },
    {
      .translation = MakeVec4F32(1.0f, 0.0f, 0.0f, 1.0f),
    },
  };
  RHI_PushBuffer(app_context.entity_datas_buffer, (U8*)entity_datas, sizeof(EntityData)*3);

  app_context.arguments_buffer = RHI_CreateBuffer(Str8C("ArgumentsBuffer"), Megabytes(16), RHI_BufferUsageFlag_Storage|RHI_BufferUsageFlag_Address, RHI_BufferPropertyFlag_HostCoherent|RHI_BufferPropertyFlag_HostVisible);
  RHI_DeviceAddress materials_buffer_address = RHI_BufferDeviceAddress(app_context.materials_buffer);
  Arguments args = {
    .materials = RHI_BufferDeviceAddress(app_context.materials_buffer),
    .entity_datas = RHI_BufferDeviceAddress(app_context.entity_datas_buffer),
  };
  RHI_PushBuffer(app_context.arguments_buffer, (U8*)&args, sizeof(Arguments));

  U16 indecies[] = {0, 1, 2};
  app_context.indecies_offset = RHI_PushBuffer(app_context.storage_buffer, (U8*)(indecies), sizeof(U16)*ArrayLength(indecies));

  app_context.uniform_buffer = RHI_CreateBuffer(Str8C("UniformBuffer"), Megabytes(16), RHI_BufferUsageFlag_Uniform, RHI_BufferPropertyFlag_HostCoherent|RHI_BufferPropertyFlag_HostVisible);

  RHI_CommandBuffer command_buffer = RHI_GetCommandBuffer();

  RHI_Shader vertex_shader = RHI_CreateShaderNew(
    app_context.arena,
    &(RHI_ShaderCreateInfoNew) {
      .file_name = Str8C("./data/shaders/macos/triangle.vs"),
      .kind = RHI_ShaderKind_Vertex,
      .arguments[0] = {
        .size = sizeof(Arguments),
      }
    }
  );
  RHI_Shader fragment_shader = RHI_CreateShaderNew(
    app_context.arena,
    &(RHI_ShaderCreateInfoNew) {
      .file_name = Str8C("./data/shaders/macos/triangle.fs"),
      .kind = RHI_ShaderKind_Fragment,
    }
  );
  RHI_GraphicsPipelineCreateInfo pipeline_info = {
    .vertex_shader = &vertex_shader,
    .fragment_shader = &fragment_shader,
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
  app_context.pipeline = RHI_CreateGraphicsPipelineNew(&pipeline_info);

  U64 start_ts = OS_GetTimeTicks();
  while (!app_context.finished) {
    OS_EventList event_list = OS_DispatchEvents(app_context.frame_arena, app_context.window);

    if (OS_KeyPressed(OS_KEY_ESC)) {
      app_context.finished = 1;
    }

    Draw(command_buffer, app_context.dt);

    ResetArena(app_context.frame_arena);

    U64 end_ts = OS_GetTimeTicks();
    app_context.dt = ((F32)(end_ts - start_ts))*0.001f;
    start_ts = end_ts;
  }

  return 0;
}

func void
Draw(RHI_CommandBuffer command_buffer, F32 dt) {
  static F32 animation_time = 0.0f;
  static F32 animation_duration = 5.0f;

  if (animation_time > animation_duration) {
    animation_time = 0.0f;
  }

  RHI_BeginCommandBuffer(command_buffer);
    RHI_Texture swapchain_texture = RHI_AcquireSwapchainTexture(command_buffer);

    RHI_ColorTarget color_target = {
      .texture = swapchain_texture,
      .load_operation = RHI_AttachmentLoadOperation_Clear,
      .store_operation = RHI_AttachmentStoreOperation_Store,
      .clear_color = MakeVec4F32(sinf(dt*0.003f), 0.09f, 0.18f, 1.0f),
    };

    RHI_Resource resources[] = {
      {
        .buffer = app_context.materials_buffer,
      },
      {
        .buffer = app_context.entity_datas_buffer,
      },
    };

    RHI_RenderPass* render_pass = RHI_BeginRenderPassNew(command_buffer, 1, &color_target, 0, resources, ArrayLength(resources));
      RectI32 viewport = {
        .x = 0,
        .y = 0,
        .w = app_context.window->size.w,
        .h = app_context.window->size.h,
      };
      RHI_SetViewport(command_buffer, viewport);
      RHI_SetScissor(command_buffer, viewport);
      RHI_BindGraphicsPipeline(command_buffer, app_context.pipeline);
      Arguments args = {
        .materials = RHI_BufferDeviceAddress(app_context.materials_buffer),
        .entity_datas = RHI_BufferDeviceAddress(app_context.entity_datas_buffer),
      };
        RHI_BindShaderArgument(command_buffer, (RHI_ShaderArgument){
          .slot = 4,
          .stage = RHI_ShaderKind_Vertex,
          .buffer = app_context.arguments_buffer,
          .size = sizeof(args),
          .data = (U8*)&args,
        });


        RHI_BindVertexBuffer(command_buffer, app_context.storage_buffer, app_context.vertecies_offset);
        RHI_BindIndexBuffer(command_buffer, app_context.storage_buffer, app_context.indecies_offset, RHI_IndexSize_U16);
        for (I32 i = 0; i < 3; i += 1) {
          RHI_DrawIndexedPrimitives(command_buffer, 3, 1, 0, 0, i);
        }
    RHI_EndRenderPass(command_buffer, render_pass);
  RHI_SubmitCommandBuffer(command_buffer);
}
