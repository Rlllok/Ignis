#include "render/vulkan/r_vk.h"
#include "base/base_math.h"
#include "base/base_string.h"
#include "r_vk.h"
#include "render/r_core.h"
#include "third_party/vulkan/include/vulkan_core.h"

func B32
R_VK_Init(OS_Window* window)
{
  r_vk_state.arena = AllocateArena(Megabytes(256));
  
  LOG_INFO("Create Instance\n")
  
  VkApplicationInfo app_info = {};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "VulkanRenderingFramework";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "RenderingEngine";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_API_VERSION_1_3;

  const char* extension_names[] = {
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
    "VK_KHR_surface",
    "VK_KHR_win32_surface",
  };

#if IGNIS_DEBUG
  const char* validation_layers[] = {
    "VK_LAYER_KHRONOS_validation",
    "VK_LAYER_LUNARG_monitor",
  };
#else
  const char* validation_layers[] = {};
#endif // IGNIS_DEBUG

  VkInstanceCreateInfo instance_info = {};
  instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_info.pApplicationInfo = &app_info;
  instance_info.enabledLayerCount = CountArrayElements(validation_layers);
  instance_info.ppEnabledLayerNames = validation_layers;
  instance_info.enabledExtensionCount = CountArrayElements(extension_names);
  instance_info.ppEnabledExtensionNames = extension_names;

#if IGNIS_DEBUG
  VkDebugUtilsMessengerCreateInfoEXT messenger_info;
  R_VK_PopulateDebugMessengerCreateInfo(messenger_info);
  instance_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&messenger_info;
#endif // IGNIS_DEBUG

  VK_CHECK(vkCreateInstance(&instance_info, 0, &r_vk_state.instance));

#if IGNIS_DEBUG
  VK_CHECK(R_VK_CreateDebugMessenger(r_vk_state.instance, &r_vk_state.debug_messenger));
#endif // IGNIS_DEBUG

  R_VK_CreateDevice(&r_vk_state);
  R_VK_CreateSurface(&r_vk_state, window);
  R_VK_CreateSwapchain(&r_vk_state);
  
  VkCommandPoolCreateInfo cmd_pool_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
    .queueFamilyIndex = r_vk_state.device.graphics_queue_index
  };
  VK_CHECK(vkCreateCommandPool(r_vk_state.device.logical, &cmd_pool_info, 0, &r_vk_state.cmd_pool));
  
  r_vk_state.geometry_buffer = R_VK_CreateBuffer(Megabytes(256), BUFFER_USAGE_FLAG_UNIFORM | BUFFER_USAGE_FLAG_VERTEX | BUFFER_USAGE_FLAG_INDEX, BUFFER_PROPERTY_HOST_COHERENT | BUFFER_PROPERTY_HOST_VISIBLE);
  r_vk_state.staging_buffer = R_VK_CreateBuffer(Megabytes(64), BUFFER_USAGE_FLAG_TRANSFER_SRC, BUFFER_PROPERTY_HOST_COHERENT | BUFFER_PROPERTY_HOST_VISIBLE);

  R_VK_CreateDescriptorPool();
  R_VK_CreateDescriptorSet();
  R_VK_WriteDescriptorSet();

  R_VK_CreateTexture(Str8FromC("data/uv_checker.png"));
  
  return true;
}

func B32
R_VK_Shutdown()
{
  R_VK_DestroyBuffer(&r_vk_state);
  R_VK_DestroyPipeline(&r_vk_state);
  R_VK_DestroySwapchain(&r_vk_state);
  R_VK_DestroySurface(&r_vk_state);
  R_VK_DestroyDevice(&r_vk_state);
  
#if IGNIS_DEBUG
  R_VK_DestroyDebugUtilsMessenger(r_vk_state.instance, r_vk_state.debug_messenger, 0);
#endif // IGNIS_DEBUG

  vkDestroyInstance(r_vk_state.instance, 0);

  return true;
}

func void R_VK_PushGeometry(AST_Geometry* geometry)
{
  R_VK_State* state = &r_vk_state;
  
  void* data;
  vkMapMemory(state->device.logical, state->geometry_buffer.memory, state->geometry_buffer.size, VK_WHOLE_SIZE, 0, &data);
  {
    U64 offset = 0;
    
    U64 index_data_size = geometry->index_size * geometry->index_count;
    memcpy(data, geometry->index_data, index_data_size);
    geometry->index_r_backend_offset = state->geometry_buffer.size;
    offset += index_data_size + index_data_size%4;
    state->geometry_buffer.size += offset;
    
    U64 vertex_data_size = geometry->vertex_size * geometry->vertex_count;
    memcpy((U8*)data + offset, geometry->vertex_data, vertex_data_size);
    geometry->vertex_r_backend_offset = state->geometry_buffer.size;
    offset += vertex_data_size + vertex_data_size%4;
    state->geometry_buffer.size += offset;
  }
  vkUnmapMemory(state->device.logical, state->geometry_buffer.memory);
}

