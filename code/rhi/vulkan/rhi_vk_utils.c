#include "rhi_vk_utils.h"
#include "base/base_logger.h"
#include "third_party/vulkan/include/vulkan_core.h"

func VkIndexType
RHI_VK_GetVkIndexTypeFrom(RHI_IndexSize index_size)
{
	VkIndexType vk_index_type_table[] = {
		VK_INDEX_TYPE_UINT16,
		VK_INDEX_TYPE_UINT32,
	};
	return vk_index_type_table[index_size];
}

func VkAttachmentLoadOp
RHI_VK_GetVkAttachmentLoadOperation(RHI_LoadOperation operation)
{
  VkAttachmentLoadOp result = VK_ATTACHMENT_LOAD_OP_MAX_ENUM;

  switch (operation)
  {
    default: Assert(1); break;

    case (RHI_AttachmentLoadOperation_DontCare):
    {
      result = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    } break;
    case (RHI_AttachmentLoadOperation_Load):
    {
      result = VK_ATTACHMENT_LOAD_OP_LOAD;
    } break;
    case (RHI_AttachmentLoadOperation_Clear):
    {
      result = VK_ATTACHMENT_LOAD_OP_CLEAR;
    } break;
  }

  return result;
}

func VkAttachmentStoreOp
RHI_VK_GetVkAttachmentStoreOperation(RHI_StoreOperation operation)
{
  VkAttachmentStoreOp result = VK_ATTACHMENT_STORE_OP_MAX_ENUM;

  switch (operation)
  {
    default: Assert(1); break;

    case RHI_AttachmentStoreOperation_DontCare: result = VK_ATTACHMENT_STORE_OP_DONT_CARE; break;
    case RHI_AttachmentStoreOperation_Store:    result = VK_ATTACHMENT_STORE_OP_DONT_CARE; break;
  }

  return result;
}

func VkFormat
RHI_VK_GetVkFormatAttribute(RHI_VertexAttributeFormat format)
{
  VkFormat result = {0};

  switch (format)
  {
    case RHI_VertexAttributeFormat_Vec2F32: result = VK_FORMAT_R32G32B32A32_SFLOAT; break;
    case RHI_VertexAttributeFormat_Vec3F32: result = VK_FORMAT_R32G32B32A32_SFLOAT; break;
    case RHI_VertexAttributeFormat_Vec4F32: result = VK_FORMAT_R32G32B32A32_SFLOAT; break;
    
    case RHI_VertexAttributeFormat_Vec4I32: result = VK_FORMAT_R32G32B32A32_SINT; break;

    default: Assert(1); break;
  }

  return result;
}

func VkShaderStageFlags
RHI_VK_GetVkShaderStage(RHI_ShaderKind shader_kind)
{
  switch (shader_kind)
  {
    case RHI_ShaderKind_Vertex:   return VK_SHADER_STAGE_VERTEX_BIT;
    case RHI_ShaderKind_Fragment: return VK_SHADER_STAGE_FRAGMENT_BIT;

    default: Assert(1); return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
  }
}

func VkDescriptorType
RHI_VK_GetVkDescriptorType(RHI_BindingKind binding_kind)
{
  switch (binding_kind)
  {
    case RHI_BindingKind_UniformBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case RHI_BindingKind_Sampler:       return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    default: Assert(1); return VK_DESCRIPTOR_TYPE_MAX_ENUM;
  }
}

