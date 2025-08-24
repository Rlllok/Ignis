#pragma once

#include "render/r_core.h"
#include "third_party/vulkan/include/vulkan_core.h"

func VkIndexType R_VK_GetVkIndexTypeFrom(R_IndexSize index_size);

func VkAttachmentLoadOp R_VK_GetVkAttachmentLoadOperation(R_LoadOperation operation);
func VkAttachmentStoreOp R_VK_GetVkAttachmentStoreOperation(R_StoreOperation operation);
func VkFormat R_VK_GetVkFormatAttribute(R_VertexAttributeFormat format);
func VkShaderStageFlags R_VK_GetVkShaderStage(R_ShaderType shader_type);
func VkDescriptorType R_VK_GetVkDescriptorType(R_BindingType binding_type);

func U32 R_VK_FindMemoryTypeIndex(U32 type_filter, VkMemoryPropertyFlags property_flags);

func void R_VK_TransitImageLayout(VkCommandBuffer cmd, VkImage image, VkImageLayout layout,
                   VkImageLayout new_layout, VkAccessFlags srs_access,
                   VkAccessFlags dst_access, VkPipelineStageFlags src_stage,
                   VkPipelineStageFlags dst_stage, VkImageAspectFlags apect_mask);

func VkImageType R_VK_GetVkImageType(VkImageType image_type);
func VkFormat R_VK_GetVkFormat(R_TextureFormat format);
func R_TextureFormat R_VK_TextureFormatFromVkFormat(VkFormat format);
func VkImageUsageFlags R_VK_GetVkImageUsageFlags(R_TextureUsageFlags flags);

func VkFilter R_VK_GetVkFilter(R_FilterType filter);

func VkSamplerMipmapMode R_VK_GetVkSamplerMipmapMode(R_SamplerMipmapMode mode);
func VkSamplerAddressMode R_VK_GetVkSamplerAddressMode(R_SamplerAddressMode mode);

func VkCompareOp R_VK_GetVkFromCompareOperation(R_CompareOperation operation);
