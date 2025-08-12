#pragma once

#include "render/r_core.h"
#include "third_party/vulkan/include/vulkan_core.h"

func VkIndexType R_VK_GetVkIndexTypeFrom(R_IndexSize index_size);

func VkAttachmentLoadOp R_VK_GetVkAttachmentLoadOperation(R_AttachmentLoadOperation operation);
func VkFormat R_VK_GetVkFormatAttribute(R_VertexAttributeFormat format);
func VkShaderStageFlags R_VK_GetVkShaderStage(R_ShaderType shader_type);
func VkDescriptorType R_VK_GetVkDescriptorType(R_BindingType binding_type);

func U32 R_VK_FindMemoryTypeIndex(U32 type_filter, VkMemoryPropertyFlags property_flags);

func void R_VK_TransitImageLayout(VkCommandBuffer cmd, VkImage image, VkImageLayout layout,
                   VkImageLayout new_layout, VkAccessFlags srs_access,
                   VkAccessFlags dst_access, VkPipelineStageFlags src_stage,
                   VkPipelineStageFlags dst_stage, VkImageAspectFlags apect_mask);
