#pragma once

struct R_VK_Buffer
{
  VkBuffer handle;
  VkDeviceMemory memory;
  U64 size;
  U64 capacity;
};

func R_VK_Buffer R_VK_CreateBuffer(U64 capacity, BufferUsageFlags usage_flags, BufferPropertyFlags flags);
func void R_VK_DestroyBuffer(R_VK_State* state);