func void
R_VK_BindPipeline(R_Pipeline* pipeline)
{
  R_VK_State* state = &r_vk_state;
  
  R_VK_GraphicsPipeline vk_pipeline = state->graphics_pipelines[pipeline->backend_handle];
  VkCommandBuffer cmd = state->swapchain.frame_resources[state->current_frame].cmd_buffer;
}

func void
R_VK_BeginFrame()
{
  R_VK_State* state = &r_vk_state;
  
  vkWaitForFences(state->device.logical, 1, &state->swapchain.frame_resources[r_vk_state.current_frame].submit_fence, VK_TRUE, U64_MAX);
  
  R_VK_AcquireNextImage(state, &r_vk_state.current_target);
  state->geometry_buffer.size = 0;
  vkResetDescriptorPool(r_vk_state.device.logical, r_vk_state.descriptor_pools[r_vk_state.current_frame], 0);
  
  vkResetFences(state->device.logical, 1, &state->swapchain.frame_resources[r_vk_state.current_frame].submit_fence);

  vkResetCommandPool(state->device.logical, state->swapchain.frame_resources[r_vk_state.current_frame].cmd_pool, 0);
  
  VkCommandBufferBeginInfo begin_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
  }; 

  VkCommandBuffer cmd = state->swapchain.frame_resources[state->current_frame].cmd_buffer;
  VK_CHECK(vkBeginCommandBuffer(cmd, &begin_info));
  
  R_VK_TransitImageLayout(
    cmd,
    state->swapchain.images[r_vk_state.current_target],
    VK_IMAGE_LAYOUT_UNDEFINED,
    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    0,
    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    VK_IMAGE_ASPECT_COLOR_BIT
  );
}

func void
R_VK_EndFrame()
{
  R_VK_State* state = &r_vk_state;
  
  VkCommandBuffer cmd = state->swapchain.frame_resources[state->current_frame].cmd_buffer;
  vkCmdEndRendering(cmd);
  
  R_VK_TransitImageLayout(
    cmd,
    state->swapchain.images[state->current_target],
    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    0,
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
    VK_IMAGE_ASPECT_COLOR_BIT
  );

  VK_CHECK(vkEndCommandBuffer(cmd));

  VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

  VkSubmitInfo submit_info = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = &state->swapchain.frame_resources[state->current_frame].acquire_semaphore,
    .pWaitDstStageMask = &wait_stage,
    .commandBufferCount = 1,
    .pCommandBuffers = &cmd,
    .signalSemaphoreCount = 1,
    .pSignalSemaphores = &state->swapchain.frame_resources[state->current_frame].release_semaphore
  };

  VkResult vk_result = (vkQueueSubmit(state->device.graphics_queue, 1, &submit_info, state->swapchain.frame_resources[state->current_frame].submit_fence));
  VK_CHECK(vk_result);
  
  VkPresentInfoKHR present_info = {
    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = &state->swapchain.frame_resources[state->current_frame].release_semaphore,
    .swapchainCount = 1,
    .pSwapchains = &state->swapchain.handle,
    .pImageIndices = &state->current_target
  };

  VkResult present_result = vkQueuePresentKHR(state->device.graphics_queue, &present_info);

  if (present_result != VK_SUCCESS)
  {
    // return false;
  }

  r_vk_state.current_frame = (r_vk_state.current_frame + 1) % FRAMES_IN_FLIGHT;
}

