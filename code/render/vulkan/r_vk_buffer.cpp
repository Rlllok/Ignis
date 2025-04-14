#include "r_vk_buffer.h"

#include "render/r_gltf.h"
#include "render/r_gltf.cpp"

func void
R_VK_CreateBuffer(R_VK_State* state)
{
  // @TODO @NOTE HARDCODED
  GLTFReader gltf_reader = {};
  gltf_reader.file_buffer = ReadFile("data/box_gltf/test.gltf");
  ParseGLTF(&gltf_reader);

  GLTFElement* buffers_list_element = LookUpElement(gltf_reader.element, ConstString("buffers"));
  GLTFElement* buffer_element = buffers_list_element->first_sub_element->first_sub_element;
  Buffer mesh_buffer = buffer_element->value;
  
  U64 comma_position = FindPosition(mesh_buffer, ',');
  U64 quat_position = FindPosition(mesh_buffer, '"');
  mesh_buffer.data = mesh_buffer.data + comma_position + 1;
  mesh_buffer.size = mesh_buffer.size - comma_position - 1;
  Buffer decoded = Base64Decode(mesh_buffer);
  U64 vertex_offset = 8;
  U64 vertex_length = 36;
  
  PrintBuffer(mesh_buffer);
  printf("\n");
  PrintBuffer(decoded);
  
  // VkDeviceSize size = sizeof(app_state.vertecies[0]) * CountArrayElements(app_state.vertecies);
  state->vertex_buffer.size = decoded.size;
  
  VkBufferCreateInfo buffer_info = {
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .size = state->vertex_buffer.size,
    .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE
  };

  VK_CHECK(vkCreateBuffer(state->device.logical, &buffer_info, 0, &state->vertex_buffer.handle));

  VkMemoryRequirements memory_requirements;
  vkGetBufferMemoryRequirements(state->device.logical, state->vertex_buffer.handle, &memory_requirements);
  
  VkPhysicalDeviceMemoryProperties mem_properties = {};
  vkGetPhysicalDeviceMemoryProperties(state->device.physical, &mem_properties);

  VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
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
  VK_CHECK(vkAllocateMemory(state->device.logical, &allocation_info , 0, &state->vertex_buffer.memory));

  VK_CHECK(vkBindBufferMemory(state->device.logical, state->vertex_buffer.handle, state->vertex_buffer.memory, 0));

  void* data;
  VK_CHECK(vkMapMemory(state->device.logical, state->vertex_buffer.memory, 0, state->vertex_buffer.size, 0, &data));
  {
      memcpy(data, decoded.data, decoded.size);
  }
  vkUnmapMemory(state->device.logical, state->vertex_buffer.memory);
}

func void
R_VK_DestroyBuffer(R_VK_State* state)
{
  vkFreeMemory(state->device.logical, state->vertex_buffer.memory, 0);
  vkDestroyBuffer(state->device.logical, state->vertex_buffer.handle, 0);
}
