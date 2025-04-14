#pragma once

struct R_VK_Buffer
{
  VkBuffer handle;
  U64 size;
  VkDeviceMemory memory;
};

func void R_VK_CreateBuffer(R_VK_State* state);
func void R_VK_DestroyBuffer(R_VK_State* state);