func void
R_VK_BeginRenderPass(R_AttachmentLoadOperation load_operation, Vec4f clear_color)
{
  R_VK_State* state = &r_vk_state;
  
  VkClearValue clear_value = {};
  clear_value.color.float32[0] = clear_color.r;
  clear_value.color.float32[1] = clear_color.g;
  clear_value.color.float32[2] = clear_color.b;
  clear_value.color.float32[3] = clear_color.a;

  VkRenderingAttachmentInfo color_attachment = {
    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
    .imageView = state->swapchain.image_views[state->current_target],
    .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    .loadOp = R_VK_GetVkAttachmentLoadOperation(load_operation),
    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    .clearValue = clear_value
  };

  VkRenderingAttachmentInfo depth_attachment = {
    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
    .imageView = state->swapchain.depth_image_view,
    .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
    .loadOp = R_VK_GetVkAttachmentLoadOperation(load_operation),
    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    .clearValue = {
      .depthStencil = {
        .depth = 0.0
      }
    }
  };

  VkExtent2D render_area = {
    .width = state->swapchain.size.width,
    .height = state->swapchain.size.height
  };
  VkRenderingInfo rendering_info = {
    .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
    .renderArea = {
      .offset = { 0, 0 },
      .extent = render_area
    },
    .layerCount = 1,
    .colorAttachmentCount = 1,
    .pColorAttachments = &color_attachment,
    .pDepthAttachment = &depth_attachment
  };
  
  VkCommandBuffer cmd = state->swapchain.frame_resources[state->current_frame].cmd_buffer;
  R_VK_TransitImageLayout(
    cmd,
    state->swapchain.depth_image,
    VK_IMAGE_LAYOUT_UNDEFINED,
    VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
    0,
    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
    VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
    VK_IMAGE_ASPECT_DEPTH_BIT
  );

  vkCmdBeginRendering(cmd, &rendering_info);
}

func void
R_VK_EndRenderPass()
{
  R_VK_State* state = &r_vk_state;
  
  VkCommandBuffer cmd = state->swapchain.frame_resources[state->current_frame].cmd_buffer;
  R_VK_TransitImageLayout(
    cmd,
    state->swapchain.depth_image,
    VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
    VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
    VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
    VK_IMAGE_ASPECT_DEPTH_BIT
  );
  
  vkCmdEndRendering(cmd);
}

func B32
R_VK_DrawGeometry(R_DrawGeometryInfo* draw_info)
{
  R_VK_State* state = &r_vk_state;
  
  VkCommandBuffer cmd = state->swapchain.frame_resources[state->current_frame].cmd_buffer;
  
  VkExtent2D render_area = {
    .width = state->swapchain.size.width,
    .height = state->swapchain.size.height
  };
    
  VkViewport viewport = {
    .width = (F32)render_area.width,
    .height = (F32)render_area.height,
    .minDepth = 0.0f,
    .maxDepth = 1.0f
  };
  vkCmdSetViewport(cmd, 0, 1, &viewport);

  VkRect2D scissor = {
    .extent = {
      .width = render_area.width,
      .height = render_area.height
    }
  };
  vkCmdSetScissor(cmd, 0, 1, &scissor);
  
  U64 uniform_buffer_offset = 0;
  
  R_VK_GraphicsPipeline vk_pipeline = state->graphics_pipelines[draw_info->pipeline->backend_handle];
  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline.handle);
  
  r_vk_state.geometry_buffer.size = (r_vk_state.geometry_buffer.size + 64) - r_vk_state.geometry_buffer.size%64;
  
  void* data;
  vkMapMemory(r_vk_state.device.logical, r_vk_state.geometry_buffer.memory, r_vk_state.geometry_buffer.size, VK_WHOLE_SIZE, 0, &data);
  {
    uniform_buffer_offset = r_vk_state.geometry_buffer.size;
    
    U64 uniform_data_size = draw_info->uniform_data_size;
    memcpy((U8*)data, draw_info->uniform_data, uniform_data_size);
    U32 offset = (uniform_data_size + 64) - (uniform_data_size%64);
    r_vk_state.geometry_buffer.size += offset;
  }
  vkUnmapMemory(r_vk_state.device.logical, r_vk_state.geometry_buffer.memory);
  
  VkDescriptorSetAllocateInfo allocate_info = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
    .descriptorPool = state->descriptor_pools[r_vk_state.current_frame],
    .descriptorSetCount = 1,
    .pSetLayouts = &state->descriptor_layout
  };

  VkDescriptorSet set = {};
  VK_CHECK(vkAllocateDescriptorSets(state->device.logical, &allocate_info, &set));
  
  VkDescriptorBufferInfo buffer_info = {
    .buffer = r_vk_state.geometry_buffer.handle,
    .offset = uniform_buffer_offset,
    .range = draw_info->uniform_data_size
  };
  
  VkDescriptorImageInfo image_info = {
    .sampler = r_vk_state.default_sampler,
    .imageView = r_vk_state.default_texture.view,
    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
  };
  
  VkWriteDescriptorSet write_infos[2] = {};
  write_infos[0] = {
    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    .dstSet = set,
    .dstBinding = 0,
    .dstArrayElement = 0,
    .descriptorCount = 1,
    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    .pBufferInfo = &buffer_info
  };
  write_infos[1] = {
    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    .dstSet = set,
    .dstBinding = 1,
    .dstArrayElement = 0,
    .descriptorCount = 1,
    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
    .pImageInfo = &image_info
  };
  
  vkUpdateDescriptorSets(r_vk_state.device.logical, 2, write_infos, 0, 0);
  vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline.layout, 0, 1, &set, 0, 0);

  vkCmdBindIndexBuffer(cmd, state->geometry_buffer.handle, draw_info->geometry->index_r_backend_offset, VK_INDEX_TYPE_UINT16);
  
  VkDeviceSize offset = { draw_info->geometry->vertex_r_backend_offset };
  vkCmdBindVertexBuffers(cmd, 0, 1, &state->geometry_buffer.handle, &offset);
  
  vkCmdDrawIndexed(cmd, draw_info->geometry->index_count, 1, 0, 0, 0);

  return true;
}

