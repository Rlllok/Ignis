#pragma once

#include "base/base_include.cpp"

struct R_VK_Buffer
{
  VkBuffer        buffer;
  VkDeviceMemory  memory;
  void*           mapped_memory;
  U32             current_position;
  U32             size;
};


func R_Buffer _VK_CreateBuffer(U64 size, BufferUsageFlags usage_type, BufferPropertyFlags flags);
func void _VK_FillBuffer(struct R_Buffer* buffer, U8* data, U64 size, U64 offset);
