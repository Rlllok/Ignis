#pragma once

#include "base/base_include.h"

#include "third_party/vulkan/include/vulkan.h"

struct R_VK_Device
{
  VkDevice logical;
  VkPhysicalDevice physical;

  U32 graphics_queue_index;
  VkQueue graphics_queue;
};

func void R_VK_CreateDevice(struct R_VK_State* state);
func void R_VK_DestroyDevice(struct R_VK_State* state);
