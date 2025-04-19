#include "r_vk_utils.h"
#include "base/base_logger.h"
#include "render/r_core.h"
#include "render/r_pipeline.h"
#include "third_party/vulkan/include/vulkan_core.h"


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
  VkFormat result = {};

  switch (format)
  {
    case R_VERTEX_ATTRIBUTE_FORMAT_VEC3F: result = VK_FORMAT_R32G32B32_SFLOAT; break;
    case R_VERTEX_ATTRIBUTE_FORMAT_VEC2F: result = VK_FORMAT_R32G32_SFLOAT; break;
    default: Assert(1); break;
  }

  return result;
}

func U32
R_VK_FindMemoryTypeIndex(U32 type_filter, VkMemoryPropertyFlags property_flags)
{
  R_VK_State* state = &r_vk_state;
  
  VkPhysicalDeviceMemoryProperties memory_properties;
  vkGetPhysicalDeviceMemoryProperties(state->device.physical, &memory_properties);

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
