#include "r_vk_utils.h"
#include "base/base_logger.h"
#include "third_party/vulkan/include/vulkan_core.h"

func VkIndexType
R_VK_GetVkIndexTypeFrom(R_IndexSize index_size)
{
	VkIndexType vk_index_type_table[] = {
		VK_INDEX_TYPE_UINT16,
		VK_INDEX_TYPE_UINT32,
	};
	return vk_index_type_table[index_size];
}

func VkAttachmentLoadOp
R_VK_GetVkAttachmentLoadOperation(R_AttachmentLoadOperation operation)
{
  VkAttachmentLoadOp result = VK_ATTACHMENT_LOAD_OP_MAX_ENUM;

  switch (operation)
  {
    case (R_ATTACHMENT_LOAD_OPERATION_DONT_CARE):
    {
      result = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    } break;
    case (R_ATTACHMENT_LOAD_OPERATION_LOAD):
    {
      result = VK_ATTACHMENT_LOAD_OP_LOAD;
    } break;
    case (R_ATTACHMENT_LOAD_OPERATION_CLEAR):
    {
      result = VK_ATTACHMENT_LOAD_OP_CLEAR;
    } break;
  }

  return result;
}

func VkFormat
R_VK_GetVkFormatAttribute(R_VertexAttributeFormat format)
{
  VkFormat result = {0};

  switch (format)
  {
    case R_VERTEX_ATTRIBUTE_FORMAT_VEC2F32: result = VK_FORMAT_R32G32_SFLOAT; break;
    case R_VERTEX_ATTRIBUTE_FORMAT_VEC3F32: result = VK_FORMAT_R32G32B32_SFLOAT; break;
    case R_VERTEX_ATTRIBUTE_FORMAT_VEC4F32: result = VK_FORMAT_R32G32B32A32_SFLOAT; break;

    default: Assert(1); break;
  }

  return result;
}

func VkShaderStageFlags
R_VK_GetVkShaderStage(R_ShaderType shader_type)
{
  switch (shader_type)
  {
    case R_SHADER_TYPE_VERTEX: return VK_SHADER_STAGE_VERTEX_BIT;
    case R_SHADER_TYPE_FRAGMENT: return VK_SHADER_STAGE_FRAGMENT_BIT;

    default: Assert(1); return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
  }
}

func VkDescriptorType
R_VK_GetVkDescriptorType(R_BindingType binding_type)
{
  switch (binding_type)
  {
    case R_BINDING_TYPE_UNIFORM_BUFFER: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case R_BINDING_TYPE_TEXTURE_2D: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    default: Assert(1); return VK_DESCRIPTOR_TYPE_MAX_ENUM;
  }
}

func U32
R_VK_FindMemoryTypeIndex(U32 type_filter, VkMemoryPropertyFlags property_flags)
{
  VkPhysicalDeviceMemoryProperties memory_properties;
  vkGetPhysicalDeviceMemoryProperties(_r_vk_state.device.physical, &memory_properties);

  for (U32 i = 0; i < memory_properties.memoryTypeCount; i += 1)
  {
    if ((type_filter & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & property_flags) == property_flags)
    {
      return i;
    }
  }

  AssertMessage(1, "Failed to find suitable memory type.");

  return 0;
}

func void
R_VK_TransitImageLayout(VkCommandBuffer cmd, VkImage image, VkImageLayout layout,
                   VkImageLayout new_layout, VkAccessFlags srs_access,
                   VkAccessFlags dst_access, VkPipelineStageFlags src_stage,
                   VkPipelineStageFlags dst_stage, VkImageAspectFlags aspect_mask)
{
  VkImageMemoryBarrier image_barrier = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
    .oldLayout = layout,
    .newLayout = new_layout,
    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .image = image,
    .subresourceRange = {
      .aspectMask = aspect_mask,
      .baseMipLevel = 0,
      .levelCount = 1,
      .baseArrayLayer = 0,
      .layerCount = 1
    }
  };

  vkCmdPipelineBarrier(cmd, src_stage, dst_stage, 0, 0, 0, 0, 0, 1, &image_barrier);
}