func void
R_VK_CreateDescriptorPool()
{
  R_VK_State* state = &r_vk_state;
  
  VkDescriptorPoolSize pool_sizes[2] = {};
  pool_sizes[0] = {
    .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    .descriptorCount = 1
  };
  
  pool_sizes[1] = {
    .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
    .descriptorCount = 1
  };
  
  VkDescriptorPoolCreateInfo pool_info = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
    .maxSets = 1024,
    .poolSizeCount = 2,
    .pPoolSizes = pool_sizes
  };

  for (I32 i = 0; i < FRAMES_IN_FLIGHT; i += 1)
  {
    VK_CHECK(vkCreateDescriptorPool(state->device.logical, &pool_info, 0, &state->descriptor_pools[i]));
  }
}

func void
R_VK_CreateDescriptorSet()
{
  R_VK_State* state = &r_vk_state;

  VkDescriptorSetLayoutBinding bindings[2] = {};
  bindings[0] = {
    .binding = 0,
    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    .descriptorCount = 1,
    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT
  };
  bindings[1] = {
    .binding = 1,
    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
    .descriptorCount = 1,
    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
  };
  
  VkDescriptorSetLayoutCreateInfo layout = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
    .bindingCount = 2,
    .pBindings = bindings
  };
  VK_CHECK(vkCreateDescriptorSetLayout(state->device.logical, &layout, 0, &state->descriptor_layout));
  
  VkDescriptorSetAllocateInfo allocate_info = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
    .descriptorPool = state->descriptor_pools[r_vk_state.current_frame],
    .descriptorSetCount = 1,
    .pSetLayouts = &state->descriptor_layout
  };

  VK_CHECK(vkAllocateDescriptorSets(state->device.logical, &allocate_info, &state->descriptor_set));
}

func void
R_VK_WriteDescriptorSet()
{
}

func VkCommandBuffer
R_VK_BeginSingleCmd()
{
  VkCommandBufferAllocateInfo allocate_info = {};
  allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocate_info.commandPool = r_vk_state.cmd_pool;
  allocate_info.commandBufferCount = 1;

  VkCommandBuffer command_buffer;
  VK_CHECK(vkAllocateCommandBuffers(r_vk_state.device.logical, &allocate_info, &command_buffer));

  VkCommandBufferBeginInfo begin_info = {};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  VK_CHECK(vkBeginCommandBuffer(command_buffer, &begin_info));

  return command_buffer; 
}

func void
R_VK_EndSingleCmd(VkCommandBuffer cmd)
{
  VK_CHECK(vkEndCommandBuffer(cmd));

  VkSubmitInfo submit_info = {};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &cmd;

  VK_CHECK(vkQueueSubmit(r_vk_state.device.graphics_queue, 1, &submit_info, 0));
  VK_CHECK(vkQueueWaitIdle(r_vk_state.device.graphics_queue));

  vkFreeCommandBuffers(r_vk_state.device.logical, r_vk_state.cmd_pool, 1, &cmd);
}

