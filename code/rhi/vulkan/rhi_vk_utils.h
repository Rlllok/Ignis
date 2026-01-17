#pragma once

#include "third_party/vulkan/include/vulkan_core.h"
#include "rhi/rhi_core.h"

func VkIndexType RHI_VK_GetVkIndexTypeFrom(RHI_IndexSize index_size);

func VkAttachmentLoadOp RHI_VK_GetVkAttachmentLoadOperation(RHI_LoadOperation operation);
func VkAttachmentStoreOp RHI_VK_GetVkAttachmentStoreOperation(RHI_StoreOperation operation);
func VkFormat RHI_VK_GetVkFormatAttribute(RHI_VertexAttributeFormat format);
func VkShaderStageFlags RHI_VK_GetVkShaderStage(RHI_ShaderType shader_type);
func VkDescriptorType RHI_VK_GetVkDescriptorType(RHI_BindingType binding_type);

func U32 RHI_VK_FindMemoryTypeIndex(U32 type_filter, VkMemoryPropertyFlags property_flags);

func void RHI_VK_TransitImageLayout(VkCommandBuffer cmd, VkImage image, VkImageLayout layout,
                   VkImageLayout new_layout, VkAccessFlags srs_access,
                   VkAccessFlags dst_access, VkPipelineStageFlags src_stage,
                   VkPipelineStageFlags dst_stage, VkImageAspectFlags apect_mask);

func VkImageType RHI_VK_GetVkImageType(VkImageType image_type);
func VkFormat RHI_VK_GetVkFormat(RHI_TextureFormat format);
func RHI_TextureFormat RHI_VK_FormatFromVk(VkFormat format);
func RHI_TextureFormat RHI_VK_TextureFormatFromVkFormat(VkFormat format);
func VkImageUsageFlags RHI_VK_GetVkImageUsageFlags(RHI_TextureUsageFlags flags);

func VkFilter RHI_VK_GetVkFilter(RHI_FilterType filter);

func VkSamplerMipmapMode RHI_VK_GetVkSamplerMipmapMode(RHI_SamplerMipmapMode mode);
func VkSamplerAddressMode RHI_VK_GetVkSamplerAddressMode(RHI_SamplerAddressMode mode);

func VkCompareOp RHI_VK_GetVkFromCompareOperation(RHI_CompareOperation operation);
