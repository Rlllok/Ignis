#include "r_vk.h"

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
  R_VK_CreateGraphicsPipeline(&r_vk_state);
  R_VK_CreateBuffer(&r_vk_state);

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

func B32
R_VK_Draw()
{
  R_VK_State* state = &r_vk_state;
  
  U32 image_index;
  R_VK_AcquireNextImage(state, &image_index);
  
  VkCommandBuffer cmd = state->swapchain.frame_resources[image_index].cmd_buffer;

  VkCommandBufferBeginInfo begin_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
  }; 

  VK_CHECK(vkBeginCommandBuffer(cmd, &begin_info))
  {
    R_VK_TransitImageLayout(
      cmd,
      state->swapchain.images[image_index],
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      0,
      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    );

    VkClearValue clear_value = {
      .color = { {0.01f, 0.01f, 0.033f, 1.0f} }
    };

    VkRenderingAttachmentInfo color_attachment = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .imageView = state->swapchain.image_views[image_index],
      .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
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
      .pColorAttachments = &color_attachment
    };

    vkCmdBeginRendering(cmd, &rendering_info);
    {
      vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, state->pipeline.handle);
      
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

      VkDeviceSize offset = { 8 };
      vkCmdBindVertexBuffers(cmd, 0, 1, &state->vertex_buffer.handle, &offset);
      vkCmdBindIndexBuffer(cmd, state->vertex_buffer.handle, 0, VK_INDEX_TYPE_UINT16);

      vkCmdDrawIndexed(cmd, 3, 1, 0, 0, 0);
    }
    vkCmdEndRendering(cmd);

    R_VK_TransitImageLayout(
      cmd,
      state->swapchain.images[image_index],
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      0,
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT
    );
  }
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
    .pSignalSemaphores = &state->swapchain.frame_resources[image_index].release_semaphore
  };

  VK_CHECK(vkQueueSubmit(state->device.graphics_queue, 1, &submit_info, state->swapchain.frame_resources[image_index].submit_fence));
  
  VkPresentInfoKHR present_info = {
    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = &state->swapchain.frame_resources[image_index].release_semaphore,
    .swapchainCount = 1,
    .pSwapchains = &state->swapchain.handle,
    .pImageIndices = &image_index
  };

  VkResult present_result = vkQueuePresentKHR(state->device.graphics_queue, &present_info);

  if (present_result != VK_SUCCESS)
  {
    return false;
  }

  return true;
}
