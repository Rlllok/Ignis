#pragma once

struct R_VK_Device
{
  VkDevice logical;
  VkPhysicalDevice physical;
  U32 queue_index;
};

func void R_VK_CreateDevice();
