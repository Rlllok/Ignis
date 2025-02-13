#pragma once

#include "../../third_party/vulkan/include/vulkan.h"
#include "../../third_party/vulkan/include/vulkan_win32.h"

#include "r_vk_device.h"
#include "r_vk_pipeline.h"

#define VK_CHECK(expression) Assert(expression != VK_SUCCESS);

#define MAX_PIPELINE_COUNT 10

struct R_VK_State
{
  VkInstance instance;
  VkDebugUtilsMessengerEXT debug_messenger;
  
  R_VK_Device device;

  R_VK_RenderPass render_pass;

  R_VK_Pipeline pipelines[MAX_PIPELINE_COUNT];
  U32 pipelines_count;
} r_vk_state;

func void R_VK_Init(OS_Window* window);
func void R_VK_Destroy();