func U32
RHI_VK_FindMemoryTypeIndex(U32 type_filter, VkMemoryPropertyFlags property_flags)
{
  VkPhysicalDeviceMemoryProperties memory_properties;
  vkGetPhysicalDeviceMemoryProperties(_rhi_vk_state.device.physical, &memory_properties);

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
RHI_VK_TransitImageLayout(VkCommandBuffer cmd, VkImage image, VkImageLayout layout,
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
RHI_VK_GetVkImageType(VkImageType image_type)
{
  VkImageType vk_image_type = 0;

  switch (image_type)
  {
    default: Assert(1); break;

    case RHI_TextureKind_2D: vk_image_type =  VK_IMAGE_TYPE_2D; break;
  }

  return vk_image_type;
}

func RHI_TextureFormat RHI_VK_TextureFormatFromVkFormat(VkFormat format)
{
  RHI_TextureFormat texture_format = 0;

  switch (format)
  {
    default: Assert(1); break;

    case VK_FORMAT_R8G8B8A8_SRGB: texture_format = RHI_TextureFormat_R8G8B8A8_SRGB; break; 
    case VK_FORMAT_R8G8B8A8_UNORM: texture_format = RHI_TextureFormat_R8G8B8A8_UNORM; break; 
    case VK_FORMAT_B8G8R8A8_UNORM: texture_format = RHI_TextureFormat_B8G8R8A8_UNORM; break;
    case VK_FORMAT_D16_UNORM: texture_format = RHI_TextureFormat_D16_UNORM; break;
    case VK_FORMAT_R16_UINT: texture_format = RHI_TextureFormat_R16_UINT; break;
  }

  return texture_format;
}

func VkFormat
RHI_VK_GetVkFormat(RHI_TextureFormat format)
{
  VkFormat vk_format = 0;

  switch (format)
  {
    default: Assert(1); break;

    case RHI_TextureFormat_None: vk_format = VK_FORMAT_UNDEFINED; break;
    case RHI_TextureFormat_R8G8B8A8_SRGB: vk_format = VK_FORMAT_R8G8B8A8_SRGB; break;
    case RHI_TextureFormat_R8G8B8A8_UNORM: vk_format = VK_FORMAT_R8G8B8A8_UNORM; break;
    case RHI_TextureFormat_B8G8R8A8_UNORM: vk_format = VK_FORMAT_B8G8R8A8_UNORM; break;
    case RHI_TextureFormat_R16G16B16A16_SFLOAT: vk_format = VK_FORMAT_R16G16B16A16_SFLOAT; break;
    case RHI_TextureFormat_D16_UNORM: vk_format = VK_FORMAT_D16_UNORM; break;
    case RHI_TextureFormat_R16_UINT: vk_format = VK_FORMAT_R16_UINT; break;
  }

  return vk_format;
}

func RHI_TextureFormat
RHI_VK_FormatFromVk(VkFormat format)
{
  RHI_TextureFormat result = 0;

  switch (format)
  {
    default: Assert(1); break;

    case VK_FORMAT_UNDEFINED:           result = RHI_TextureFormat_None; break;
    case VK_FORMAT_R8G8B8A8_SRGB:       result = RHI_TextureFormat_R8G8B8A8_SRGB; break;
    case VK_FORMAT_R8G8B8A8_UNORM:      result = RHI_TextureFormat_R8G8B8A8_UNORM; break;
    case VK_FORMAT_B8G8R8A8_UNORM:      result = RHI_TextureFormat_B8G8R8A8_UNORM; break;
    case VK_FORMAT_R16G16B16A16_SFLOAT: result = RHI_TextureFormat_R16G16B16A16_SFLOAT;
    case VK_FORMAT_D16_UNORM:           result = RHI_TextureFormat_D16_UNORM; break;
    case VK_FORMAT_R16_UINT:            result = RHI_TextureFormat_R16_UINT; break;
  }

  return result;
}

func VkImageUsageFlags
RHI_VK_GetVkImageUsageFlags(RHI_TextureUsageFlags flags)
{
  VkImageUsageFlags vk_flags = 0;
  if ((flags & RHI_TEXTURE_USAGE_FLAG_COLOR_ATTACHMENT) == RHI_TEXTURE_USAGE_FLAG_COLOR_ATTACHMENT)
  {
    vk_flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  }
  if ((flags & RHI_TEXTURE_USAGE_FLAG_DEPTH_STENCIL_ATTACHMENT) == RHI_TEXTURE_USAGE_FLAG_DEPTH_STENCIL_ATTACHMENT)
  {
    vk_flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  }
  if ((flags & RHI_TEXTURE_USAGE_FLAG_TRANSFER_SRC) == RHI_TEXTURE_USAGE_FLAG_TRANSFER_SRC)
  {
    vk_flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  }
  if ((flags & RHI_TEXTURE_USAGE_FLAG_TRANSFER_DST) == RHI_TEXTURE_USAGE_FLAG_TRANSFER_DST)
  {
    vk_flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  }
  if ((flags & RHI_TEXTURE_USAGE_FLAG_SAMPLED) == RHI_TEXTURE_USAGE_FLAG_SAMPLED)
  {
    vk_flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
  }

  return vk_flags;
}

func VkFilter
RHI_VK_GetVkFilter(RHI_FilterKind filter)
{
  switch (filter)
  {
    default: Assert(1); return 0;

    case RHI_FilterKind_Nearest: return VK_FILTER_NEAREST;
    case RHI_FilterKind_Linear:  return VK_FILTER_LINEAR;
  }
}

func VkSamplerMipmapMode
RHI_VK_GetVkSamplerMipmapMode(RHI_SamplerMipmapMode mode)
{
  switch (mode)
  {
    default: Assert(1); return 0;

    case RHI_SamplerMipmapMode_Nearest: return VK_SAMPLER_MIPMAP_MODE_NEAREST;
    case RHI_SamplerMipmapMode_Linear:  return VK_SAMPLER_MIPMAP_MODE_LINEAR;
  }
}

func VkSamplerAddressMode
RHI_VK_GetVkSamplerAddressMode(RHI_SamplerAddressMode mode)
{
  switch (mode)
  {
    default: Assert(1); return 0;

    case RHI_SamplerAddressMode_Repeat:         return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case RHI_SamplerAddressMode_MirroredRepeat: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    case RHI_SamplerAddressMode_ClampToEdge:    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case RHI_SamplerAddressMode_ClampToBorder:  return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
  }
}

func VkCompareOp
RHI_VK_GetVkFromCompareOperation(RHI_CompareOperation operation)
{
  VkCompareOp vk_operation = 0;
  
  switch (operation)
  {
    default: Assert(1); break;

    case RHI_CompareOperation_Equal: vk_operation = VK_COMPARE_OP_EQUAL; break;
    case RHI_CompareOperation_NotEqual: vk_operation = VK_COMPARE_OP_NOT_EQUAL; break;
    case RHI_CompareOperation_Less: vk_operation = VK_COMPARE_OP_LESS; break;
    case RHI_CompareOperation_LessOrEqual: vk_operation = VK_COMPARE_OP_LESS_OR_EQUAL; break;
    case RHI_CompareOperation_Greater: vk_operation = VK_COMPARE_OP_GREATER; break;
    case RHI_CompareOperation_GreaterOrEqual: vk_operation = VK_COMPARE_OP_GREATER_OR_EQUAL; break;
  }

  return vk_operation;
}
