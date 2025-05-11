#include "r_vk_buffer.h"
#include "render/r_buffer.h"
#include "third_party/vulkan/include/vulkan_core.h"

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
  if((flags & BUFFER_USAGE_FLAG_TRANSFER_SRC) == BUFFER_USAGE_FLAG_TRANSFER_SRC)
  {
    result |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
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

func R_VK_Buffer R_VK_CreateBuffer(U64 capacity, BufferUsageFlags usage_flags, BufferPropertyFlags property_flags)
{
  // @NOTE This is to create Vulkan Buffer and Memory
  R_VK_State* state = &r_vk_state;
  R_VK_Buffer result = {};
  result.capacity = capacity;
  
  VkBufferCreateInfo buffer_info = {
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .size = capacity,
    .usage = _VkFromBufferUsageFlags(usage_flags),
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE
  };

  VK_CHECK(vkCreateBuffer(state->device.logical, &buffer_info, 0, &result.handle));

  VkMemoryRequirements memory_requirements;
  vkGetBufferMemoryRequirements(state->device.logical, result.handle, &memory_requirements);
  
  VkPhysicalDeviceMemoryProperties mem_properties = {};
  vkGetPhysicalDeviceMemoryProperties(state->device.physical, &mem_properties);

  VkMemoryPropertyFlags flags = _VkFromBufferPropertyFlags(property_flags);
  U32 memory_type_index = 0;
  for (U32 type_index = 0; type_index < mem_properties.memoryTypeCount; type_index += 1)
  {
    if (memory_requirements.memoryTypeBits & (1 << type_index) && ((mem_properties.memoryTypes[type_index].propertyFlags & flags) == flags))
    {
      memory_type_index = type_index;
      break;
    }
  }

  VkMemoryAllocateInfo allocation_info = {
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .allocationSize = memory_requirements.size,
    .memoryTypeIndex = memory_type_index
  };
  VK_CHECK(vkAllocateMemory(state->device.logical, &allocation_info , 0, &result.memory));

  VK_CHECK(vkBindBufferMemory(state->device.logical, result.handle, result.memory, 0));

  return result;
}

func void
R_VK_DestroyBuffer(R_VK_State* state)
{
  vkFreeMemory(state->device.logical, state->geometry_buffer.memory, 0);
  vkDestroyBuffer(state->device.logical, state->geometry_buffer.handle, 0);
}
