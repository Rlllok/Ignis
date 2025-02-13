#pragma once

#include "third_party/vulkan/include/vulkan_core.h"
struct R_VK_Device
{
  VkDevice logical;
  VkPhysicalDevice physical;

  U32 graphics_queue_index;
  U32 present_queue_index;
  U32 transfer_queue_index;

  VkQueue graphics_queue;
  VkQueue present_queue;
  VkQueue transfer_queue;

  VkPhysicalDeviceProperties properties;
};

func void R_VK_CreateDevice(R_VK_Device* device);
func void R_VK_DestroyDevice(R_VK_Device* device);
