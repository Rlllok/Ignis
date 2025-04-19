#include "r_vk.h"
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
  
  r_vk_state.geometry_buffer = R_VK_CreateBuffer(Megabytes(256), BUFFER_USAGE_FLAG_VERTEX | BUFFER_USAGE_FLAG_INDEX, BUFFER_PROPERTY_HOST_COHERENT | BUFFER_PROPERTY_HOST_VISIBLE);
  
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
  VkCommandBuffer cmd = state->swapchain.frame_resources[state->current_image_id].cmd_buffer;
  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, state->graphics_pipelines[pipeline->backend_handle].handle);
}

func void
R_VK_BeginFrame()
{
  R_VK_State* state = &r_vk_state;
  
  R_VK_AcquireNextImage(state, &state->current_image_id);
  
  VkCommandBuffer cmd = state->swapchain.frame_resources[state->current_image_id].cmd_buffer;

  VkCommandBufferBeginInfo begin_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
  }; 

  VK_CHECK(vkBeginCommandBuffer(cmd, &begin_info));
  
  R_VK_TransitImageLayout(
    cmd,
    state->swapchain.images[state->current_image_id],
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
  
  VkCommandBuffer cmd = state->swapchain.frame_resources[state->current_image_id].cmd_buffer;
  vkCmdEndRendering(cmd);

  R_VK_TransitImageLayout(
    cmd,
    state->swapchain.images[state->current_image_id],
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
    .pWaitSemaphores = &state->swapchain.frame_resources[state->swapchain.current_index].acquire_semaphore,
    .pWaitDstStageMask = &wait_stage,
    .commandBufferCount = 1,
    .pCommandBuffers = &cmd,
    .signalSemaphoreCount = 1,
    .pSignalSemaphores = &state->swapchain.frame_resources[state->current_image_id].release_semaphore
  };

  VkResult vk_result = (vkQueueSubmit(state->device.graphics_queue, 1, &submit_info, state->swapchain.frame_resources[state->current_image_id].submit_fence));
  VK_CHECK(vk_result);
  
  VkPresentInfoKHR present_info = {
    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = &state->swapchain.frame_resources[state->current_image_id].release_semaphore,
    .swapchainCount = 1,
    .pSwapchains = &state->swapchain.handle,
    .pImageIndices = &state->current_image_id
  };

  VkResult present_result = vkQueuePresentKHR(state->device.graphics_queue, &present_info);

  if (present_result != VK_SUCCESS)
  {
    // return false;
  }
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
    .imageView = state->swapchain.image_views[state->current_image_id],
    .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    .loadOp = R_VK_GetVkAttachmentLoadOperation(load_operation),
    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    .clearValue = clear_value
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
  };

  VkCommandBuffer cmd = state->swapchain.frame_resources[state->current_image_id].cmd_buffer;
  vkCmdBeginRendering(cmd, &rendering_info);
}

func void
R_VK_EndRenderPass()
{
  R_VK_State* state = &r_vk_state;
  
  VkCommandBuffer cmd = state->swapchain.frame_resources[state->current_image_id].cmd_buffer;
  vkCmdEndRendering(cmd);
}

func B32
R_VK_DrawGeometry(AST_Geometry* geometry)
{
  R_VK_State* state = &r_vk_state;
  
  VkCommandBuffer cmd = state->swapchain.frame_resources[state->current_image_id].cmd_buffer;
  
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
  
  vkCmdBindIndexBuffer(cmd, state->geometry_buffer.handle, geometry->index_r_backend_offset, VK_INDEX_TYPE_UINT16);
  
  VkDeviceSize offset = { geometry->vertex_r_backend_offset };
  vkCmdBindVertexBuffers(cmd, 0, 1, &state->geometry_buffer.handle, &offset);
  
  vkCmdDrawIndexed(cmd, geometry->index_count, 1, 0, 0, 0);

  return true;
}
