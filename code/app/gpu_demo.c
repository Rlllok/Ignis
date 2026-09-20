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

struct {
  Arena* global_arena;
  Arena* frame_arena;

  OS_Window* window;

  RHI_CommandBuffer rhi_command_buffer;
  RHI_ResourceTable rhi_resource_table;
  RHI_Buffer        rhi_transfer_buffer;
  RHI_CommandBuffer rhi_draw_data_buffer;

  B32 finished;
} gpu_demo_context;

I32 main() {
  gpu_demo_context.global_arena = AllocateArena(Gigabytes(1), Kilobytes(4));
  gpu_demo_context.frame_arena = AllocateArena(Gigabytes(1), Kilobytes(4));

  OS_Init(Megabytes(4));
  gpu_demo_context.window = OS_CreateWindow(Str8C("GPU Demo"), MakeVec2U32(1280, 720));
  OS_ShowWindow(gpu_demo_context.window);

  RHI_Init(gpu_demo_context.window);
  gpu_demo_context.rhi_command_buffer = RHI_GetCommandBuffer();
  gpu_demo_context.rhi_resource_table = RHI_CreateResourceTable(gpu_demo_context.global_arena, 16, 1);
  gpu_demo_context.rhi_transfer_buffer = RHI_CreateBuffer(Str8C("TransferBuffer"), Megabytes(128), RHI_BufferUsageFlag_Transfer, RHI_BufferPropertyFlag_HostCoherent|RHI_BufferPropertyFlag_HostVisible);
  gpu_demo_context.rhi_draw_data_buffer = RHI_CreateBuffer(Str8C("DrawBuffer"), Megabytes(16), RHI_BufferUsageFlag_Storage|RHI_BufferUsageFlag_Address, RHI_BufferPropertyFlag_HostCoherent|RHI_BufferPropertyFlag_HostVisible);

  RHI_TextureSampler test_sampler = RHI_CreateTextureSampler(
    &(RHI_TextureSamplerCreateInfo){
      .mag_filter = RHI_FilterKind_Linear,
      .min_filter = RHI_FilterKind_Linear,
      .address_mode_u = RHI_SamplerAddressMode_Repeat,
      .address_mode_v = RHI_SamplerAddressMode_Repeat,
      .address_mode_w = RHI_SamplerAddressMode_Repeat,
      .mipmap_mode = RHI_SamplerMipmapMode_Linear,
    }
  );
  RHI_SamplerDeviceId test_sampler_id = RHI_ResourceTableAddSampler(gpu_demo_context.rhi_resource_table, test_sampler);

  I32 tex_width = 0;
  I32 tex_height = 0;
  I32 tex_channels = 0;
  U8* tex_pixels = stbi_load("data/asteroids/Asteroids.png", &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);

  RHI_Texture test_texture = RHI_CreateTexture(&(RHI_TextureCreateInfo) {
    .kind = RHI_TextureKind_2D,
    .format = RHI_TextureFormat_R8G8B8A8_UNORM,
    .usage_flags = RHI_TEXTURE_USAGE_FLAG_SAMPLED | RHI_TEXTURE_USAGE_FLAG_TRANSFER_DST,
    .width = tex_width,
    .height = tex_height,
    .depth = 1,
    .num_levels = 1,
  });
  U64 texture_offset = RHI_PushBuffer(gpu_demo_context.rhi_transfer_buffer, tex_pixels, tex_width*tex_height*4);
  RHI_CommandBuffer transfer_command_buffer = ZeroStruct();
  RHI_BeginCommandBuffer(transfer_command_buffer); {
    RHI_CopyBufferToTexture(transfer_command_buffer, gpu_demo_context.rhi_transfer_buffer, texture_offset, test_texture);
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
  RHI_TextureDeviceId test_texture_device_id = RHI_ResourceTableAddTexture(gpu_demo_context.rhi_resource_table, test_texture);

  D_Init(Megabytes(4));

  while (!gpu_demo_context.finished) {
    OS_DispatchEvents(gpu_demo_context.frame_arena, gpu_demo_context.window);

    if (OS_KeyPressed(OS_KEY_ESC)) {
      gpu_demo_context.finished = 1;
    }

    RHI_BeginCommandBuffer(gpu_demo_context.rhi_command_buffer); {
      RHI_Texture swapchain_texture = RHI_AcquireSwapchainTexture(gpu_demo_context.rhi_command_buffer);
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
      RHI_BeginRenderPass(gpu_demo_context.rhi_command_buffer, render_pass_info); {
        RHI_BindResourceTable(gpu_demo_context.rhi_command_buffer, gpu_demo_context.rhi_resource_table);
        D_DrawRect(gpu_demo_context.rhi_command_buffer, gpu_demo_context.rhi_draw_data_buffer, MakeRectF32(50.0f, 50.0f, 100.0f, 100.0f), MakeVec4F32(5.0f, 5.0f, 5.0f, 5.0f), MakeVec4F32(1.0f, 0.0f, 0.0f, 1.0f));
        D_DrawSprite(gpu_demo_context.rhi_command_buffer, gpu_demo_context.rhi_draw_data_buffer, MakeRectF32(300.0, 20.0f, 400.0, 400.0f), 0.0f, test_texture_device_id, MakeVec4F32(1.0f, 1.0f, 1.0f, 1.0f));
      }
      RHI_EndRenderPass(gpu_demo_context.rhi_command_buffer);

      RHI_Present(gpu_demo_context.rhi_command_buffer);
    }
    RHI_EndCommandBuffer(gpu_demo_context.rhi_command_buffer);
    RHI_SubmitCommandBuffer(gpu_demo_context.rhi_command_buffer, 0, 0, 0, 0);

    ResetArena(gpu_demo_context.frame_arena);
  }

  return 0;
}