func R_Texture
R_VK_CreateTexture(Str8 path)
{
  R_Texture texture = {};

  I32 tex_width    = 0;
  I32 tex_height   = 0;
  I32 tex_channels = 0;
  U8* tex_pixels   = stbi_load(CFromStr8(path), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);

  if (!tex_pixels)
  {
    LOG_ERROR("Cannot load texture %s\n", path);
  }

  texture.size = tex_width * tex_height * 4;

  void* data;
  vkMapMemory(r_vk_state.device.logical, r_vk_state.staging_buffer.memory, 0, VK_WHOLE_SIZE, 0, &data);
    memcpy(data, tex_pixels, texture.size);
  vkUnmapMemory(r_vk_state.device.logical, r_vk_state.staging_buffer.memory);

  stbi_image_free(tex_pixels);

  VkImageCreateInfo image_info = {};
  image_info.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image_info.samples       = VK_SAMPLE_COUNT_1_BIT;
  image_info.imageType     = VK_IMAGE_TYPE_2D;
  image_info.extent.width  = tex_width;
  image_info.extent.height = tex_height;
  image_info.extent.depth  = 1;
  image_info.mipLevels     = 1;
  image_info.arrayLayers   = 1;
  image_info.format        = VK_FORMAT_R8G8B8A8_SRGB;
  image_info.tiling        = VK_IMAGE_TILING_OPTIMAL;
  image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_info.usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  image_info.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
  image_info.samples       = VK_SAMPLE_COUNT_1_BIT;
  if (vkCreateImage(r_vk_state.device.logical, &image_info, 0, &r_vk_state.default_texture.image) != VK_SUCCESS)
  {
    LOG_ERROR("Cannot create Image for Texture.\n");
    return texture;
  }

  VkMemoryRequirements mem_requirements = {};
  vkGetImageMemoryRequirements(r_vk_state.device.logical, r_vk_state.default_texture.image, &mem_requirements);

  VkMemoryAllocateInfo mem_info = {};
  mem_info.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  mem_info.allocationSize  = mem_requirements.size;
  mem_info.memoryTypeIndex = R_VK_FindMemoryTypeIndex(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  VK_CHECK(vkAllocateMemory(r_vk_state.device.logical, &mem_info, 0, &r_vk_state.default_texture.memory));

  VK_CHECK(vkBindImageMemory(r_vk_state.device.logical, r_vk_state.default_texture.image, r_vk_state.default_texture.memory, 0));

  VkCommandBuffer cmd = R_VK_BeginSingleCmd();
  {
    R_VK_TransitImageLayout(
      cmd,
      r_vk_state.default_texture.image,
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      0,
      VK_ACCESS_TRANSFER_WRITE_BIT,
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
      VK_PIPELINE_STAGE_TRANSFER_BIT,
      VK_IMAGE_ASPECT_COLOR_BIT
    );
    
    VkBufferImageCopy copy_info = {};
    copy_info.bufferOffset                    = 0;
    copy_info.bufferRowLength                 = 0;
    copy_info.bufferImageHeight               = 0;
    copy_info.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_info.imageSubresource.mipLevel       = 0;
    copy_info.imageSubresource.baseArrayLayer = 0;
    copy_info.imageSubresource.layerCount     = 1;
    copy_info.imageOffset                     = { 0, 0, 0 };
    copy_info.imageExtent                     = { (U32)tex_width, (U32)tex_height, 1 };
    vkCmdCopyBufferToImage(cmd, r_vk_state.staging_buffer.handle, r_vk_state.default_texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_info);
    
    R_VK_TransitImageLayout(
      cmd,
      r_vk_state.default_texture.image,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      VK_ACCESS_TRANSFER_WRITE_BIT,
      VK_ACCESS_SHADER_READ_BIT,
      VK_PIPELINE_STAGE_TRANSFER_BIT,
      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
      VK_IMAGE_ASPECT_COLOR_BIT
    );
    R_VK_EndSingleCmd(cmd);
  }
  
  // AlNov: Create Texture Image View
  VkImageViewCreateInfo view_info = {};
  view_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  view_info.image                           = r_vk_state.default_texture.image;
  view_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
  view_info.format                          = VK_FORMAT_R8G8B8A8_SRGB;
  view_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
  view_info.subresourceRange.baseMipLevel   = 0;
  view_info.subresourceRange.levelCount     = 1;
  view_info.subresourceRange.baseArrayLayer = 0;
  view_info.subresourceRange.layerCount     = 1;

  VK_CHECK(vkCreateImageView(r_vk_state.device.logical, &view_info, 0, &r_vk_state.default_texture.view));

  // AlNov: Create Texture Sampler
  VkSamplerCreateInfo sampler_info = {};
  sampler_info.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  sampler_info.magFilter               = VK_FILTER_LINEAR;
  sampler_info.minFilter               = VK_FILTER_LINEAR;
  sampler_info.addressModeU            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
  sampler_info.addressModeV            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
  sampler_info.addressModeW            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
  sampler_info.anisotropyEnable        = VK_FALSE;
  sampler_info.borderColor             = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
  sampler_info.unnormalizedCoordinates = VK_FALSE;
  sampler_info.compareEnable           = VK_FALSE;
  sampler_info.compareOp               = VK_COMPARE_OP_ALWAYS;
  sampler_info.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  sampler_info.mipLodBias              = 0.0f;
  sampler_info.minLod                  = 0.0f;
  sampler_info.maxLod                  = 0.0f;

  VK_CHECK(vkCreateSampler(r_vk_state.device.logical, &sampler_info, 0, &r_vk_state.default_sampler));

  return texture;  
}
