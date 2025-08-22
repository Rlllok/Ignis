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
R_VK_GetVkAttachmentLoadOperation(R_LoadOperation operation)
{
  VkAttachmentLoadOp result = VK_ATTACHMENT_LOAD_OP_MAX_ENUM;

  switch (operation)
  {
    default: Assert(1); break;

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

func VkAttachmentStoreOp
R_VK_GetVkAttachmentStoreOperation(R_StoreOperation operation)
{
  VkAttachmentStoreOp result = VK_ATTACHMENT_STORE_OP_MAX_ENUM;

  switch (operation)
  {
    default: Assert(1); break;

    case R_ATTACHMENT_STORE_OPERATION_DONT_CARE: result = VK_ATTACHMENT_STORE_OP_DONT_CARE; break;
    case R_ATTACHMENT_STORE_OPERATION_STORE: result = VK_ATTACHMENT_STORE_OP_STORE; break;
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

func VkImageType
R_VK_GetVkImageType(VkImageType image_type)
{
  VkImageType vk_image_type = 0;

  switch (image_type)
  {
    default: Assert(1); break;

    case R_TEXTURE_TYPE_2D: vk_image_type =  VK_IMAGE_TYPE_2D; break;
  }

  return vk_image_type;
}

func R_TextureFormat R_VK_TextureFormatFromVkFormat(VkFormat format)
{
  R_TextureFormat texture_format = 0;

  switch (format)
  {
    default: Assert(1); break;

    case VK_FORMAT_R8G8B8A8_SRGB: texture_format = R_TEXTURE_FORMAT_R8G8B8A8_UNORM_SRGB; break; 
    case VK_FORMAT_B8G8R8A8_UNORM: texture_format = R_TEXTURE_FORMAT_B8G8R8A8_UNORM; break;
    case VK_FORMAT_D16_UNORM: texture_format = R_TEXTURE_FORMAT_D16_UNORM; break;
    case VK_FORMAT_R16_UINT: texture_format = R_TEXTURE_FORMAT_R16_UINT; break;
  }

  return texture_format;
}

func VkFormat
R_VK_GetVkFormat(R_TextureFormat format)
{
  VkFormat vk_format = 0;

  switch (format)
  {
    default: Assert(1); break;

    case R_TEXTURE_FORMAT_NONE: vk_format = VK_FORMAT_UNDEFINED; break;
    case R_TEXTURE_FORMAT_R8G8B8A8_UNORM_SRGB: vk_format = VK_FORMAT_R8G8B8A8_SRGB; break;
    case R_TEXTURE_FORMAT_B8G8R8A8_UNORM: vk_format = VK_FORMAT_B8G8R8A8_UNORM; break;
    case R_TEXTURE_FORMAT_R16G16B16A16_SFLOAT: vk_format = VK_FORMAT_R16G16B16A16_SFLOAT; break;
    case R_TEXTURE_FORMAT_D16_UNORM: vk_format = VK_FORMAT_D16_UNORM; break;
    case R_TEXTURE_FORMAT_R16_UINT: vk_format = VK_FORMAT_R16_UINT; break;
  }

  return vk_format;
}

func VkImageUsageFlags
R_VK_GetVkImageUsageFlags(R_TextureUsageFlags flags)
{
  VkImageUsageFlags vk_flags = 0;
  if ((flags & R_TEXTURE_USAGE_FLAG_COLOR_ATTACHMENT) == R_TEXTURE_USAGE_FLAG_COLOR_ATTACHMENT)
  {
    vk_flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  }
  if ((flags & R_TEXTURE_USAGE_FLAG_DEPTH_STENCIL_ATTACHMENT) == R_TEXTURE_USAGE_FLAG_DEPTH_STENCIL_ATTACHMENT)
  {
    vk_flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  }
  if ((flags & R_TEXTURE_USAGE_FLAG_TRANSFER_SRC) == R_TEXTURE_USAGE_FLAG_TRANSFER_SRC)
  {
    vk_flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  }
  if ((flags & R_TEXTURE_USAGE_FLAG_TRANSFER_DST) == R_TEXTURE_USAGE_FLAG_TRANSFER_DST)
  {
    vk_flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  }

  return vk_flags;
}

func VkCompareOp
R_VK_GetVkFromCompareOperation(R_CompareOperation operation)
{
  VkCompareOp vk_operation = 0;
  
  switch (operation)
  {
    default: Assert(1); break;

    case R_COMPARE_OPERATION_EQUAL: vk_operation = VK_COMPARE_OP_EQUAL; break;
    case R_COMPARE_OPERATION_NOT_EQUAL: vk_operation = VK_COMPARE_OP_NOT_EQUAL; break;
    case R_COMPARE_OPERATION_LESS: vk_operation = VK_COMPARE_OP_LESS; break;
    case R_COMPARE_OPERATION_LESS_OR_EQUAL: vk_operation = VK_COMPARE_OP_LESS_OR_EQUAL; break;
    case R_COMPARE_OPERATION_GREATER: vk_operation = VK_COMPARE_OP_GREATER; break;
    case R_COMPARE_OPERATION_GREATER_OR_EQUAL: vk_operation = VK_COMPARE_OP_GREATER_OR_EQUAL; break;
  }

  return vk_operation;
}
