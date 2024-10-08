#include "r_vk_buffer.h"
#include "r_vk_core.h"

#include "third_party/vulkan/include/vulkan.h"

func VkBufferUsageFlags
_VkFromBufferUsageFlags(BufferUsageFlags flags)
{
  VkBufferUsageFlags result = 0;

  if((flags & BUFFER_USAGE_FLAG_VERTEX) == BUFFER_USAGE_FLAG_VERTEX)
  {
    result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  }
  if((flags & BUFFER_USAGE_FLAG_INDEX) == BUFFER_USAGE_FLAG_INDEX)
  {
    result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  }
  if((flags & BUFFER_USAGE_FLAG_UNIFORM) == BUFFER_USAGE_FLAG_UNIFORM)
  {
    result |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  }

  return result;
}

func VkMemoryPropertyFlags
_VkFromBufferPropertyFlags(BufferPropertyFlags flags)
{
  VkMemoryPropertyFlags result = 0;

  if ((flags & BUFFER_PROPERTY_HOST_COHERENT) == BUFFER_PROPERTY_HOST_COHERENT)
  {
    result |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  }
  if ((flags & BUFFER_PROPERTY_HOST_VISIBLE) == BUFFER_PROPERTY_HOST_VISIBLE)
  {
    result |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
  }

  return result;
}

func R_Buffer
_VK_CreateBuffer(U64 size, BufferUsageFlags usage_type, BufferPropertyFlags property_flags)
{
  R_Buffer result = {};
  result.size = 0;
  result.handle = r_vk_state.free_buffer_index;

  VkBufferCreateInfo buffer_info = {};
  buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_info.size = size;
  buffer_info.usage = _VkFromBufferUsageFlags(usage_type);
  buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  
  _VK_Buffer* vk_buffer = r_vk_state.buffers + result.handle;

  VK_CHECK(vkCreateBuffer(r_vk_state.device.logical, &buffer_info, 0, &vk_buffer->buffer));
  r_vk_state.free_buffer_index += 1;

  VkMemoryRequirements memory_requirements;
  vkGetBufferMemoryRequirements(
      r_vk_state.device.logical,
      vk_buffer->buffer,
      &memory_requirements);

  VkMemoryAllocateInfo allocate_info = {};
  allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocate_info.allocationSize = memory_requirements.size;
  allocate_info.memoryTypeIndex = R_VK_FindMemoryType(
      memory_requirements.memoryTypeBits,
      _VkFromBufferPropertyFlags(property_flags));
  
  VK_CHECK(vkAllocateMemory(
        r_vk_state.device.logical,
        &allocate_info,
        0,
        &vk_buffer->memory));

  VK_CHECK(vkBindBufferMemory(
        r_vk_state.device.logical,
        vk_buffer->buffer,
        vk_buffer->memory,
        0));

  return result;
}

func void
_VK_FillBuffer(R_Buffer* buffer, U8* data, U64 size, U64 offset)
{
  _VK_Buffer* vk_buffer = r_vk_state.buffers + buffer->handle;

  // --AlNov: @NOTE Better to use staging buffer.
  //                Not CPU/GPU visible memory.
  vkMapMemory(r_vk_state.device.logical, vk_buffer->memory, 0, size, 0, &vk_buffer->mapped_memory);
  {
    memcpy((U8*)vk_buffer->mapped_memory, data, size);
  }
  vkUnmapMemory(r_vk_state.device.logical, vk_buffer->memory);
}
